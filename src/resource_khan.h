/**
 * @file resource_khan.h
 * @brief Graph resource manager.
 * @author Philipp Schilk 2023
 * https://github.com/schilkp/ResourceKhan
 */
#ifndef RESOURCE_KHAN_H_
#define RESOURCE_KHAN_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef RK_USE_CUSTOM_CONF
#include "resource_khan_conf_custom.h"
#else

#include <stdio.h>

/** @brief Maximum length of node/client name  */
#define RK_MAX_NAME_LEN 15

/** @brief Maximum number of parent nodes per node or client */
#define RK_MAX_PARENTS  4

/** @brief Maximum number of children nodes and clients per node */
#define RK_MAX_CHILDREN 4

/**
 * @brief Return value indicating a function did not complete succesfully
 * @note Must be an 'int' value that is not '0'
 */
#define RK_ERR          1

/** @brief Information log function */
#define RK_LOG_INF(_fmt_, ...)                                                                                         \
  do {                                                                                                                 \
    printf("INF: "_fmt_                                                                                                \
           "\n",                                                                                                       \
           __VA_ARGS__);                                                                                               \
  } while (0)

/** @brief Error log function */
#define RK_LOG_ERR(_fmt_, ...)                                                                                         \
  do {                                                                                                                 \
    printf("ERR: "_fmt_                                                                                                \
           "\n",                                                                                                       \
           __VA_ARGS__);                                                                                               \
  } while (0)

#endif /* RK_USE_CUSTOM_CONF */

/**
 * @brief A resource graph.
 * Must be initialized with a pointer to an array containing pointers to all nodes,
 * the length of said array (number of nodes), and a pointer to the root node.
 */
struct rk_graph {
  /** @brief Array of all nodes. */
  struct rk_node **nodes;

  /** @brief Number of nodes in this graph. */
  size_t node_count;

  /** @brief Root of the graph. */
  struct rk_node *root;

  /** @brief Scratch data used by implementation. Initialize to zero. */
  struct rk_node *ll_topo_tail;
};

// Scratch data used by implementation.
struct rk_node_ctx {
  struct rk_node *ll_trv;
  struct rk_node *ll_topo_next;
  struct rk_node *ll_topo_prev;
};

/**
 * @brief A node in the resource graph.
 * Represents an automatically managed resource, such as a regulator or power switch.
 */
struct rk_node {
  /** @brief Node name/identifier.  */
  char name[RK_MAX_NAME_LEN + 1];

  /**
   * @brief Current state of this node. True if this node is enabled.
   * @warning Do not modify directly. Automatically managed by enable_client/disable_client functions.
   * @note If initialized as 'enabled', all parent nodes until the root must also be 'enabled'.
   *
   * Initialize to actual state of managed resource.
   *
   * If initialized as 'enabled' and no child/client is initialized as 'enabled', the graph
   * will be left in an non-optimal state, until a child/client is enabled or the graph is
   * optimized.
   */
  bool state;

  /**
   * @brief Number of parent nodes
   * @note May only be 0 for root node.
   */
  uint32_t parent_count;

  /**
   * @brief Pointer to parent nodes.
   * @note May only be empty for the root node
   */
  struct rk_node *parents[RK_MAX_PARENTS];

  /** @brief Number of children nodes. */
  uint32_t child_count;

  /** @brief Pointer to child nodes. */
  struct rk_node *children[RK_MAX_CHILDREN];

  /** @brief Number of clients. */
  uint32_t client_count;

  /** @brief Pointer to clients. */
  struct rk_client *clients[RK_MAX_CHILDREN];

  /**
   * @brief Update callback
   * @note Optional.
   * @param Pointer to node being updated.
   * Called when any dependent client (direct or indirect) is enabled or disabled,
   * or when the graph is optimized.
   *
   * - Use self->desired_state to determine if this resources should be turned on or off.
   * - Use self->state to determine if the node is currently enabled.
   * - Use self->previous_cb_return to check the return value of this node's callback during the
   *   last update.
   * - If this callback needs to check the state of any other nodes (for example the state of parents or children),
   *   Note that other_node->state indicates the current state of the node, while other_node->desired_state
   *   inidcates the state that ResourceKhan will attempt to set the node to during this update.
   *
   * @warning This callback should *not* change the value of self->state.
   * @return 0 if update successful and this node is now in the state self->desired_state,
   *         any non-zero number if this update failed.
   */
  int (*cb_update)(const struct rk_node *self);

  /**
   * @brief Previous return value of the node's callback.
   * @warning only valid during cb_update call.
   * Initialization does not matter.
   */
  int previous_cb_return;

  /**
   * @brief State that this node should be set to after the graph update is complete.
   * @warning only valid during cb_update call.
   * Initialization does not matter.
   */
  bool desired_state;

  /** @brief Scratch data used by implantation. Initialize to zero. */
  struct rk_node_ctx ctx;
};

/** @brief A client that requires the resources represented by some graph nodes */
struct rk_client {
  /** @brief Client name/identifier.  */
  char name[RK_MAX_NAME_LEN + 1];

  /**
   * @brief The state of the client.
   * @warning Do not modify directly. Automatically managed by enable_client/disable_client function.
   * @note If initialized as 'enabled', all parent nodes until the root must also be 'enabled'.
   * Initialize to the actual state of the client.
   */
  bool enabled;

  /**
   * @brief Number of parent nodes
   * @note May not be 0.
   */
  uint32_t parent_count;

  /** @brief The nodes representing the resources this client requires */
  struct rk_node *parents[RK_MAX_PARENTS];

  /** @brief Scratch data used by implantation. Initialize to zero. */
  bool in_dot_graph;
};

/**
 * @brief Enable a client in the resource graph.
 * This ensures that all resources that it depends on are enabled from the root down.
 *
 * @param graph resource graph.
 * @param client client to be enabled.
 * @return 0 if successful
 * @return RK_ERR if an unexpected nullpointer is encountered
 * @return the error code returned by a node's cb_update callback if a callback fails
 */
int rk_enable_client(struct rk_graph *graph, struct rk_client *client);

/**
 * @brief Disable a client in the resouce graph.
 * This disables all resource from the client upwards that are no longer required.
 *
 * @param graph resource graph.
 * @return 0 if successful
 * @return RK_ERR if an unexpected nullpointer is encountered
 * @return the error code returned by a node's cb_update callback if a callback fails
 */
int rk_disable_client(struct rk_graph *graph, struct rk_client *client);

/**
 * @brief Attempt to optimize the resource graph.
 * Scans the whole resource graph for nodes that are enabled although they have no active dependents.
 * This may happen if a node's callback returns a non-zero value, indicating an error. Any
 * nodes that are found to be in such a state are disabled.
 *
 * @param graph resource graph.
 * @return 0 if successful
 * @return RK_ERR if an unexpected nullpointer is encountered
 * @return the error code returned by a node's cb_update callback if a callback fails
 */
int rk_optimize(struct rk_graph *graph);

/**
 * @brief Add a child node to a node.
 * @warning This un-initializes the graph. Must call rk_init() before using the graph.
 * Updates the child's parent pointer, and the parent's child_count and children pointers.
 *
 * @param node node to receive new child
 * @param child child to be added
 * @return 0 if successful
 * @return RK_ERR if an unexpected nullpointer is encountered
 * @return RK_ERR if the RK_MAX_PARENTS or RK_MAX_CHILDREN are exceeded
 */
int rk_node_add_child(struct rk_node *node, struct rk_node *child);

/**
 * @brief Add a client to a node.
 * Updates the client's parent pointer, and the parent's client_count and client pointers.
 * @warning This un-initializes the graph. Must call rk_init() before using the graph.
 *
 * @param node node to receive new child
 * @param child child to be added
 * @return 0 if successful
 * @return RK_ERR if an unexpected nullpointer is encountered
 * @return RK_ERR if the RK_MAX_PARENTS or RK_MAX_CHILDREN are exceeded
 */
int rk_node_add_client(struct rk_node *node, struct rk_client *client);

/**
 * @brief Initialize a resource graph.
 * Must be called after all nodes and clients have been added to the graph,
 * and before the graph is used.
 *
 * @param graph resource graph
 * @return 0 if successful
 * @return RK_ERR if the graph could not be initialized
 */
int rk_init(struct rk_graph *graph);

#endif /* RESOURCE_KHAN_H_ */

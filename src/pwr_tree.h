/**
 * @file pwr_tree.h
 * @brief Dynamic power resource manager.
 * @author Philipp Schilk 2023
 * https://github.com/schilkp/pwr_tree
 */
#ifndef PWR_TREE_H_
#define PWR_TREE_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef PWR_TREE_USE_CUSTOM_CONF
#include "pwr_tree_conf_custom.h"
#else
#include "pwr_tree_conf.h"
#endif /* PWR_TREE_USE_CUSTOM_CONF */

/**
 * @brief A power tree.
 * Must be initialised with a pointer to an array containing pointers to all nodes,
 * the length of said array (number of nodes), and a pointer to the root node.
 */
struct pt {
  /** @brief Array of all nodes. */
  struct pt_node **nodes;

  /** @brief Number of nodes in this tree. */
  size_t node_count;

  /** @brief Root of the tree. */
  struct pt_node *root;

  /** @brief Scratch data used by implentation. Initialize to zero. */
  struct pt_node *ll_topo_tail;
};

// Scratch data used by implentation.
struct pt_ctx {
  struct pt_node *ll_trv;
  struct pt_node *ll_topo_next;
  struct pt_node *ll_topo_prev;
};

/**
 * @brief A node in the power tree
 * Represents an automatically managed resource, such as a regulator or power switch.
 */
struct pt_node {
  /** @brief Node name/indentifier.  */
  char name[PWR_TREE_MAX_NAME_LEN + 1];

  /**
   * @brief Current state of this node.
   * @warning Do not modify directly. Automatically managed by enable_client/disable_client funcs.
   * @note If initialized as 'enabled', all parent nodes until the root must also be 'enabled'.
   *
   * Initialize to actual state of managed resource.
   *
   * If initialized as 'enabled', ensure that at at least one (possibly) indirect client is also
   * enabled. Otherwise this resource, although not required, will not be disabled until a client
   * which depends on it is enabled and disabled again.
   */
  bool enabled;

  /**
   * @brief Previous state of this node
   * @warning only valid during cb_update call.
   * Initialisation does not matter.
   */
  bool previous_state;

  /**
   * @brief Previous return value of the node's callback.
   * @warning only valid during cb_update call.
   * Initialisation does not matter.
   */
  int previous_cb_return;

  /**
   * @brief Update callback
   * @note Optional.
   * @param Pointer to node being updated.
   * Called when node's state changes, or any dependent client (direct or indirect) was changed.
   *
   * Use self->enabled to determine if this resources should be turned on or off.
   * self->previous_state can be used to determine if this node's state was changed during this tree
   * update.
   *
   * Note that the state of any dependents of this note are already updated once this function is
   * called, But the state of the parent may not necessarily be.
   *
   * @return 0 if update successful, negative otherwise.
   */
  int (*cb_update)(const struct pt_node *self);

  /**
   * @brief Number of parent nodes
   * @note May only be 0 for root node.
   */
  uint32_t parent_count;

  /**
   * @brief Pointer to parent nodes.
   * @note May only be empty for the root node
   */
  struct pt_node *parents[PWR_TREE_MAX_PARENTS];

  /** @brief Number of children nodes. */
  uint32_t child_count;

  /** @brief Pointer to child nodes. */
  struct pt_node *children[PWR_TREE_MAX_CHILDREN];

  /** @brief Number of clients. */
  uint32_t client_count;

  /** @brief Pointer to clients. */
  struct pt_client *clients[PWR_TREE_MAX_CHILDREN];

  /** @brief Scratch data used by implentation. Initialize to zero. */
  struct pt_ctx ctx;
};

/** @brief A client that requires power from some tree nodes */
struct pt_client {
  /** @brief Client name/identifier.  */
  char name[PWR_TREE_MAX_NAME_LEN + 1];

  /**
   * @brief The state of the client.
   * @warning Do not modify directly. Automatically managed by enable_client/disable_client funcs.
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
  struct pt_node *parents[PWR_TREE_MAX_PARENTS];
};

/**
 * @brief Enable a client in the power tree.
 * This ensures that all resources that it depends on are enabled from the root down.
 *
 * @param pt power tree.
 * @param client client to be enabled.
 * @return 0 if successful, an error code returned by a node's cb_update callback otherwise.
 */
int pt_enable_client(struct pt *pt, struct pt_client *client);

/**
 * @brief Disable a client in the power tree.
 * This disables all resource from the client upwards that are no longer required.
 *
 * @param pt power tree.
 * @return 0 if successful, an error code returned by a node's cb_update callback otherwise.
 */
int pt_disable_client(struct pt *pt, struct pt_client *client);

/**
 * @brief Attempt to optimize the power tree.
 * Scans the whole power tree for nodes that are enabled although they have no active dependants.
 * This may happen if a node's callback returns a non-zero value, indicating an error. Any
 * nodes that are found to be in such a state are disabled.
 *
 * @param pt power tree.
 * @return 0 if successful, an error code returned by a node's cb_update callback otherwise.
 */
int pt_optimise(struct pt *pt);

/**
 * @brief Add a child node to a node.
 * Updates the child's parent pointer, and the parent's child_count and children pointers.
 *
 * @warning This un-initialises the tree. Must call pt_init() before using the tree.
 * @param node node to receive new child
 * @param child child to be added
 */
void pt_node_add_child(struct pt_node *node, struct pt_node *child);

/**
 * @brief Add a client to a node.
 * Updates the client's parent pointer, and the parent's client_count and client pointers.
 *
 * @warning This un-initialises the tree. Must call pt_init() before using the tree.
 * @param node node to receive new child
 * @param child child to be added
 */
void pt_node_add_client(struct pt_node *node, struct pt_client *client);

/**
 * @brief Initialise a power tree.
 * Must be called after all nodes and clients have been added to the tree,
 * and before the tree is used.
 *
 * @param pt power tree.
 * @return 0 if successful, 1 if tree could not be initialised.
 */
int pt_init(struct pt *pt);

#endif /* PWR_TREE_H_ */
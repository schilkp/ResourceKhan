/**
 * @file resource_khan.c
 * @brief Graph resource manager.
 * @author Philipp Schilk 2023
 * https://github.com/schilkp/ResourceKhan
 */
#include "resource_khan.h"
#include <string.h>

// ==== Private Prototypes =====================================================

static void reset_ctx_all(struct rk_graph *pt);
static void reset_ctx_ll_trv(struct rk_graph *pt);
static int enable_node(struct rk_graph *pt, struct rk_node *node);
static int optimize_node(struct rk_graph *pt, struct rk_node *node);
static int update_node(struct rk_node *node, bool new_state);
static bool has_active_dependant(struct rk_node *node);

#define RK_ASSERT_NO_NULLPTR(_pt_)                                                                                     \
  do {                                                                                                                 \
    RK_ASSERT((_pt_) != 0);                                                                                            \
    RK_ASSERT((_pt_)->nodes != 0);                                                                                     \
    RK_ASSERT((_pt_)->root != 0);                                                                                      \
  } while (0)

#define RK_ON_OFF(_i_) ((_i_) ? "ON" : "OFF")

// ==== Public Functions =======================================================

int rk_enable_client(struct rk_graph *pt, struct rk_client *client) {
  RK_ASSERT_NO_NULLPTR(pt);
  RK_ASSERT(client != 0);

  for (size_t i = 0; i < client->parent_count; i++) {
    int err = enable_node(pt, client->parents[i]);
    if (err) return err;
  }

  client->enabled = true;

  return 0;
}

int rk_disable_client(struct rk_graph *pt, struct rk_client *client) {
  RK_ASSERT_NO_NULLPTR(pt);
  RK_ASSERT(client != 0);

  client->enabled = false;

  for (size_t i = 0; i < client->parent_count; i++) {
    int err = optimize_node(pt, client->parents[i]);
    if (err) return err;
  }

  return 0;
}

int rk_optimize(struct rk_graph *pt) {
  RK_ASSERT_NO_NULLPTR(pt);

  // Traverse in reverse-topological order, disabling all nodes if they no longer have
  // any active dependent:
  struct rk_node *node = pt->ll_topo_tail;

  while (node != 0) {

    int err = update_node(node, has_active_dependant(node));
    if (err) return err;

    node = node->ctx.ll_topo_prev;
  }

  return 0;
}

void rk_node_add_child(struct rk_node *node, struct rk_node *child) {
  RK_ASSERT(node != 0);
  RK_ASSERT(node->child_count < RK_MAX_CHILDREN);
  RK_ASSERT(child != 0);
  RK_ASSERT(child->parent_count < RK_MAX_PARENTS);

  node->children[node->child_count] = child;
  node->child_count++;
  child->parents[child->parent_count] = node;
  child->parent_count++;
}

void rk_node_add_client(struct rk_node *node, struct rk_client *client) {
  RK_ASSERT(node != 0);
  RK_ASSERT(node->client_count < RK_MAX_CHILDREN);
  RK_ASSERT(client != 0);
  RK_ASSERT(client->parent_count < RK_MAX_PARENTS);

  node->clients[node->client_count] = client;
  node->client_count++;
  client->parents[client->parent_count] = node;
  client->parent_count++;
}

int rk_init(struct rk_graph *pt) {
  RK_ASSERT_NO_NULLPTR(pt);

  reset_ctx_all(pt);

  // == Topological sort (Kahn's algorithm): ==

  // The "topo" doubly-linked-list (stored in the nodes themselves) serves
  // as the L list, storing all sorted nodes:
  struct rk_node *ll_topo_tail = 0;

  // The "trv" traversal single-linked list (also stored in the nodes themselves)
  // serves as the S list, storing nodes that are not yet in L, but already have
  // no parents when ignoring all nodes already in L. They are next candidates
  // to be added to L.
  struct rk_node *trv_head = pt->root;
  struct rk_node *trv_tail = pt->root;

  while (trv_head != 0) {
    // Append trv_head to topo list (L):
    if (ll_topo_tail == 0) {
      ll_topo_tail = trv_head;
    } else {
      trv_head->ctx.ll_topo_prev = ll_topo_tail;
      ll_topo_tail->ctx.ll_topo_next = trv_head;
      ll_topo_tail = trv_head;
    }

    // Check all children nodes. If they no longer have any parents outside of
    // L, add them to S:
    for (size_t child_idx = 0; child_idx < trv_head->child_count; child_idx++) {
      struct rk_node *node_to_check = trv_head->children[child_idx];

      // Validate that this child is not inside L, which can only happen if the
      // graph is not acyclic:
      if (node_to_check->ctx.ll_topo_next != 0 || ll_topo_tail == node_to_check) {
        // Cyclic graph. Abort.
        RK_ERR("Graph contains cycle, likely involving node %s!", node_to_check->name);
        return 1;
      }

      bool has_parent_outside_l = false;
      for (size_t parent_idx = 0; parent_idx < node_to_check->parent_count; parent_idx++) {
        struct rk_node *parent = node_to_check->parents[parent_idx];

        if (parent->ctx.ll_topo_next == 0 && parent != ll_topo_tail) { // Check if not in L.
          has_parent_outside_l = true;
          break;
        }
      }

      // Add to trv list (S):
      if (!has_parent_outside_l) {
        // Check if node_to_check is already in S, which can happen
        // if the node contains redundant parallel edges between nodes:
        if (node_to_check->ctx.ll_trv != 0 || node_to_check == trv_tail) {
          continue;
        }
        trv_tail->ctx.ll_trv = node_to_check;
        trv_tail = node_to_check;
      }
    }

    // Advance traversal list (S), effectively removing the current node from it:
    trv_head = trv_head->ctx.ll_trv;
  }

  // == Validate that graph is acyclic & connected ==
  size_t topo_count = 0;
  struct rk_node *ll_topo_head = pt->root;

  while (ll_topo_head != 0) {
    topo_count++;
    ll_topo_head = ll_topo_head->ctx.ll_topo_next;
  }

  if (topo_count != pt->node_count) {
    RK_ERR("Graph malformed. It may contain nodes not in the node list, be cyclic or be disconnected. "
           "Topological list contains %zd elements, but graph claims to have %zd nodes!",
           topo_count, pt->node_count);
    return 1;
  }

  pt->ll_topo_tail = ll_topo_tail;

  return 0;
}

// ==== Private Functions ======================================================

static void reset_ctx_all(struct rk_graph *pt) {
  for (size_t i = 0; i < pt->node_count; i++) {
    struct rk_ctx *ctx = &(pt->nodes[i]->ctx);
    memset(ctx, 0, sizeof(*ctx));
  }
}

static void reset_ctx_ll_trv(struct rk_graph *pt) {
  for (size_t i = 0; i < pt->node_count; i++) {
    struct rk_ctx *ctx = &(pt->nodes[i]->ctx);
    ctx->ll_trv = 0;
  }
}

static int enable_node(struct rk_graph *pt, struct rk_node *node) {

  // == STEP 1: Flood from node up to root to discover all nodes which require an update ==

  reset_ctx_ll_trv(pt);

  // "Traverse" linked-list:
  struct rk_node *trv_head = node;
  struct rk_node *trv_tail = node;

  while (trv_head != 0) {

    for (size_t i = 0; i < trv_head->parent_count; i++) {
      struct rk_node *parent = trv_head->parents[i];
      RK_ASSERT(parent != 0);

      // Check if parent is already in "traverse" linked list:
      if (parent->ctx.ll_trv == 0 && parent != trv_tail) {
        // Parent not already in list. Append:
        trv_tail->ctx.ll_trv = parent;
        trv_tail = parent;
      }
    }

    trv_head = trv_head->ctx.ll_trv;
  }

  // == STEP 2: Traverse in topological order, enabling all nodes that were traversed in step 1 ==

  struct rk_node *topo_head = pt->root;
  while (topo_head != 0) {

    // Check if this node is in the "traverse" list:
    if (topo_head->ctx.ll_trv != 0 || topo_head == trv_tail) {
      int err = update_node(topo_head, true);
      if (err) return err;
    }

    topo_head = topo_head->ctx.ll_topo_next;
  }

  return 0;
}

static int optimize_node(struct rk_graph *pt, struct rk_node *node) {

  // == STEP 1: Flood from node up to root to discover all nodes which require an update ==

  reset_ctx_ll_trv(pt);

  // "Traverse" linked-list:
  struct rk_node *trv_head = node;
  struct rk_node *trv_tail = node;

  while (trv_head != 0) {

    for (size_t i = 0; i < trv_head->parent_count; i++) {
      struct rk_node *parent = trv_head->parents[i];
      RK_ASSERT(parent != 0);

      // Check if parent is already in "traverse" linked list:
      if (parent->ctx.ll_trv == 0 && parent != trv_tail) {
        // Parent not already in list. Append:
        trv_tail->ctx.ll_trv = parent;
        trv_tail = parent;
      }
    }

    trv_head = trv_head->ctx.ll_trv;
  }

  // == STEP 2: Traverse in reverse-topological order, updating all nodes ==

  struct rk_node *topo_tail = pt->ll_topo_tail;
  while (topo_tail != 0) {

    // Check if this node is in the "traverse" list:
    if (topo_tail->ctx.ll_trv != 0 || topo_tail == trv_tail) {
      int err = update_node(topo_tail, has_active_dependant(topo_tail));
      if (err) return err;
    }

    topo_tail = topo_tail->ctx.ll_topo_prev;
  }

  return 0;
}

static int update_node(struct rk_node *node, bool new_state) {
  node->desired_state = new_state;

  if (node->desired_state != node->state) {
    RK_INF("%s: %s -> %s", node->name, RK_ON_OFF(node->state), RK_ON_OFF(node->desired_state));
  }

  if (node->cb_update != 0) {
    // Attempt to update node using callback:
    int err = node->cb_update(node);
    node->previous_cb_return = err;
    if (err) {
      RK_ERR("%s: Callback returned error %i! Graph in non-optimal state. Node left %s.", node->name, err,
             RK_ON_OFF(node->state));
      return err;
    } else {
      node->state = new_state;
    }
  } else {
    // Update cannot fail if there is no update.
    node->state = new_state;
  }

  return 0;
}

// Check if a given node has any direct children or clients that are active.
static bool has_active_dependant(struct rk_node *node) {
  for (size_t i = 0; i < node->child_count; i++) {
    RK_ASSERT(node->children[i] != 0);
    if (node->children[i]->state) {
      return true;
    }
  }
  for (size_t i = 0; i < node->client_count; i++) {
    RK_ASSERT(node->clients[i] != 0);
    if (node->clients[i]->enabled) {
      return true;
    }
  }
  return false;
}

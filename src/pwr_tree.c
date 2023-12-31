/**
 * @file pwr_tree.c
 * @brief Dynamic power resource manager.
 * @author Philipp Schilk 2023
 * https://github.com/schilkp/pwr_tree
 *
 */
#include "pwr_tree.h"
#include <string.h>

// ==== Private Prototypes =====================================================

static void reset_ctx_all(struct pt *pt);
static void reset_ctx_ll_trv(struct pt *pt);
static int enable_node(struct pt *pt, struct pt_node *node);
static int update_node(struct pt_node *node, bool new_state);
static bool has_active_dependant(struct pt_node *node);

static int inner_disable(struct pt_node *node, uint32_t current_depth);
static int inner_optimise(struct pt_node *node, uint32_t current_depth);

// ==== Public Functions =======================================================

int pt_enable_client(struct pt *tree, struct pt_client *client) {
  PWR_TREE_ASSERT(tree != 0);
  PWR_TREE_ASSERT(client != 0);

  for (size_t i = 0; i < client->parent_count; i++) {
    int err = enable_node(tree, client->parents[i]);
    if (err) return err;
  }

  client->enabled = true;

  return 0;
}

int pt_disable_client(struct pt *tree, struct pt_client *client) {
  PWR_TREE_ASSERT(tree != 0);
  PWR_TREE_ASSERT(client != 0);

  client->enabled = false;

  for (size_t i = 0; i < client->parent_count; i++) {
    int err = inner_disable(client->parents[i], 0);
    if (err) return err;
  }

  return 0;
}

int pt_optimise(struct pt_node *root) {
  PWR_TREE_ASSERT(root != 0);
  return inner_optimise(root, 0);
}

void pt_node_add_child(struct pt_node *node, struct pt_node *child) {
  PWR_TREE_ASSERT(node != 0);
  PWR_TREE_ASSERT(node->child_count < PWR_TREE_MAX_CHILDREN);
  PWR_TREE_ASSERT(child != 0);
  PWR_TREE_ASSERT(child->parent_count < PWR_TREE_MAX_PARENTS);

  node->children[node->child_count] = child;
  node->child_count++;
  child->parents[child->parent_count] = node;
  child->parent_count++;
}

void pt_node_add_client(struct pt_node *node, struct pt_client *client) {
  PWR_TREE_ASSERT(node != 0);
  PWR_TREE_ASSERT(node->client_count < PWR_TREE_MAX_CHILDREN);
  PWR_TREE_ASSERT(client != 0);
  PWR_TREE_ASSERT(client->parent_count < PWR_TREE_MAX_PARENTS);

  node->clients[node->client_count] = client;
  node->client_count++;
  client->parents[client->parent_count] = node;
  client->parent_count++;
}

int pt_init(struct pt *tree) { return 0; }

// ==== Private Functions ======================================================

static void reset_ctx_all(struct pt *pt) {
  for (size_t i = 0; i < pt->count; i++) {
    struct pt_ctx *ctx = &(pt->nodes[i]->ctx);
    memset(ctx, 0, sizeof(*ctx));
  }
}

static void reset_ctx_ll_trv(struct pt *pt) {
  for (size_t i = 0; i < pt->count; i++) {
    struct pt_ctx *ctx = &(pt->nodes[i]->ctx);
    ctx->ll_trv = 0;
  }
}

static int enable_node(struct pt *pt, struct pt_node *node) {
  // PWR_TREE_ASSERT(node != 0);
  //
  // reset_ctx_all(pt);
  //
  // // == STEP 1: Determine update order ==
  //
  // // Traverse upwards by flooding.
  // // "Traverse" linked-list:
  // struct pt_node *trv_head = node;
  // struct pt_node *trv_tail = node;
  // node->ctx.req_upd = true;
  //
  // while (trv_head != 0) {
  //
  //   for (size_t i = 0; i < trv_head->parent_count; i++) {
  //     struct pt_node *parent = trv_head->parents[i];
  //     PWR_TREE_ASSERT(parent != 0);
  //
  //     // Check if parent is already in "traverese" linked list:
  //     if (parent->ctx.ll_trv == 0 && parent != trv_tail) {
  //       parent->ctx.req_upd = true; // Must be updated.
  //       trv_tail->ctx.ll_trv = parent;
  //       trv_tail = parent;
  //     }
  //   }
  //
  //   trv_head = trv_head->ctx.ll_trv;
  // }
  //
  // // == STEP 2: Traverse downwards from root to construct the update-order list: ==
  //
  // // "Traverse" linked-list:
  // reset_ctx_ll_trv(pt);
  // trv_head = pt->root;
  // trv_tail = pt->root;
  //
  // // "Update" linked-list:
  // struct pt_node *upd_head = node;
  // struct pt_node *upd_tail = node;
  //
  // // Traverse by flooding from :
  // while (trv_head != 0) {
  //
  //   for (size_t i = 0; i < trv_head->child_count; i++) {
  //     struct pt_node *child = trv_head->children[i];
  //     PWR_TREE_ASSERT(child != 0);
  //   }
  //
  //   trv_head = trv_head->ctx.ll_trv;
  // }
  //
  // // node = upd_head;
  // // while (node != 0) {
  // //   int err = update_node(node, true);
  // //   if (err) return err;
  // //   node = node->ctx.upd;
  // // }
  //
  return 0;
}

static int update_node(struct pt_node *node, bool new_state) {
  node->previous_state = node->enabled;
  node->enabled = new_state;

  if (node->previous_state != node->enabled) {
    PWR_TREE_INF("%s: %d -> %d", node->name, node->previous_state, node->enabled);
  }

  if (node->cb_update != 0) {
    int err = node->cb_update(node);
    node->previous_cb_return = err;
    if (err) {
      PWR_TREE_ERR("%s: Callback returned error %i! Tree in non-optiomal state.", node->name, err);
      node->enabled = node->previous_state;
      return err;
    }
  }

  return 0;
}

static bool has_active_dependant(struct pt_node *node) {
  PWR_TREE_ASSERT(node != 0);
  for (size_t i = 0; i < node->child_count; i++) {
    PWR_TREE_ASSERT(node->children[i] != 0);
    if (node->children[i]->enabled) {
      return true;
    }
  }
  for (size_t i = 0; i < node->client_count; i++) {
    PWR_TREE_ASSERT(node->clients[i] != 0);
    if (node->clients[i]->enabled) {
      return true;
    }
  }
  return false;
}

//// TO REMOVE
#define PWR_TREE_MAX_DEPTH 10
static int inner_disable(struct pt_node *node, uint32_t current_depth) {
  PWR_TREE_ASSERT(node != 0);
  PWR_TREE_ASSERT(current_depth != PWR_TREE_MAX_DEPTH);
  if (current_depth == PWR_TREE_MAX_DEPTH) {
    return -1;
  }

  bool new_state = has_active_dependant(node);
  node->previous_state = node->enabled;
  node->enabled = new_state;
  if (node->previous_state != node->enabled) {
    PWR_TREE_INF("%s: %d -> %d", node->name, node->previous_state, node->enabled);
  }
  if (node->cb_update != 0) {
    int err = node->cb_update(node);
    node->previous_cb_return = err;
    if (err) {
      PWR_TREE_ERR("%s: Callback returned error %i! Tree in non-optiomal state.", node->name, err);
      node->enabled = node->previous_state;
      return err;
    }
  }

  for (size_t i = 0; i < node->parent_count; i++) {
    int err = inner_disable(node->parents[i], current_depth + 1);
    if (err) return err;
  }

  return 0;
}

static int inner_optimise(struct pt_node *node, uint32_t current_depth) {
  PWR_TREE_ASSERT(node != 0);
  PWR_TREE_ASSERT(current_depth != PWR_TREE_MAX_DEPTH);
  if (current_depth == PWR_TREE_MAX_DEPTH) {
    return -1;
  }

  if (node->enabled && !has_active_dependant(node)) {
    PWR_TREE_INF("Node %s enabled without active dependants!", node->name);
    int err = inner_disable(node, 0);
    if (err) return err;
  }

  for (size_t i = 0; i < node->child_count; i++) {
    int err = inner_optimise(node->children[i], current_depth + 1);
    if (err) return err;
  }

  return 0;
}

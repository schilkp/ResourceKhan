/**
 * @file pwr_tree.c
 * @brief Dynamic power resource manager.
 * @author Philipp Schilk 2023
 * https://github.com/schilkp/pwr_tree
 *
 */
#include "pwr_tree.h"

#define PWR_TREE_MAX_DEPTH 10

// ==== Private Prototypes =====================================================

static int inner_enable(struct pt_node *node, uint32_t current_depth);
static int inner_disable(struct pt_node *node, uint32_t current_depth);
static int inner_optimise(struct pt_node *node, uint32_t current_depth);
static bool has_active_dependant(struct pt_node *node);

// ==== Public Functions =======================================================

int pt_enable_client(struct pt *pt, struct pt_client *client) {
  (void)pt;

  for (size_t i = 0; i < client->parent_count; i++) {
    int err = inner_enable(client->parents[i], 0);
    if (err) return err;
  }

  client->enabled = true;

  return 0;
}

int pt_disable_client(struct pt *pt, struct pt_client *client) {
  (void)pt;

  client->enabled = false;

  for (size_t i = 0; i < client->parent_count; i++) {
    int err = inner_disable(client->parents[i], 0);
    if (err) return err;
  }

  return 0;
}

int pt_optimise(struct pt *pt) { return inner_optimise(pt->root, 0); }

void pt_node_add_child(struct pt_node *node, struct pt_node *child) {
  PWR_TREE_ASSERT(node->child_count < PWR_TREE_MAX_CHILDREN);
  PWR_TREE_ASSERT(child->parent_count < PWR_TREE_MAX_PARENTS);

  node->children[node->child_count] = child;
  node->child_count++;
  child->parents[child->parent_count] = node;
  child->parent_count++;
}

void pt_node_add_client(struct pt_node *node, struct pt_client *client) {
  PWR_TREE_ASSERT(node->client_count < PWR_TREE_MAX_CHILDREN);
  PWR_TREE_ASSERT(client->parent_count < PWR_TREE_MAX_PARENTS);

  node->clients[node->client_count] = client;
  node->client_count++;
  client->parents[client->parent_count] = node;
  client->parent_count++;
}

int pt_init(struct pt *pt) {
  // Attempt to optimise. Catches loops:
  return pt_optimise(pt);
}

// ==== Private Functions ======================================================

static int inner_enable(struct pt_node *node, uint32_t current_depth) {
  if (current_depth == PWR_TREE_MAX_DEPTH) {
    return -1;
  }

  for (size_t i = 0; i < node->parent_count; i++) {
    int err = inner_enable(node->parents[i], current_depth + 1);
    if (err) return err;
  }

  bool new_state = true;
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

static int inner_disable(struct pt_node *node, uint32_t current_depth) {
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

static bool has_active_dependant(struct pt_node *node) {
  for (size_t i = 0; i < node->child_count; i++) {
    if (node->children[i]->enabled) {
      return true;
    }
  }
  for (size_t i = 0; i < node->client_count; i++) {
    if (node->clients[i]->enabled) {
      return true;
    }
  }
  return false;
}

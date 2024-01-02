#include "resource_khan.h"

#define RK_MAX_DEPTH 10

// ==== Private Prototypes =====================================================

static int inner_enable(struct rk_node *node, uint32_t current_depth);
static int inner_disable(struct rk_node *node, uint32_t current_depth);
static int inner_optimize(struct rk_node *node, uint32_t current_depth);
static int update_node(struct rk_node *node, bool new_state);
static bool has_active_dependant(struct rk_node *node);

// ==== Public Functions =======================================================

int rk_enable_client(struct rk_graph *pt, struct rk_client *client) {
  (void)pt;

  for (size_t i = 0; i < client->parent_count; i++) {
    int err = inner_enable(client->parents[i], 0);
    if (err) return err;
  }

  client->enabled = true;

  return 0;
}

int rk_disable_client(struct rk_graph *pt, struct rk_client *client) {
  (void)pt;

  client->enabled = false;

  for (size_t i = 0; i < client->parent_count; i++) {
    int err = inner_disable(client->parents[i], 0);
    if (err) return err;
  }

  return 0;
}

int rk_optimize(struct rk_graph *pt) { return inner_optimize(pt->root, 0); }

void rk_node_add_child(struct rk_node *node, struct rk_node *child) {
  RK_ASSERT(node->child_count < RK_MAX_CHILDREN);
  RK_ASSERT(child->parent_count < RK_MAX_PARENTS);

  node->children[node->child_count] = child;
  node->child_count++;
  child->parents[child->parent_count] = node;
  child->parent_count++;
}

void rk_node_add_client(struct rk_node *node, struct rk_client *client) {
  RK_ASSERT(node->client_count < RK_MAX_CHILDREN);
  RK_ASSERT(client->parent_count < RK_MAX_PARENTS);

  node->clients[node->client_count] = client;
  node->client_count++;
  client->parents[client->parent_count] = node;
  client->parent_count++;
}

int rk_init(struct rk_graph *pt) {
  // Attempt to optimize. Catches loops:
  return rk_optimize(pt);
}

// ==== Private Functions ======================================================

static int inner_enable(struct rk_node *node, uint32_t current_depth) {
  if (current_depth == RK_MAX_DEPTH) {
    return -1;
  }

  for (size_t i = 0; i < node->parent_count; i++) {
    int err = inner_enable(node->parents[i], current_depth + 1);
    if (err) return err;
  }

  return update_node(node, true);
}

static int inner_disable(struct rk_node *node, uint32_t current_depth) {
  if (current_depth == RK_MAX_DEPTH) {
    return -1;
  }

  bool new_state = has_active_dependant(node);

  int err = update_node(node, new_state);
  if (err) return err;

  for (size_t i = 0; i < node->parent_count; i++) {
    int err = inner_disable(node->parents[i], current_depth + 1);
    if (err) return err;
  }

  return 0;
}

static int inner_optimize(struct rk_node *node, uint32_t current_depth) {
  if (current_depth == RK_MAX_DEPTH) {
    return -1;
  }

  if (node->state && !has_active_dependant(node)) {
    RK_INF("Node %s enabled without active dependents!", node->name);
    int err = inner_disable(node, 0);
    if (err) return err;
  }

  for (size_t i = 0; i < node->child_count; i++) {
    int err = inner_optimize(node->children[i], current_depth + 1);
    if (err) return err;
  }

  return 0;
}

static int update_node(struct rk_node *node, bool new_state) {
  node->desired_state = new_state;

  if (node->desired_state != node->state) {
    RK_INF("%s: %d -> %d", node->name, node->state, node->desired_state);
  }

  if (node->cb_update != 0) {
    // Attempt to update note using callback:
    int err = node->cb_update(node);
    node->previous_cb_return = err;
    if (err) {
      RK_ERR("%s: Callback returned error %i! Graph in non-optimal state. Node left %d.", node->name, err, node->state);
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

static bool has_active_dependant(struct rk_node *node) {
  for (size_t i = 0; i < node->child_count; i++) {
    if (node->children[i]->state) {
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

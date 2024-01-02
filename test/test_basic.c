#include "stdlib.h"
#include "string.h"
#include "unity.h"
#include "unity_internals.h"
#include "utils.h"

#include "resource_khan.h"

bool EXPECT_INTERNAL_ASSERT = false;

// ======== Resource Graph =====================================================================

//       n_root
//    +----+----+
//    |         |
//   n_a       n_b
//    |         +-----+-+-+
//   c_a        |     |   |
//              |   c_b1 c_b2
//             n_c
//              |
//             c_c

int mock_cb_update(const struct rk_node *self);

// NODES:
struct rk_node n_root = {.name = "root", .parent_count = 0, .cb_update = mock_cb_update};
struct rk_node n_a = {.name = "n_a", .cb_update = mock_cb_update};
struct rk_node n_b = {.name = "n_b", .cb_update = mock_cb_update};
struct rk_node n_c = {.name = "n_c", .cb_update = mock_cb_update};

struct rk_node *nodes[] = {&n_root, &n_a, &n_b, &n_c};
struct rk_graph pt = {.nodes = nodes, .node_count = sizeof(nodes) / sizeof(nodes[0]), .root = &n_root};

// CLIENTS:
struct rk_client c_a = {.name = "c_a"};
struct rk_client c_b1 = {.name = "c_b1"};
struct rk_client c_b2 = {.name = "c_b2"};
struct rk_client c_c = {.name = "c_c"};

struct rk_client *clients[] = {&c_a, &c_b1, &c_b2, &c_c};

void assert_graph_state_optimal(void) {
  assert_graph_state_legal(&pt);
  ASSERT_NODE(n_root, c_a.enabled || c_b1.enabled || c_b2.enabled || c_c.enabled);
  ASSERT_NODE(n_a, c_a.enabled);
  ASSERT_NODE(n_b, c_b1.enabled || c_b2.enabled || c_c.enabled);
  ASSERT_NODE(n_c, c_c.enabled);
}

int mock_cb_update(const struct rk_node *self) {
  (void)self;
  assert_graph_state_legal(&pt);
  return 0;
}

void init_graph(void) {
  rk_node_add_child(&n_root, &n_a);
  rk_node_add_child(&n_root, &n_b);
  rk_node_add_child(&n_b, &n_c);

  rk_node_add_client(&n_a, &c_a);
  rk_node_add_client(&n_b, &c_b1);
  rk_node_add_client(&n_b, &c_b1);
  rk_node_add_client(&n_c, &c_c);
}

// ======== Tests ==================================================================================

void test_basic_1(void) {

  ASSERT_OK(rk_init(&pt));

  assert_graph_state_optimal();
  ASSERT_OK(rk_enable_client(&pt, &c_a));
  assert_graph_state_optimal();
  ASSERT_OK(rk_enable_client(&pt, &c_b1));
  assert_graph_state_optimal();
  ASSERT_OK(rk_enable_client(&pt, &c_b2));
  assert_graph_state_optimal();
  ASSERT_OK(rk_enable_client(&pt, &c_c));
  assert_graph_state_optimal();
  ASSERT_OK(rk_disable_client(&pt, &c_c));
  assert_graph_state_optimal();
  ASSERT_OK(rk_disable_client(&pt, &c_b2));
  assert_graph_state_optimal();
  ASSERT_OK(rk_disable_client(&pt, &c_b1));
  assert_graph_state_optimal();
  ASSERT_OK(rk_disable_client(&pt, &c_a));
  assert_graph_state_optimal();
}

void test_basic_2(void) {

  ASSERT_OK(rk_init(&pt));

  assert_graph_state_optimal();
  ASSERT_OK(rk_enable_client(&pt, &c_b1));
  assert_graph_state_optimal();
  ASSERT_OK(rk_enable_client(&pt, &c_c));
  assert_graph_state_optimal();
  ASSERT_OK(rk_disable_client(&pt, &c_b1));
  assert_graph_state_optimal();
  ASSERT_OK(rk_disable_client(&pt, &c_c));
  assert_graph_state_optimal();
}

void test_basic_3(void) {

  ASSERT_OK(rk_init(&pt));

  assert_graph_state_optimal();
  ASSERT_OK(rk_enable_client(&pt, &c_c));
  assert_graph_state_optimal();
  ASSERT_OK(rk_disable_client(&pt, &c_c));
  assert_graph_state_optimal();
}

void test_basic_optimize_1(void) {

  ASSERT_OK(rk_init(&pt));

  n_root.state = true;
  n_a.state = true;

  assert_graph_state_legal(&pt);

  rk_optimize(&pt);
  assert_graph_state_optimal();
}

// ======== Main ===================================================================================

void setUp(void) {
  for (size_t i = 0; i < pt.node_count; i++) {
    pt.nodes[i]->state = false;
  }
  for (size_t i = 0; i < (sizeof(clients) / sizeof(clients[0])); i++) {
    clients[i]->enabled = false;
  }
}

void tearDown(void) {}

int main(void) {
  init_graph();
  UNITY_BEGIN();
  RUN_TEST(test_basic_1);
  RUN_TEST(test_basic_2);
  RUN_TEST(test_basic_3);
  RUN_TEST(test_basic_optimize_1);
  return UNITY_END();
}

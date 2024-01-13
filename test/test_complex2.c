#include "stdlib.h"
#include "string.h"
#include "unity.h"
#include "unity_internals.h"
#include "utils.h"

#include "resource_khan.h"

// ======== Resource Graph =========================================================================

//
//              n_root
//                |
//           +----+----+
//           |         |
//          n_a       n_b
//           |         |
//       +---+---+     |
//       |       |     |
//      n_d     n_c    |
//      | |      |     |
//      | |      +-+ +-+
//      | |        | |
//      | |        n_e
//      | |         |
//      | +---+ +---+
//      |     | |
//      +-----n_f
//            |||
//            n_g
//
// Very similar to complex1, but there are two parallel connections between n_d and n_f, and
// three parallel connections between n_f and n_g.
//
// All nodes have a single, identically named client (n_root -> c_root, n_a -> c_a etc),
// except n_g, which has two (c_g1, c_g2). In addition, there is c_many, which has
// parents n_a, n_d, and n_e.

int mock_cb_update(const struct rk_node *self);

// NODES:
struct rk_node n_root = {.name = "n_root", .cb_update = mock_cb_update};
struct rk_node n_a = {.name = "n_a", .cb_update = mock_cb_update};
struct rk_node n_b = {.name = "n_b", .cb_update = mock_cb_update};
struct rk_node n_c = {.name = "n_c", .cb_update = mock_cb_update};
struct rk_node n_d = {.name = "n_d", .cb_update = mock_cb_update};
struct rk_node n_e = {.name = "n_e", .cb_update = mock_cb_update};
struct rk_node n_f = {.name = "n_f", .cb_update = mock_cb_update};
struct rk_node n_g = {.name = "n_g", .cb_update = mock_cb_update};

struct rk_node *nodes[] = {&n_root, &n_a, &n_b, &n_c, &n_d, &n_e, &n_f, &n_g};
struct rk_graph pt = {.nodes = nodes, .node_count = sizeof(nodes) / sizeof(nodes[0]), .root = &n_root};

// CLIENTS:
struct rk_client c_root = {.name = "c_root"};
struct rk_client c_a = {.name = "c_a"};
struct rk_client c_b = {.name = "c_b"};
struct rk_client c_c = {.name = "c_c"};
struct rk_client c_d = {.name = "c_d"};
struct rk_client c_e = {.name = "c_e"};
struct rk_client c_f = {.name = "c_f"};
struct rk_client c_g1 = {.name = "c_g1"};
struct rk_client c_g2 = {.name = "c_g2"};
struct rk_client c_many = {.name = "c_many"};

struct rk_client *clients[] = {
    &c_root, &c_a, &c_b, &c_c, &c_d, &c_e, &c_f, &c_g1, &c_g2, &c_many,
};

void assert_graph_state_optimal(void) {
  assert_graph_state_legal(&pt);
  ASSERT_NODE(n_root, c_root.enabled || c_a.enabled || c_b.enabled || c_c.enabled || c_d.enabled || c_e.enabled ||
                          c_f.enabled || c_g1.enabled || c_g2.enabled || c_many.enabled);

  ASSERT_NODE(n_a, c_a.enabled || c_c.enabled || c_d.enabled || c_e.enabled || c_f.enabled || c_g1.enabled ||
                       c_g2.enabled || c_many.enabled);

  ASSERT_NODE(n_b, c_b.enabled || c_e.enabled || c_f.enabled || c_g1.enabled || c_g2.enabled || c_many.enabled);

  ASSERT_NODE(n_c, c_c.enabled || c_e.enabled || c_f.enabled || c_g1.enabled || c_g2.enabled || c_many.enabled);

  ASSERT_NODE(n_d, c_d.enabled || c_f.enabled || c_g1.enabled || c_g2.enabled || c_many.enabled);

  ASSERT_NODE(n_e, c_e.enabled || c_f.enabled || c_g1.enabled || c_g2.enabled || c_many.enabled);

  ASSERT_NODE(n_f, c_f.enabled || c_g1.enabled || c_g2.enabled);

  ASSERT_NODE(n_g, c_g1.enabled || c_g2.enabled);
}

int mock_cb_update(const struct rk_node *self) {
  (void)self;
  assert_graph_state_legal(&pt);
  return 0;
}

void init_graph(void) {

  rk_node_add_child(&n_root, &n_a);
  rk_node_add_child(&n_root, &n_b);
  rk_node_add_client(&n_root, &c_root);

  rk_node_add_child(&n_a, &n_d);
  rk_node_add_child(&n_a, &n_c);
  rk_node_add_child(&n_b, &n_e);
  rk_node_add_client(&n_a, &c_a);
  rk_node_add_client(&n_a, &c_many);
  rk_node_add_client(&n_b, &c_b);
  rk_node_add_client(&n_c, &c_c);

  rk_node_add_child(&n_d, &n_f);
  rk_node_add_child(&n_d, &n_f);
  rk_node_add_child(&n_c, &n_e);
  rk_node_add_client(&n_d, &c_d);
  rk_node_add_client(&n_d, &c_many);
  rk_node_add_client(&n_c, &c_c);

  rk_node_add_child(&n_e, &n_f);
  rk_node_add_client(&n_e, &c_e);
  rk_node_add_client(&n_e, &c_many);

  rk_node_add_child(&n_f, &n_g);
  rk_node_add_child(&n_f, &n_g);
  rk_node_add_child(&n_f, &n_g);
  rk_node_add_client(&n_f, &c_f);

  rk_node_add_client(&n_g, &c_g1);
  rk_node_add_client(&n_g, &c_g2);
}

// ======== Tests ==================================================================================

void test_complex2_1(void) {

  ASSERT_OK(rk_init(&pt));

  assert_graph_state_optimal();
  ASSERT_OK(rk_enable_client(&pt, &c_f));
  assert_graph_state_optimal();
  ASSERT_OK(rk_disable_client(&pt, &c_f));
  assert_graph_state_optimal();
}

void test_complex2_2(void) {

  ASSERT_OK(rk_init(&pt));

  assert_graph_state_optimal();
  ASSERT_OK(rk_enable_client(&pt, &c_g1));
  assert_graph_state_optimal();
  ASSERT_OK(rk_enable_client(&pt, &c_g2));
  assert_graph_state_optimal();
  ASSERT_OK(rk_disable_client(&pt, &c_g1));
  assert_graph_state_optimal();
  ASSERT_OK(rk_disable_client(&pt, &c_g2));
  assert_graph_state_optimal();
}

void test_many_parent_nodes(void) {

  ASSERT_OK(rk_init(&pt));

  assert_graph_state_optimal();
  ASSERT_OK(rk_enable_client(&pt, &c_many));
  assert_graph_state_optimal();
  ASSERT_OK(rk_disable_client(&pt, &c_many));
  assert_graph_state_optimal();
  ASSERT_OK(rk_enable_client(&pt, &c_many));
  assert_graph_state_optimal();
  ASSERT_OK(rk_enable_client(&pt, &c_e));
  assert_graph_state_optimal();
  ASSERT_OK(rk_enable_client(&pt, &c_d));
  assert_graph_state_optimal();
}

void test_complex2_optimize1(void) {

  ASSERT_OK(rk_init(&pt));

  n_root.state = true;
  n_a.state = true;
  n_d.state = true;
  n_b.state = true;

  assert_graph_state_legal(&pt);

  rk_optimize(&pt);
  assert_graph_state_optimal();
}

void test_complex2_optimize2(void) {

  ASSERT_OK(rk_init(&pt));

  for (size_t i = 0; i < pt.node_count; i++) {
    pt.nodes[i]->state = true;
  }

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
  RUN_TEST(test_complex2_1);
  RUN_TEST(test_complex2_2);
  RUN_TEST(test_many_parent_nodes);
  RUN_TEST(test_complex2_optimize1);
  RUN_TEST(test_complex2_optimize2);
  return UNITY_END();
}

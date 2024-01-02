#include "stdlib.h"
#include "string.h"
#include "unity.h"
#include "unity_internals.h"
#include "utils.h"

#include "pwr_tree.h"

bool EXPECT_INTERNAL_ASSERT = false;

// ======== Power Tree =============================================================================

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
//       |       |     |
//       |       +-+ +-+
//       |         | |
//       |         n_e
//       |          |
//       +----+ +---+
//            | |
//            n_f
//             |
//            n_g
//
// All nodes have a single, identically named client (n_root -> c_root, n_a -> c_a etc),
// except n_g, which has two (c_g1, c_g2). In addition, there is c_many, which has
// parents n_a, n_d, and n_e.

int mock_cb_update(const struct pt_node *self);

// NODES:
struct pt_node n_root = {.name = "n_root", .cb_update = mock_cb_update};
struct pt_node n_a = {.name = "n_a", .cb_update = mock_cb_update};
struct pt_node n_b = {.name = "n_b", .cb_update = mock_cb_update};
struct pt_node n_c = {.name = "n_c", .cb_update = mock_cb_update};
struct pt_node n_d = {.name = "n_d", .cb_update = mock_cb_update};
struct pt_node n_e = {.name = "n_e", .cb_update = mock_cb_update};
struct pt_node n_f = {.name = "n_f", .cb_update = mock_cb_update};
struct pt_node n_g = {.name = "n_g", .cb_update = mock_cb_update};

struct pt_node *nodes[] = {&n_root, &n_a, &n_b, &n_c, &n_d, &n_e, &n_f, &n_g};
struct pt pt = {.nodes = nodes, .node_count = sizeof(nodes) / sizeof(nodes[0]), .root = &n_root};

// CLIENTS:
struct pt_client c_root = {.name = "c_root"};
struct pt_client c_a = {.name = "c_a"};
struct pt_client c_b = {.name = "c_b"};
struct pt_client c_c = {.name = "c_c"};
struct pt_client c_d = {.name = "c_d"};
struct pt_client c_e = {.name = "c_e"};
struct pt_client c_f = {.name = "c_f"};
struct pt_client c_g1 = {.name = "c_g1"};
struct pt_client c_g2 = {.name = "c_g2"};
struct pt_client c_many = {.name = "c_many"};

void assert_tree_state_optimal(void) {
  assert_tree_state_legal(&pt);
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

int mock_cb_update(const struct pt_node *self) {
  (void)self;
  assert_tree_state_legal(&pt);
  return 0;
}

void init_tree(void) {

  pt_node_add_child(&n_root, &n_a);
  pt_node_add_child(&n_root, &n_b);
  pt_node_add_client(&n_root, &c_root);

  pt_node_add_child(&n_a, &n_d);
  pt_node_add_child(&n_a, &n_c);
  pt_node_add_child(&n_b, &n_e);
  pt_node_add_client(&n_a, &c_a);
  pt_node_add_client(&n_a, &c_many);
  pt_node_add_client(&n_b, &c_b);
  pt_node_add_client(&n_c, &c_c);

  pt_node_add_child(&n_d, &n_f);
  pt_node_add_child(&n_c, &n_e);
  pt_node_add_client(&n_d, &c_d);
  pt_node_add_client(&n_d, &c_many);
  pt_node_add_client(&n_c, &c_c);

  pt_node_add_child(&n_e, &n_f);
  pt_node_add_client(&n_e, &c_e);
  pt_node_add_client(&n_e, &c_many);

  pt_node_add_child(&n_f, &n_g);
  pt_node_add_client(&n_f, &c_f);

  pt_node_add_client(&n_g, &c_g1);
  pt_node_add_client(&n_g, &c_g2);
}

// ======== Tests ==================================================================================

void test_complex1_1(void) {

  ASSERT_OK(pt_init(&pt));

  assert_tree_state_optimal();
  ASSERT_OK(pt_enable_client(&pt, &c_f));
  assert_tree_state_optimal();
  ASSERT_OK(pt_disable_client(&pt, &c_f));
  assert_tree_state_optimal();
}

void test_complex1_2(void) {

  ASSERT_OK(pt_init(&pt));

  assert_tree_state_optimal();
  ASSERT_OK(pt_enable_client(&pt, &c_g1));
  assert_tree_state_optimal();
  ASSERT_OK(pt_enable_client(&pt, &c_g2));
  assert_tree_state_optimal();
  ASSERT_OK(pt_disable_client(&pt, &c_g1));
  assert_tree_state_optimal();
  ASSERT_OK(pt_disable_client(&pt, &c_g2));
  assert_tree_state_optimal();
}

void test_many_parent_nodes(void) {

  ASSERT_OK(pt_init(&pt));

  assert_tree_state_optimal();
  ASSERT_OK(pt_enable_client(&pt, &c_many));
  assert_tree_state_optimal();
  ASSERT_OK(pt_disable_client(&pt, &c_many));
  assert_tree_state_optimal();
  ASSERT_OK(pt_enable_client(&pt, &c_many));
  assert_tree_state_optimal();
  ASSERT_OK(pt_enable_client(&pt, &c_e));
  assert_tree_state_optimal();
  ASSERT_OK(pt_enable_client(&pt, &c_d));
  assert_tree_state_optimal();
}

// ======== Main ===================================================================================

void setUp(void) {
  n_root.state = false;
  n_a.state = false;
  n_b.state = false;
  n_c.state = false;
  n_d.state = false;
  n_e.state = false;
  n_f.state = false;
  n_g.state = false;
  c_root.enabled = false;
  c_a.enabled = false;
  c_b.enabled = false;
  c_c.enabled = false;
  c_d.enabled = false;
  c_e.enabled = false;
  c_f.enabled = false;
  c_g1.enabled = false;
  c_g2.enabled = false;
  c_many.enabled = false;
}

void tearDown(void) {}

int main(void) {
  init_tree();
  UNITY_BEGIN();
  RUN_TEST(test_complex1_1);
  RUN_TEST(test_complex1_2);
  RUN_TEST(test_many_parent_nodes);
  return UNITY_END();
}

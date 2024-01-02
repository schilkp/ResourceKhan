#include "stdlib.h"
#include "string.h"
#include "unity.h"
#include "unity_internals.h"
#include "utils.h"

#include "pwr_tree.h"

bool EXPECT_INTERNAL_ASSERT = false;

// ======== Power Tree =============================================================================

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

int mock_cb_update(const struct pt_node *self);

// NODES:
struct pt_node n_root = {.name = "root", .parent_count = 0, .cb_update = mock_cb_update};
struct pt_node n_a = {.name = "n_a", .cb_update = mock_cb_update};
struct pt_node n_b = {.name = "n_b", .cb_update = mock_cb_update};
struct pt_node n_c = {.name = "n_c", .cb_update = mock_cb_update};

struct pt_node *nodes[] = {&n_root, &n_a, &n_b, &n_c};
struct pt pt = {.nodes = nodes, .node_count = sizeof(nodes) / sizeof(nodes[0]), .root = &n_root};

// CLIENTS:
struct pt_client c_a = {.name = "c_a"};
struct pt_client c_b1 = {.name = "c_b1"};
struct pt_client c_b2 = {.name = "c_b2"};
struct pt_client c_c = {.name = "c_c"};

void assert_tree_state_optimal(void) {
  assert_tree_state_legal(&pt);
  ASSERT_NODE(n_root, c_a.enabled || c_b1.enabled || c_b2.enabled || c_c.enabled);
  ASSERT_NODE(n_a, c_a.enabled);
  ASSERT_NODE(n_b, c_b1.enabled || c_b2.enabled || c_c.enabled);
  ASSERT_NODE(n_c, c_c.enabled);
}

int mock_cb_update(const struct pt_node *self) {
  (void)self;
  assert_tree_state_legal(&pt);
  return 0;
}

void init_tree(void) {
  pt_node_add_child(&n_root, &n_a);
  pt_node_add_child(&n_root, &n_b);
  pt_node_add_child(&n_b, &n_c);

  pt_node_add_client(&n_a, &c_a);
  pt_node_add_client(&n_b, &c_b1);
  pt_node_add_client(&n_b, &c_b1);
  pt_node_add_client(&n_c, &c_c);
}

// ======== Tests ==================================================================================

void test_basic_1(void) {

  ASSERT_OK(pt_init(&pt));

  assert_tree_state_optimal();
  ASSERT_OK(pt_enable_client(&pt, &c_a));
  assert_tree_state_optimal();
  ASSERT_OK(pt_enable_client(&pt, &c_b1));
  assert_tree_state_optimal();
  ASSERT_OK(pt_enable_client(&pt, &c_b2));
  assert_tree_state_optimal();
  ASSERT_OK(pt_enable_client(&pt, &c_c));
  assert_tree_state_optimal();
  ASSERT_OK(pt_disable_client(&pt, &c_c));
  assert_tree_state_optimal();
  ASSERT_OK(pt_disable_client(&pt, &c_b2));
  assert_tree_state_optimal();
  ASSERT_OK(pt_disable_client(&pt, &c_b1));
  assert_tree_state_optimal();
  ASSERT_OK(pt_disable_client(&pt, &c_a));
  assert_tree_state_optimal();
}

void test_basic_2(void) {

  ASSERT_OK(pt_init(&pt));

  assert_tree_state_optimal();
  ASSERT_OK(pt_enable_client(&pt, &c_b1));
  assert_tree_state_optimal();
  ASSERT_OK(pt_enable_client(&pt, &c_c));
  assert_tree_state_optimal();
  ASSERT_OK(pt_disable_client(&pt, &c_b1));
  assert_tree_state_optimal();
  ASSERT_OK(pt_disable_client(&pt, &c_c));
  assert_tree_state_optimal();
}

void test_basic_3(void) {

  ASSERT_OK(pt_init(&pt));

  assert_tree_state_optimal();
  ASSERT_OK(pt_enable_client(&pt, &c_c));
  assert_tree_state_optimal();
  ASSERT_OK(pt_disable_client(&pt, &c_c));
  assert_tree_state_optimal();
}

void test_basic_optimize_1(void) {

  ASSERT_OK(pt_init(&pt));

  n_root.state = true;
  n_a.state = true;

  assert_tree_state_legal(&pt);

  pt_optimize(&pt);
  assert_tree_state_optimal();
}

// ======== Main ===================================================================================

void setUp(void) {
  n_root.state = false;
  n_a.state = false;
  n_b.state = false;
  n_c.state = false;
  c_a.enabled = false;
  c_b1.enabled = false;
  c_b2.enabled = false;
  c_c.enabled = false;
}

void tearDown(void) {}

int main(void) {
  init_tree();
  UNITY_BEGIN();
  RUN_TEST(test_basic_1);
  RUN_TEST(test_basic_2);
  RUN_TEST(test_basic_3);
  RUN_TEST(test_basic_optimize_1);
  return UNITY_END();
}

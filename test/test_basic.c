#include "stdlib.h"
#include "string.h"
#include "unity.h"
#include "unity_internals.h"

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

// NODES:
struct pt_node n_root = {.name = "root", .parent_count = 0};
struct pt_node n_a = {.name = "n_a"};
struct pt_node n_b = {.name = "n_b"};
struct pt_node n_c = {.name = "n_c"};

struct pt_node *nodes[] = {&n_root, &n_a, &n_b, &n_c};
struct pt pt = {.nodes = nodes, .count = sizeof(nodes) / sizeof(nodes[0]), .root = &n_root};

// CLIENTS:
struct pt_client c_a = {.name = "c_a"};
struct pt_client c_b1 = {.name = "c_b1"};
struct pt_client c_b2 = {.name = "c_b2"};
struct pt_client c_c = {.name = "c_c"};

#define ASSERT_NODE(_node_, _state_)                                                                                   \
  do {                                                                                                                 \
    if (_state_) {                                                                                                     \
      TEST_ASSERT_MESSAGE((_node_).enabled, "Node " #_node_ " is off but should be on.");                              \
    } else {                                                                                                           \
      TEST_ASSERT_MESSAGE(!(_node_).enabled, "Node " #_node_ " is on but should be off.");                             \
    }                                                                                                                  \
  } while (0)

#define ASSERT_OK(_call_)  TEST_ASSERT_MESSAGE((_call_) == 0, "Call returned unexpected error")
#define ASSERT_ERR(_call_) TEST_ASSERT_MESSAGE((_call_) != 0, "Call returned ok but expected error")

void assert_tree_state_optimal(void) {
  ASSERT_NODE(n_root, c_a.enabled || c_b1.enabled || c_b2.enabled || c_c.enabled);
  ASSERT_NODE(n_a, c_a.enabled);
  ASSERT_NODE(n_b, c_b1.enabled || c_b2.enabled || c_c.enabled);
  ASSERT_NODE(n_c, c_c.enabled);
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

void test_optimise_1(void) {

  ASSERT_OK(pt_init(&pt));

  n_root.enabled = true;
  n_a.enabled = true;
  pt_optimise(&n_root);
  assert_tree_state_optimal();
}

// ======== Main ===================================================================================

void setUp(void) {
  n_root.enabled = false;
  n_a.enabled = false;
  n_b.enabled = false;
  n_c.enabled = false;
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
  RUN_TEST(test_optimise_1);
  return UNITY_END();
}

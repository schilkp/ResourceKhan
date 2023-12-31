#include "stdlib.h"
#include "string.h"
#include "unity.h"
#include "unity_internals.h"

#include "pwr_tree.h"

bool EXPECT_INTERNAL_ASSERT = false;

#define ASSERT_OK(_call_)  TEST_ASSERT_MESSAGE((_call_) == 0, "Call returned unexpected error")
#define ASSERT_ERR(_call_) TEST_ASSERT_MESSAGE((_call_) != 0, "Call returned ok but expected error")

// ======== Tests: Loops ===========================================================================

//    +---------+
//    |        \/
//   n_a       n_b--->c_b
//   /\         |
//    +---------+

// NODES:
struct pt_node n_a = {.name = "n_a"};
struct pt_node n_b = {.name = "n_b"};

struct pt_node *nodes[] = {&n_a, &n_b};
struct pt pt = {.nodes = nodes, .count = sizeof(nodes) / sizeof(nodes[0]), .root = &n_a};

// CLIENTS
struct pt_client c_b = {.name = "c_b"};

void test_catch_loop_init(void) {
  EXPECT_INTERNAL_ASSERT = false;
  ASSERT_ERR(pt_init(&pt));
}

void init_tree(void) {
  pt_node_add_child(&n_a, &n_b);
  pt_node_add_child(&n_b, &n_a);
  pt_node_add_client(&n_b, &c_b);
}

// ======== Tests: Nullptrs ========================================================================

void test_catch_nullptr_enable_direct(void) {
  EXPECT_INTERNAL_ASSERT = true;
  pt_enable_client(&pt, 0);
  TEST_FAIL_MESSAGE("Expected internal assertion!");
}

void test_catch_nullptr_disable_direct(void) {
  EXPECT_INTERNAL_ASSERT = true;
  pt_enable_client(&pt, 0);
  TEST_FAIL_MESSAGE("Expected internal assertion!");
}

// ======== Main ===================================================================================

void setUp(void) {}

void tearDown(void) {}

int main(void) {
  init_tree();
  UNITY_BEGIN();
  RUN_TEST(test_catch_loop_init);
  RUN_TEST(test_catch_nullptr_enable_direct);
  RUN_TEST(test_catch_nullptr_disable_direct);
  return UNITY_END();
}

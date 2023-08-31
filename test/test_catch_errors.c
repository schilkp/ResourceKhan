#include "stdlib.h"
#include "string.h"
#include "unity.h"
#include "unity_internals.h"

#include "pwr_tree.h"

bool EXPECT_INTERNAL_ASSERT = true;

// ======== Tests: Loops ===========================================================================

//    +---------+
//    |        \/
//   n_a       n_b--->c_b
//   /\         |
//    +---------+

// NODES:
struct pwr_tree_node n_a = {.name = "n_a"};
struct pwr_tree_node n_b = {.name = "n_b"};
struct pwr_tree_client c_b = {.name = "c_b"};

void test_catch_loop_enable(void) {
  pwr_tree_enable_client(&c_b);
  TEST_FAIL_MESSAGE("Expected internal assertion!");
}

void test_catch_loop_disable(void) {
  pwr_tree_disable_client(&c_b);
  TEST_FAIL_MESSAGE("Expected internal assertion!");
}

void init_tree(void) {
  pwr_tree_add_child(&n_a, &n_b);
  pwr_tree_add_child(&n_b, &n_a);
  pwr_tree_add_client(&n_b, &c_b);
}

// ======== Tests: Nullptrs ========================================================================

void test_catch_nullptr_enable_direct(void) {
  pwr_tree_enable_client(0);
  TEST_FAIL_MESSAGE("Expected internal assertion!");
}

void test_catch_nullptr_disable_direct(void) {
  pwr_tree_enable_client(0);
  TEST_FAIL_MESSAGE("Expected internal assertion!");
}

// ======== Main ===================================================================================

void setUp(void) {}

void tearDown(void) {}

int main(void) {
  init_tree();
  UNITY_BEGIN();
  RUN_TEST(test_catch_loop_enable);
  RUN_TEST(test_catch_loop_disable);
  RUN_TEST(test_catch_nullptr_enable_direct);
  RUN_TEST(test_catch_nullptr_disable_direct);
  return UNITY_END();
}

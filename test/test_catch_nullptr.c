#include "stdlib.h"
#include "string.h"
#include "unity.h"
#include "unity_internals.h"

#include "resource_khan.h"

bool EXPECT_INTERNAL_ASSERT = true;

// ======== Resource Graph =========================================================================

//   n_a
//    |
//   n_b
//    |
//   c_b

// NODES:
struct rk_node n_a = {.name = "n_a"};
struct rk_node n_b = {.name = "n_b"};

struct rk_node *nodes[] = {&n_a, &n_b};
struct rk_graph pt = {.nodes = nodes, .node_count = sizeof(nodes) / sizeof(nodes[0]), .root = &n_a};

// CLIENTS
struct rk_client c_b = {.name = "c_b"};

void init_graph(void) {
  rk_node_add_child(&n_a, &n_b);
  rk_node_add_client(&n_b, &c_b);
}

// ======== Tests ==================================================================================

void test_catch_nullptr_enable_direct(void) {
  EXPECT_INTERNAL_ASSERT = true;
  rk_enable_client(&pt, 0);
  TEST_FAIL_MESSAGE("Expected internal assertion!");
}

void test_catch_nullptr_disable_direct(void) {
  EXPECT_INTERNAL_ASSERT = true;
  rk_enable_client(&pt, 0);
  TEST_FAIL_MESSAGE("Expected internal assertion!");
}

// ======== Main ===================================================================================

void setUp(void) {}

void tearDown(void) {}

int main(void) {
  init_graph();
  UNITY_BEGIN();
  RUN_TEST(test_catch_nullptr_enable_direct);
  RUN_TEST(test_catch_nullptr_disable_direct);
  return UNITY_END();
}

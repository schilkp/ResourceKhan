#include "stdlib.h"
#include "string.h"
#include "unity.h"
#include "unity_internals.h"
#include "utils.h"

#include "resource_khan.h"

// ======== Resource Graph =====================================================================

//    +---------+
//    |        \/
//   n_a       n_b
//   /\         |
//    +---------+

// NODES:
struct rk_node n_a = {.name = "n_a"};
struct rk_node n_b = {.name = "n_b"};

struct rk_node *nodes[] = {&n_a, &n_b};
struct rk_graph pt = {.nodes = nodes, .node_count = sizeof(nodes) / sizeof(nodes[0]), .root = &n_a};

void init_graph(void) {
  rk_node_add_child(&n_a, &n_b);
  rk_node_add_child(&n_b, &n_a);
}

// ======== Tests ==================================================================================

void test_catch_loop1_init(void) { ASSERT_ERR(rk_init(&pt)); }

// ======== Main ===================================================================================

void setUp(void) {
  for (size_t i = 0; i < pt.node_count; i++) {
    pt.nodes[i]->state = false;
  }
}

void tearDown(void) {}

int main(void) {
  init_graph();
  UNITY_BEGIN();
  RUN_TEST(test_catch_loop1_init);
  return UNITY_END();
}

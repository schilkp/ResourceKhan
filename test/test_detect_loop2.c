#include "stdlib.h"
#include "string.h"
#include "unity.h"
#include "unity_internals.h"
#include "utils.h"

#include "pwr_tree.h"

bool EXPECT_INTERNAL_ASSERT = false;

// ======== Power Tree =============================================================================

//    +---------+
//    |        \/
//   n_a       n_b
//   /\         |
//    +---n_c---+

// NODES:
struct pt_node n_a = {.name = "n_a"};
struct pt_node n_b = {.name = "n_b"};
struct pt_node n_c = {.name = "n_b"};

struct pt_node *nodes[] = {&n_a, &n_b, &n_c};
struct pt pt = {.nodes = nodes, .node_count = sizeof(nodes) / sizeof(nodes[0]), .root = &n_a};

void init_tree(void) {
  pt_node_add_child(&n_a, &n_b);
  pt_node_add_child(&n_b, &n_c);
  pt_node_add_child(&n_c, &n_a);
}

// ======== Tests ==================================================================================

void test_catch_loop2_init(void) { ASSERT_ERR(pt_init(&pt)); }

// ======== Main ===================================================================================

void setUp(void) {
  for (size_t i = 0; i < pt.node_count; i++) {
    pt.nodes[i]->state = false;
  }
}

void tearDown(void) {}

int main(void) {
  init_tree();
  UNITY_BEGIN();
  RUN_TEST(test_catch_loop2_init);
  return UNITY_END();
}

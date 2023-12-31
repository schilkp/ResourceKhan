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
//          n_a       n_b  <-------+
//           |         |           |
//       +---+---+     |           |
//       |       |     |           |
//      n_d     n_c    |           |
//       |       |     |           |
//       |       +-+ +-+           |
//       |         | |             |
//       |         n_e             |
//       |          |              |
//       +----+ +---+              |
//            | |                  |
//            n_f                  |
//             |                   |
//            n_g------------------+

// NODES:
struct pt_node n_root = {.name = "n_root"};
struct pt_node n_a = {.name = "n_a"};
struct pt_node n_b = {.name = "n_b"};
struct pt_node n_c = {.name = "n_c"};
struct pt_node n_d = {.name = "n_d"};
struct pt_node n_e = {.name = "n_e"};
struct pt_node n_f = {.name = "n_f"};
struct pt_node n_g = {.name = "n_g"};

struct pt_node *nodes[] = {&n_root, &n_a, &n_b, &n_c, &n_d, &n_e, &n_f, &n_g};
struct pt pt = {.nodes = nodes, .node_count = sizeof(nodes) / sizeof(nodes[0]), .root = &n_root};

void init_tree(void) {
  pt_node_add_child(&n_root, &n_a);
  pt_node_add_child(&n_root, &n_b);
  pt_node_add_child(&n_a, &n_d);
  pt_node_add_child(&n_a, &n_c);
  pt_node_add_child(&n_b, &n_e);
  pt_node_add_child(&n_d, &n_f);
  pt_node_add_child(&n_c, &n_e);
  pt_node_add_child(&n_e, &n_f);
  pt_node_add_child(&n_f, &n_g);
  pt_node_add_child(&n_g, &n_b);
}

// ======== Tests ==================================================================================

void test_catch_loop3_init(void) {
  EXPECT_INTERNAL_ASSERT = false;
  ASSERT_ERR(pt_init(&pt));
}

// ======== Main ===================================================================================

void setUp(void) {}

void tearDown(void) {}

int main(void) {
  init_tree();
  UNITY_BEGIN();
  RUN_TEST(test_catch_loop3_init);
  return UNITY_END();
}

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
struct rk_node n_root = {.name = "n_root"};
struct rk_node n_a = {.name = "n_a"};
struct rk_node n_b = {.name = "n_b"};
struct rk_node n_c = {.name = "n_c"};
struct rk_node n_d = {.name = "n_d"};
struct rk_node n_e = {.name = "n_e"};
struct rk_node n_f = {.name = "n_f"};
struct rk_node n_g = {.name = "n_g"};

struct rk_node *nodes[] = {&n_root, &n_a, &n_b, &n_c, &n_d, &n_e, &n_f, &n_g};
struct rk_graph pt = {.nodes = nodes, .node_count = sizeof(nodes) / sizeof(nodes[0]), .root = &n_root};

void init_graph(void) {
  rk_node_add_child(&n_root, &n_a);
  rk_node_add_child(&n_root, &n_b);
  rk_node_add_child(&n_a, &n_d);
  rk_node_add_child(&n_a, &n_c);
  rk_node_add_child(&n_b, &n_e);
  rk_node_add_child(&n_d, &n_f);
  rk_node_add_child(&n_c, &n_e);
  rk_node_add_child(&n_e, &n_f);
  rk_node_add_child(&n_f, &n_g);
  rk_node_add_child(&n_g, &n_b);
}

// ======== Tests ==================================================================================

void test_catch_loop3_init(void) { ASSERT_ERR(rk_init(&pt)); }

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
  RUN_TEST(test_catch_loop3_init);
  return UNITY_END();
}

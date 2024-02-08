#include "stdlib.h"
#include "string.h"
#include "unity.h"
#include "unity_internals.h"
#include "utils.h"

#include "resource_khan.h"

// ======== Tests ==================================================================================

void test_catch_loop1_init(void) {

  //    +---------+
  //    |        \/
  //   n_a       n_b
  //   /\         |
  //    +---------+

  struct rk_node n_a = {.name = "n_a", .state = false};

  struct rk_node n_b = {.name = "n_b", .state = false};

  struct rk_node *nodes[] = {&n_a, &n_b};
  struct rk_graph pt = {.nodes = nodes, .node_count = sizeof(nodes) / sizeof(nodes[0]), .root = &n_a};

  ASSERT_OK(rk_node_add_child(&n_a, &n_b));
  ASSERT_OK(rk_node_add_child(&n_b, &n_a));

  // Should faile since graph contains a loop:
  ASSERT_ERR(rk_init(&pt));
}

void test_catch_loop2_init(void) {
  //    +---------+
  //    |        \/
  //   n_a       n_b
  //   /\         |
  //    +---n_c---+

  struct rk_node n_a = {.name = "n_a", .state = false};
  struct rk_node n_b = {.name = "n_b", .state = false};
  struct rk_node n_c = {.name = "n_b", .state = false};

  struct rk_node *nodes[] = {&n_a, &n_b, &n_c};
  struct rk_graph pt = {.nodes = nodes, .node_count = sizeof(nodes) / sizeof(nodes[0]), .root = &n_a};

  ASSERT_OK(rk_node_add_child(&n_a, &n_b));
  ASSERT_OK(rk_node_add_child(&n_b, &n_c));
  ASSERT_OK(rk_node_add_child(&n_c, &n_a));

  // Should faile since graph contains a loop:
  ASSERT_ERR(rk_init(&pt));
}

void test_catch_loop3_init(void) {

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
  struct rk_node n_root = {.name = "n_root", .state = false};
  struct rk_node n_a = {.name = "n_a", .state = false};
  struct rk_node n_b = {.name = "n_b", .state = false};
  struct rk_node n_c = {.name = "n_c", .state = false};
  struct rk_node n_d = {.name = "n_d", .state = false};
  struct rk_node n_e = {.name = "n_e", .state = false};
  struct rk_node n_f = {.name = "n_f", .state = false};
  struct rk_node n_g = {.name = "n_g", .state = false};

  struct rk_node *nodes[] = {&n_root, &n_a, &n_b, &n_c, &n_d, &n_e, &n_f, &n_g};
  struct rk_graph pt = {.nodes = nodes, .node_count = sizeof(nodes) / sizeof(nodes[0]), .root = &n_root};

  ASSERT_OK(rk_node_add_child(&n_root, &n_a));
  ASSERT_OK(rk_node_add_child(&n_root, &n_b));
  ASSERT_OK(rk_node_add_child(&n_a, &n_d));
  ASSERT_OK(rk_node_add_child(&n_a, &n_c));
  ASSERT_OK(rk_node_add_child(&n_b, &n_e));
  ASSERT_OK(rk_node_add_child(&n_d, &n_f));
  ASSERT_OK(rk_node_add_child(&n_c, &n_e));
  ASSERT_OK(rk_node_add_child(&n_e, &n_f));
  ASSERT_OK(rk_node_add_child(&n_f, &n_g));
  ASSERT_OK(rk_node_add_child(&n_g, &n_b));

  // Should faile since graph contains a loop:
  ASSERT_ERR(rk_init(&pt));
}

// ======== Main ===================================================================================

void setUp(void) {}

void tearDown(void) {}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_catch_loop1_init);
  RUN_TEST(test_catch_loop2_init);
  RUN_TEST(test_catch_loop3_init);
  return UNITY_END();
}

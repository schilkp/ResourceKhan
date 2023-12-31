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
//          n_a       n_b
//           |         |
//       +---+---+     |
//       |       |     |
//      n_d     n_c    |
//       |       |     |
//       |       +-+ +-+
//       |         | |
//       |         n_e
//       |          |
//       +----+ +---+
//            | |
//            n_f
//             |
//            n_g
//
// All nodes have a single, identically named client (n_root -> c_root, n_a -> c_a etc),
// except n_g, which has two (c_g1, c_g2). In addition, there is node c_many, which has
// parents n_a, n_d, and n_e.

int mock_cb_update(const struct pt_node *self);

// NODES:
struct pt_node n_root = {.name = "n_root", .cb_update = mock_cb_update};
struct pt_node n_a = {.name = "n_a", .cb_update = mock_cb_update};
struct pt_node n_b = {.name = "n_b", .cb_update = mock_cb_update};
struct pt_node n_c = {.name = "n_c", .cb_update = mock_cb_update};
struct pt_node n_d = {.name = "n_d", .cb_update = mock_cb_update};
struct pt_node n_e = {.name = "n_e", .cb_update = mock_cb_update};
struct pt_node n_f = {.name = "n_f", .cb_update = mock_cb_update};
struct pt_node n_g = {.name = "n_g", .cb_update = mock_cb_update};

struct pt_node *nodes[] = {&n_root, &n_a, &n_b, &n_c, &n_d, &n_e, &n_f, &n_g};
struct pt pt = {.nodes = nodes, .node_count = sizeof(nodes) / sizeof(nodes[0]), .root = &n_root};

// CLIENTS:
struct pt_client c_root = {.name = "c_root"};
struct pt_client c_a = {.name = "c_a"};
struct pt_client c_b = {.name = "c_b"};
struct pt_client c_c = {.name = "c_c"};
struct pt_client c_d = {.name = "c_d"};
struct pt_client c_e = {.name = "c_e"};
struct pt_client c_f = {.name = "c_f"};
struct pt_client c_g1 = {.name = "c_g1"};
struct pt_client c_g2 = {.name = "c_g2"};
struct pt_client c_many = {.name = "c_many"};

void assert_tree_state_optimal(void) {
  ASSERT_NODE(n_root, c_root.enabled || c_a.enabled || c_b.enabled || c_c.enabled || c_d.enabled || c_e.enabled ||
                          c_f.enabled || c_g1.enabled || c_g2.enabled || c_many.enabled);

  ASSERT_NODE(n_a, c_a.enabled || c_c.enabled || c_d.enabled || c_e.enabled || c_f.enabled || c_g1.enabled ||
                       c_g2.enabled || c_many.enabled);

  ASSERT_NODE(n_b, c_b.enabled || c_e.enabled || c_f.enabled || c_g1.enabled || c_g2.enabled || c_many.enabled);

  ASSERT_NODE(n_c, c_c.enabled || c_e.enabled || c_f.enabled || c_g1.enabled || c_g2.enabled || c_many.enabled);

  ASSERT_NODE(n_d, c_d.enabled || c_f.enabled || c_g1.enabled || c_g2.enabled || c_many.enabled);

  ASSERT_NODE(n_e, c_e.enabled || c_f.enabled || c_g1.enabled || c_g2.enabled || c_many.enabled);

  ASSERT_NODE(n_f, c_f.enabled || c_g1.enabled || c_g2.enabled);

  ASSERT_NODE(n_g, c_g1.enabled || c_g2.enabled);
}

bool n_root_fail = false;
bool n_a_fail = false;
bool n_b_fail = false;
bool n_c_fail = false;
bool n_d_fail = false;
bool n_e_fail = false;
bool n_f_fail = false;
bool n_g_fail = false;
int mock_cb_update(const struct pt_node *self) {
  if (strcmp("n_root", self->name) == 0 && n_root_fail) {
    return -1;
  }
  if (strcmp("n_a", self->name) == 0 && n_a_fail) {
    return -1;
  }
  if (strcmp("n_b", self->name) == 0 && n_b_fail) {
    return -1;
  }
  if (strcmp("n_c", self->name) == 0 && n_c_fail) {
    return -1;
  }
  if (strcmp("n_d", self->name) == 0 && n_d_fail) {
    return -1;
  }
  if (strcmp("n_e", self->name) == 0 && n_e_fail) {
    return -1;
  }
  if (strcmp("n_f", self->name) == 0 && n_f_fail) {
    return -1;
  }
  if (strcmp("n_g", self->name) == 0 && n_g_fail) {
    return -1;
  }

  return 0;
}

void init_tree(void) {
  pt_node_add_child(&n_root, &n_a);
  pt_node_add_child(&n_root, &n_b);
  pt_node_add_client(&n_root, &c_root);

  pt_node_add_child(&n_a, &n_d);
  pt_node_add_child(&n_a, &n_c);
  pt_node_add_child(&n_b, &n_e);
  pt_node_add_client(&n_a, &c_a);
  pt_node_add_client(&n_a, &c_many);
  pt_node_add_client(&n_b, &c_b);
  pt_node_add_client(&n_c, &c_c);

  pt_node_add_child(&n_d, &n_f);
  pt_node_add_child(&n_c, &n_e);
  pt_node_add_client(&n_d, &c_d);
  pt_node_add_client(&n_d, &c_many);
  pt_node_add_client(&n_c, &c_c);

  pt_node_add_child(&n_e, &n_f);
  pt_node_add_client(&n_e, &c_e);
  pt_node_add_client(&n_e, &c_many);

  pt_node_add_child(&n_f, &n_g);
  pt_node_add_client(&n_f, &c_f);

  pt_node_add_client(&n_g, &c_g1);
  pt_node_add_client(&n_g, &c_g2);
}

// ======== Tests ==================================================================================

void test_single_failure_then_retry(void) {

  ASSERT_OK(pt_init(&pt));

  assert_tree_state_optimal();

  n_a_fail = true;
  ASSERT_ERR(pt_enable_client(&pt, &c_f));
  assert_tree_state_legal(&pt);

  n_a_fail = false;
  ASSERT_OK(pt_enable_client(&pt, &c_f));
  assert_tree_state_optimal();
}

void test_single_failure_then_optimise(void) {

  ASSERT_OK(pt_init(&pt));

  assert_tree_state_optimal();

  n_a_fail = true;
  ASSERT_ERR(pt_enable_client(&pt, &c_f));
  assert_tree_state_legal(&pt);

  n_a_fail = false;
  ASSERT_OK(pt_optimise(&pt));
  assert_tree_state_optimal();

  ASSERT_NODE(n_root, 0);
}

void test_multiple_failures(void) {

  ASSERT_OK(pt_init(&pt));

  assert_tree_state_optimal();

  n_a_fail = true;
  n_d_fail = true;
  n_c_fail = true;

  ASSERT_ERR(pt_enable_client(&pt, &c_f));
  assert_tree_state_legal(&pt);

  n_c_fail = false;

  ASSERT_ERR(pt_enable_client(&pt, &c_f));
  assert_tree_state_legal(&pt);

  n_d_fail = false;

  ASSERT_ERR(pt_enable_client(&pt, &c_f));
  assert_tree_state_legal(&pt);

  n_a_fail = false;

  ASSERT_OK(pt_enable_client(&pt, &c_f));
  assert_tree_state_optimal();
}

void test_pointless_optimise(void) {

  ASSERT_OK(pt_init(&pt));

  assert_tree_state_optimal();

  ASSERT_OK(pt_enable_client(&pt, &c_f));
  assert_tree_state_optimal();

  ASSERT_OK(pt_enable_client(&pt, &c_b));
  assert_tree_state_optimal();

  ASSERT_OK(pt_optimise(&pt));
  assert_tree_state_optimal();

  ASSERT_OK(pt_disable_client(&pt, &c_b));
  assert_tree_state_optimal();

  ASSERT_OK(pt_disable_client(&pt, &c_f));
  assert_tree_state_optimal();

  ASSERT_OK(pt_optimise(&pt));
  assert_tree_state_optimal();
}

// ======== Main ===================================================================================

void setUp(void) {
  n_root.enabled = false;
  n_a.enabled = false;
  n_b.enabled = false;
  n_c.enabled = false;
  n_d.enabled = false;
  n_e.enabled = false;
  n_f.enabled = false;
  n_g.enabled = false;
  c_root.enabled = false;
  c_a.enabled = false;
  c_b.enabled = false;
  c_c.enabled = false;
  c_d.enabled = false;
  c_e.enabled = false;
  c_f.enabled = false;
  c_g1.enabled = false;
  c_g2.enabled = false;
  c_many.enabled = false;
  n_root_fail = false;
  n_a_fail = false;
  n_b_fail = false;
  n_c_fail = false;
  n_d_fail = false;
  n_e_fail = false;
  n_f_fail = false;
  n_g_fail = false;
}

void tearDown(void) {}

int main(void) {
  init_tree();
  UNITY_BEGIN();
  RUN_TEST(test_single_failure_then_retry);
  RUN_TEST(test_single_failure_then_optimise);
  RUN_TEST(test_multiple_failures);
  RUN_TEST(test_pointless_optimise);
  return UNITY_END();
}

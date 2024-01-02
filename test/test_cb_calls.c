#include "stdlib.h"
#include "string.h"
#include "unity.h"
#include "unity_internals.h"
#include "utils.h"

#include "resource_khan.h"

bool EXPECT_INTERNAL_ASSERT = false;

// ======== Resource Graph =========================================================================

//
//              n_root
//                |
//           +----+----+
//           |         |
//          n_a       n_b
//           |         |
//          n_c        |
//           |         |
//           +---+ +---+
//               | |
//               n_d
//
// All nodes have a single, identically named client (n_root -> c_root, n_a -> c_a etc).

int mock_cb_update(const struct rk_node *self);

// NODES:
#define N_ROOT 0
struct rk_node n_root = {.name = "n_root", .cb_update = mock_cb_update};
#define N_A 1
struct rk_node n_a = {.name = "n_a", .cb_update = mock_cb_update};
#define N_B 2
struct rk_node n_b = {.name = "n_b", .cb_update = mock_cb_update};
#define N_C 3
struct rk_node n_c = {.name = "n_c", .cb_update = mock_cb_update};
#define N_D 4
struct rk_node n_d = {.name = "n_d", .cb_update = mock_cb_update};

struct rk_node *nodes[] = {[N_ROOT] = &n_root, [N_A] = &n_a, [N_B] = &n_b, [N_C] = &n_c, [N_D] = &n_d};
struct rk_graph pt = {.nodes = nodes, .node_count = sizeof(nodes) / sizeof(nodes[0]), .root = &n_root};

// CLIENTS:
struct rk_client c_root = {.name = "c_root"};
struct rk_client c_a = {.name = "c_a"};
struct rk_client c_b = {.name = "c_b"};
struct rk_client c_c = {.name = "c_c"};
struct rk_client c_d = {.name = "c_d"};

struct rk_client *clients[] = {&c_root, &c_a, &c_b, &c_c, &c_d};

bool node_cb_called[sizeof(nodes) / sizeof(nodes[0])] = {0};

int mock_cb_update(const struct rk_node *self) {
  (void)self;
  assert_graph_state_legal(&pt);
  // Track that the callback for this node was called:
  for (size_t i = 0; i < pt.node_count; i++) {
    if (pt.nodes[i] == self) {
      node_cb_called[i] = true;
      return 0;
    }
  }
  TEST_FAIL(); // Node not in nodes array?!
}

void init_graph(void) {

  rk_node_add_child(&n_root, &n_a);
  rk_node_add_child(&n_root, &n_b);
  rk_node_add_client(&n_root, &c_root);

  rk_node_add_child(&n_a, &n_c);
  rk_node_add_client(&n_a, &c_a);

  rk_node_add_child(&n_b, &n_d);
  rk_node_add_client(&n_b, &c_b);

  rk_node_add_child(&n_c, &n_d);
  rk_node_add_client(&n_c, &c_c);

  rk_node_add_client(&n_d, &c_d);
}

void assert_correct_callbacks_called(bool *is, bool *expected, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (is[i] != expected[i]) {
      if (is[i]) {
        TEST_FAIL_MESSAGE("Callback was called, but it should not have been!");
      } else {
        TEST_FAIL_MESSAGE("Callback was not called, but it should have been!");
      }
    }
  }
}

// ======== Tests ==================================================================================

// Test that enabeling a client only causes the callback of nodes that the client
// depends on to get called:

void test_cb_calls_enable1(void) {
  ASSERT_OK(rk_init(&pt));

  memset(node_cb_called, 0, sizeof(node_cb_called));
  ASSERT_OK(rk_enable_client(&pt, &c_c));
  bool expected1[] = {[N_ROOT] = true, [N_A] = true, [N_B] = false, [N_C] = true, [N_D] = false};
  assert_correct_callbacks_called(node_cb_called, expected1, pt.node_count);
}

void test_cb_calls_enable2(void) {
  ASSERT_OK(rk_init(&pt));

  ASSERT_OK(rk_enable_client(&pt, &c_c));

  memset(node_cb_called, 0, sizeof(node_cb_called));
  ASSERT_OK(rk_enable_client(&pt, &c_b));
  bool expected[] = {[N_ROOT] = true, [N_A] = false, [N_B] = true, [N_C] = false, [N_D] = false};
  assert_correct_callbacks_called(node_cb_called, expected, pt.node_count);
}

void test_cb_calls_enable3(void) {
  ASSERT_OK(rk_init(&pt));

  ASSERT_OK(rk_enable_client(&pt, &c_c));

  ASSERT_OK(rk_enable_client(&pt, &c_b));

  memset(node_cb_called, 0, sizeof(node_cb_called));
  ASSERT_OK(rk_enable_client(&pt, &c_d));
  bool expected[] = {[N_ROOT] = true, [N_A] = true, [N_B] = true, [N_C] = true, [N_D] = true};
  assert_correct_callbacks_called(node_cb_called, expected, pt.node_count);
}

void test_cb_calls_enable4(void) {
  ASSERT_OK(rk_init(&pt));

  ASSERT_OK(rk_enable_client(&pt, &c_c));

  ASSERT_OK(rk_enable_client(&pt, &c_b));

  ASSERT_OK(rk_enable_client(&pt, &c_d));

  memset(node_cb_called, 0, sizeof(node_cb_called));
  ASSERT_OK(rk_enable_client(&pt, &c_d));
  bool expected[] = {[N_ROOT] = true, [N_A] = true, [N_B] = true, [N_C] = true, [N_D] = true};
  assert_correct_callbacks_called(node_cb_called, expected, pt.node_count);
}

// Test that disabling a client only causes the callback of nodes that the client
// depends on to get called:

void test_cb_calls_disable1(void) {
  ASSERT_OK(rk_init(&pt));

  memset(node_cb_called, 0, sizeof(node_cb_called));
  ASSERT_OK(rk_disable_client(&pt, &c_root));
  bool expected[] = {[N_ROOT] = true, [N_A] = false, [N_B] = false, [N_C] = false, [N_D] = false};
  assert_correct_callbacks_called(node_cb_called, expected, pt.node_count);
}

void test_cb_calls_disable2(void) {
  ASSERT_OK(rk_init(&pt));

  ASSERT_OK(rk_enable_client(&pt, &c_root));

  memset(node_cb_called, 0, sizeof(node_cb_called));
  ASSERT_OK(rk_disable_client(&pt, &c_root));
  bool expected[] = {[N_ROOT] = true, [N_A] = false, [N_B] = false, [N_C] = false, [N_D] = false};
  assert_correct_callbacks_called(node_cb_called, expected, pt.node_count);
}

void test_cb_calls_disable3(void) {
  ASSERT_OK(rk_init(&pt));

  memset(node_cb_called, 0, sizeof(node_cb_called));
  ASSERT_OK(rk_disable_client(&pt, &c_c));
  bool expected[] = {[N_ROOT] = true, [N_A] = true, [N_B] = false, [N_C] = true, [N_D] = false};
  assert_correct_callbacks_called(node_cb_called, expected, pt.node_count);
}

void test_cb_calls_disable4(void) {
  ASSERT_OK(rk_init(&pt));

  memset(node_cb_called, 0, sizeof(node_cb_called));
  ASSERT_OK(rk_disable_client(&pt, &c_d));
  bool expected[] = {[N_ROOT] = true, [N_A] = true, [N_B] = true, [N_C] = true, [N_D] = true};
  assert_correct_callbacks_called(node_cb_called, expected, pt.node_count);
}

// ======== Main ===================================================================================

void setUp(void) {
  for (size_t i = 0; i < pt.node_count; i++) {
    pt.nodes[i]->state = false;
  }
  for (size_t i = 0; i < (sizeof(clients) / sizeof(clients[0])); i++) {
    clients[i]->enabled = false;
  }
}

void tearDown(void) {}

int main(void) {
  init_graph();
  UNITY_BEGIN();
  RUN_TEST(test_cb_calls_enable1);
  RUN_TEST(test_cb_calls_enable2);
  RUN_TEST(test_cb_calls_enable3);
  RUN_TEST(test_cb_calls_enable4);
  RUN_TEST(test_cb_calls_disable1);
  RUN_TEST(test_cb_calls_disable2);
  RUN_TEST(test_cb_calls_disable3);
  RUN_TEST(test_cb_calls_disable4);
  return UNITY_END();
}

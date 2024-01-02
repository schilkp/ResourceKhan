#include "utils.h"
#include "unity.h"
#include "unity_internals.h"

void assert_tree_state_legal(struct pt *pt) {
  // Check that no node is enabled without its parent being enabled:
  for (size_t i = 0; i < pt->node_count; i++) {
    struct pt_node *node = pt->nodes[i];
    if (node->enabled) {
      for (size_t parent_idx = 0; parent_idx < node->parent_count; parent_idx++) {
        TEST_ASSERT_MESSAGE(node->parents[parent_idx], "Node has disable parent but is on!");
      }
    }
  }
}

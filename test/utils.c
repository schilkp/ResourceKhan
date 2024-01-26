#include "utils.h"
#include "unity.h"
#include "unity_internals.h"

#include "resource_khan_ext.h"

void assert_graph_state_legal(struct rk_graph *pt) {
  // Check that no node is enabled without its parent being enabled:
  for (size_t i = 0; i < pt->node_count; i++) {
    struct rk_node *node = pt->nodes[i];
    if (node->state) {
      for (size_t parent_idx = 0; parent_idx < node->parent_count; parent_idx++) {
        TEST_ASSERT_MESSAGE(node->parents[parent_idx], "Node has disable parent but is on!");
      }
    }
  }
}

FILE *fptr;

static void out_to_fptr(const char *msg) { fprintf(fptr, "%s", msg); }

void debug_render_graph(struct rk_graph *pt, const char *filename) {

  fptr = fopen(filename, "w");
  if (fptr == 0) {
    return;
  }

  struct rk_dot_params params = {
      .include_state = true,
      .include_trv_list = true,
      .include_topo_list = true,
  };

  rk_exportdot_cb(pt, out_to_fptr, &params);

  fclose(fptr);
}

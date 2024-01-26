/**
 * @file resource_khan_ext.h
 * @brief Resource Khan extenstion/utility methods.
 * @author Philipp Schilk, 2024
 * https://github.com/schilkp/ResourceKhan
 */
#include "resource_khan_ext.h"
#include <string.h>

// ==== Private Prototypes =====================================================

static void reset_client_in_dot_graph(struct rk_graph *pt);

static inline bool handle_contains_nullptr(struct rk_graph *graph) {
  if (graph == 0) return true;
  if (graph->nodes == 0) return true;
  if (graph->root == 0) return true;
  return false;
}

// ==== Public Functions =======================================================

int rk_exportdot_cb(struct rk_graph *graph, void (*out)(const char *msg), const struct rk_dot_params *params) {
  if (handle_contains_nullptr(graph)) return RK_ERR;
  if (out == 0) return RK_ERR;

  reset_client_in_dot_graph(graph);

  // Graph start:
  out("digraph {\r\n");

  // Render all graph of all nodes & clients:
  for (size_t node_idx = 0; node_idx < graph->node_count; node_idx++) {

    // Node oval:
    struct rk_node *node = graph->nodes[node_idx];
    out("  \"");
    out(node->name);
    out("\" [shape=oval");
    if (params->include_state) {
      out(", style=\"filled\", fillcolor=\"");
      out(node->state ? "limegreen" : "white");
      out("\"");
    }
    out("];\r\n");

    // Node -> Node edges:
    for (size_t child_idx = 0; child_idx < node->child_count; child_idx++) {
      out("  \"");
      out(node->name);
      out("\" -> \"");
      out(node->children[child_idx]->name);
      out("\";\r\n");
    }

    for (size_t client_idx = 0; client_idx < node->client_count; client_idx++) {
      struct rk_client *client = node->clients[client_idx];

      // Client rectangle (if not already included in graph):
      if (!client->in_dot_graph) {
        out("  \"");
        out(client->name);
        out("\" [shape=rectangle");
        if (params->include_state) {
          out(", style=\"filled\", fillcolor=\"");
          out(client->enabled ? "limegreen" : "white");
          out("\"");
        }
        out("];\r\n");

        client->in_dot_graph = true; // mark as included.
      }

      // Node->Client edges:
      out("  \"");
      out(node->name);
      out("\" -> \"");
      out(client->name);
      out("\";\r\n");
    }

    // Topo list (if enabled):
    if (params->include_topo_list) {
      if (node->ctx.ll_topo_next != 0) {
        out("  \"");
        out(node->name);
        out("\" -> \"");
        out(node->ctx.ll_topo_next->name);
        out("\" [constraint=false, style=dashed, arrowsize=0.5];\r\n");
      }
    }

    // Trv list (if enabled)
    if (params->include_trv_list) {
      if (node->ctx.ll_trv != 0) {
        out("  \"");
        out(node->name);
        out("\" -> \"");
        out(node->ctx.ll_trv->name);
        out("\" [constraint=false, style=dotted, arrowsize=0.5];\r\n");
      }
    }
  }

  // Graph end:
  out("}\r\n");

  return 0;
}

// ==== Private Functions ======================================================

static void reset_client_in_dot_graph(struct rk_graph *pt) {
  for (size_t i = 0; i < pt->node_count; i++) {
    struct rk_node *node = pt->nodes[i];
    for (size_t j = 0; j < node->client_count; j++) {
      node->clients[j]->in_dot_graph = false;
    }
  }
}

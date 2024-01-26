/**
 * @file resource_khan_ext.h
 * @brief Resource Khan extenstion/utility methods.
 * @author Philipp Schilk, 2024
 * https://github.com/schilkp/ResourceKhan
 */
#ifndef RESOURCE_KHAN_EXT_H_
#define RESOURCE_KHAN_EXT_H_

#include "resource_khan.h"

/** @brief Dot export parameters */
struct rk_dot_params {
  bool include_state;     //!< Color active nodes/elements green.
  bool include_topo_list; //!< Include the internal "topo" list (for debugging.)
  bool include_trv_list;  //!< Include the internal "trv" list (for debugging.)
};

/**
 * @brief Render a graph into dot, to be visualized using graphviz.
 * The output is generated by repeatedly calling the "out" output stream callback.
 *
 * @param graph resource graph
 * @param out output stream
 * @param params graph visualization parameters
 * @return 0 if successful
 * @return RK_ERR if the graph could not be initialized
 */
int rk_exportdot_cb(struct rk_graph *graph, void (*out)(const char *msg), const struct rk_dot_params *params);

#endif /* RESOURCE_KHAN_EXT_H_ */
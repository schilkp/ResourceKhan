#ifndef UTILS_H_
#define UTILS_H_

#include "resource_khan.h"

#define ASSERT_NODE(_node_, _state_)                                                                                   \
  do {                                                                                                                 \
    if (_state_) {                                                                                                     \
      TEST_ASSERT_MESSAGE((_node_).state, "Node " #_node_ " is off but should be on.");                                \
    } else {                                                                                                           \
      TEST_ASSERT_MESSAGE(!(_node_).state, "Node " #_node_ " is on but should be off.");                               \
    }                                                                                                                  \
  } while (0)

#define ASSERT_OK(_call_)  TEST_ASSERT_MESSAGE((_call_) == 0, "Call returned unexpected error")
#define ASSERT_ERR(_call_) TEST_ASSERT_MESSAGE((_call_) != 0, "Call returned ok but expected error")

void assert_graph_state_legal(struct rk_graph *pt);

#endif /* UTILS_H_ */

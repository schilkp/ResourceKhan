#ifndef PWR_TREE_CONF_CUSTOM_H_
#define PWR_TREE_CONF_CUSTOM_H_

#include "unity.h"
#include "unity_internals.h"
#include <stdbool.h>

extern bool EXPECT_INTERNAL_ASSERT;

/** @brief Maximum length of node/client name  */
#define PWR_TREE_MAX_NAME_LEN 15

/** @brief Maximum number of parent nodes per node or client */
#define PWR_TREE_MAX_PARENTS 4

/** @brief Maximum number of children nodes and clients per node */
#define PWR_TREE_MAX_CHILDREN 4

/** @brief Information log function */
#define PWR_TREE_INF(_fmt_, ...)                                                                                       \
  do {                                                                                                                 \
  } while (0)

/** @brief Error log function */
#define PWR_TREE_ERR(_fmt_, ...)                                                                                       \
  do {                                                                                                                 \
  } while (0)

/** @brief Assertion */
#define PWR_TREE_ASSERT(_condition_)                                                                                   \
  do {                                                                                                                 \
    if (EXPECT_INTERNAL_ASSERT) {                                                                                      \
      if (!(_condition_)) {                                                                                            \
        TEST_PASS();                                                                                                   \
      }                                                                                                                \
    } else {                                                                                                           \
      TEST_ASSERT_MESSAGE(_condition_, "Internal assert failed.");                                                     \
    }                                                                                                                  \
  } while (0);

#endif /* PWR_TREE_CONF_CUSTOM_H_ */

#ifndef RESOURCE_KHAN_CONF_CUSTOM_H_
#define RESOURCE_KHAN_CONF_CUSTOM_H_

#include "unity.h"
#include "unity_internals.h"
#include <stdbool.h>

extern bool EXPECT_INTERNAL_ASSERT;

/** @brief Maximum length of node/client name  */
#define RK_MAX_NAME_LEN 15

/** @brief Maximum number of parent nodes per node or client */
#define RK_MAX_PARENTS 4

/** @brief Maximum number of children nodes and clients per node */
#define RK_MAX_CHILDREN 4

/** @brief Information log function */
#define RK_INF(_fmt_, ...)                                                                                             \
  do {                                                                                                                 \
  } while (0)

/** @brief Error log function */
#define RK_ERR(_fmt_, ...)                                                                                             \
  do {                                                                                                                 \
  } while (0)

/** @brief Assertion */
#define RK_ASSERT(_condition_)                                                                                         \
  do {                                                                                                                 \
    if (EXPECT_INTERNAL_ASSERT) {                                                                                      \
      if (!(_condition_)) {                                                                                            \
        TEST_PASS();                                                                                                   \
      }                                                                                                                \
    } else {                                                                                                           \
      TEST_ASSERT_MESSAGE(_condition_, "Internal assert failed.");                                                     \
    }                                                                                                                  \
  } while (0);

#endif /* RESOURCE_KHAN_CONF_CUSTOM_H_ */

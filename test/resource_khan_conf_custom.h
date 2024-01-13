#ifndef RESOURCE_KHAN_CONF_CUSTOM_H_
#define RESOURCE_KHAN_CONF_CUSTOM_H_

#include "unity.h"
#include "unity_internals.h"
#include <stdbool.h>

/** @brief Maximum length of node/client name  */
#define RK_MAX_NAME_LEN 15

/** @brief Maximum number of parent nodes per node or client */
#define RK_MAX_PARENTS 4

/** @brief Maximum number of children nodes and clients per node */
#define RK_MAX_CHILDREN 4

/**
 * @brief Return value indicating a function did not complete succesfully
 * @note Must be an 'int' value that is not '0'
 */
#define RK_ERR 1

/** @brief Information log function */
#define RK_LOG_INF(_fmt_, ...)                                                                                         \
  do {                                                                                                                 \
  } while (0)

/** @brief Error log function */
#define RK_LOG_ERR(_fmt_, ...)                                                                                         \
  do {                                                                                                                 \
  } while (0)

#endif /* RESOURCE_KHAN_CONF_CUSTOM_H_ */

/**
 * @file resource_khan_conf.h
 * @brief Graph resource manager.
 * @author Philipp Schilk 2023
 * https://github.com/schilkp/ResourceKhan
 */
#ifndef RESOURCE_KHAN_CONF_H_
#define RESOURCE_KHAN_CONF_H_

#include <stdio.h>

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
    printf("INF: "_fmt_                                                                                                \
           "\n",                                                                                                       \
           __VA_ARGS__);                                                                                               \
  } while (0)

/** @brief Error log function */
#define RK_LOG_ERR(_fmt_, ...)                                                                                         \
  do {                                                                                                                 \
    printf("ERR: "_fmt_                                                                                                \
           "\n",                                                                                                       \
           __VA_ARGS__);                                                                                               \
  } while (0)

#endif /* RESOURCE_KHAN_CONF_H_ */

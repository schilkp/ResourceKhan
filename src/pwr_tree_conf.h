/**
 * @file pwr_tree_conf.h
 * @brief Dynamic power resource manager.
 * @author Philipp Schilk 2023
 * https://github.com/schilkp/pwr_tree
 */
#ifndef PWR_TREE_CONF_H_
#define PWR_TREE_CONF_H_

#include <stdio.h>

/** @brief Maximum length of node/client name  */
#define PWR_TREE_MAX_NAME_LEN 15

/** @brief Maximum number of parent nodes per node or client */
#define PWR_TREE_MAX_PARENTS 4

/** @brief Maximum number of children nodes and clients per node */
#define PWR_TREE_MAX_CHILDREN 4

/** @brief Information log function */
#define PWR_TREE_INF(_fmt_, ...)                                                                                       \
  do {                                                                                                                 \
    printf("INF: "_fmt_                                                                                                \
           "\n",                                                                                                       \
           __VA_ARGS__);                                                                                               \
  } while (0)

/** @brief Error log function */
#define PWR_TREE_ERR(_fmt_, ...)                                                                                       \
  do {                                                                                                                 \
    printf("ERR: "_fmt_                                                                                                \
           "\n",                                                                                                       \
           __VA_ARGS__);                                                                                               \
  } while (0)

/** @brief Assertion */
#define PWR_TREE_ASSERT(_condition_)                                                                                   \
  do {                                                                                                                 \
    if (!(_condition_)) {                                                                                              \
      while (1) {                                                                                                      \
        PWR_TREE_ERR("Assert failed @ %s:%i!", __FILE__, __LINE__);                                                    \
      }                                                                                                                \
    }                                                                                                                  \
  } while (0);

#endif /* PWR_TREE_CONF_H_ */

#ifndef DYNARR_H
#define DYNARR_H

#include "logger.h"
#include "utils/globals.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define ARR_INIT_CAP 4
#define ARR_GROW_FACTOR 2

#define DYNARR_INIT(arr, T) da_init(arr, sizeof(T))
#define DYNARR_PUSH(arr, T) ((T *)da_push(arr))
#define DYNARR_GET(arr, i, T) ((T *)da_get(arr, i))
#define DYNARR_LAST(arr, T)                                                    \
  ((arr)->count > 0 ? (T *)da_get(arr, (arr)->count - 1) : NULL)
#define DYNARR_FOREACH(arr, T, var)                                            \
  if ((arr)->count > 0)                                                        \
    for (T *var = (T *)(arr)->data; var < (T *)(arr)->data + (arr)->count;     \
         var++)

typedef struct {
  void *data;
  usize count;
  usize cap;
  usize elemSize;
} DynArr;

static inline void da_init(DynArr *arr, usize elemSize) {
  arr->data = NULL;
  arr->count = 0;
  arr->cap = 0;
  arr->elemSize = elemSize;
}

static inline void da_destroy(DynArr *arr) {
  free(arr->data);
  arr->data = NULL;
  arr->count = 0;
  arr->cap = 0;
}

static inline bool da_grow(DynArr *arr) {
  usize newCap = arr->cap == 0 ? ARR_INIT_CAP : arr->cap * ARR_GROW_FACTOR;
  void *tmp = realloc(arr->data, newCap * arr->elemSize);
  if (!tmp) {
    vs_log_error("failed to grow dynamic array");
    return false;
  }
  arr->data = tmp;
  arr->cap = newCap;
  return true;
}

static inline void *da_push(DynArr *arr) {
  if (arr->count >= arr->cap && !da_grow(arr))
    return NULL;
  void *elem = (char *)arr->data + arr->count * arr->elemSize;
  arr->count++;
  return elem;
}

static inline void da_pop(DynArr *arr) {
  if (arr->count == 0) {
    vs_log_warn("attempted to pop from empty dynamic array");
    return;
  }

  arr->count--;
}

static inline void *da_get(DynArr *arr, usize index) {
  if (index >= arr->count)
    return NULL;
  return (char *)arr->data + index * arr->elemSize;
}

static inline void *da_insert(DynArr *arr, usize index) {
  if (index > arr->count) {
    vs_log_warn(
        "attempted to insert at index %zu in dynamic array of length %zu",
        index, arr->count);
    return NULL;
  }
  if (arr->count >= arr->cap && !da_grow(arr))
    return NULL;
  void *dest = (char *)arr->data + index * arr->elemSize;
  memmove((char *)dest + arr->elemSize, dest,
          (arr->count - index) * arr->elemSize);
  arr->count++;
  return dest;
}

static inline void da_remove(DynArr *arr, usize index) {
  if (index >= arr->count) {
    vs_log_warn(
        "attempted to remove at index %zu in dynamic array of length %zu",
        index, arr->count);
    return;
  }

  void *dest = (char *)arr->data + index * arr->elemSize;
  memmove(dest, (char *)dest + arr->elemSize,
          (arr->count - index - 1) * arr->elemSize);
  arr->count--;
}

static inline usize da_len(DynArr *arr) { return arr->count; }

static inline void da_clear(DynArr *arr) { arr->count = 0; }

#endif // DYNARR_H

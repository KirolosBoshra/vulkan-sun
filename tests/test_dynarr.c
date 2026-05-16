#include "utils/dynarr.h"
#include <assert.h>
#include <stdio.h>

static void test_init(void) {
  DynArr arr;
  DYNARR_INIT(&arr, int);
  assert(arr.data == NULL);
  assert(arr.count == 0);
  assert(arr.cap == 0);
  assert(arr.elemSize == sizeof(int));
  da_destroy(&arr);
}

static void test_push_one(void) {
  DynArr arr;
  DYNARR_INIT(&arr, int);
  int *p = DYNARR_PUSH(&arr, int);
  assert(p != NULL);
  *p = 42;
  assert(arr.count == 1);
  int *g = DYNARR_GET(&arr, 0, int);
  assert(g != NULL && *g == 42);
  da_destroy(&arr);
}

static void test_push_many(void) {
  DynArr arr;
  DYNARR_INIT(&arr, int);
  i32 n = 100;
  for (i32 i = 0; i < n; i++) {
    int *p = DYNARR_PUSH(&arr, int);
    assert(p != NULL);
    *p = i * 2;
  }
  assert(da_len(&arr) == (usize)n);
  assert(arr.cap >= (usize)n);
  for (i32 i = 0; i < n; i++) {
    int *v = DYNARR_GET(&arr, (usize)i, int);
    assert(v != NULL && *v == i * 2);
  }
  da_destroy(&arr);
}

static void test_pop(void) {
  DynArr arr;
  DYNARR_INIT(&arr, int);
  for (i32 i = 0; i < 5; i++) {
    int *p = DYNARR_PUSH(&arr, int);
    *p = i;
  }
  assert(da_len(&arr) == 5);
  da_pop(&arr);
  assert(da_len(&arr) == 4);
  int *v = DYNARR_GET(&arr, 4, int);
  assert(v == NULL);
  da_destroy(&arr);
}

static void test_get_out_of_range(void) {
  DynArr arr;
  DYNARR_INIT(&arr, i32);
  int *p = DYNARR_PUSH(&arr, int);
  *p = 7;
  assert(DYNARR_GET(&arr, 0, int) != NULL);
  assert(DYNARR_GET(&arr, 1, int) == NULL);
  assert(DYNARR_GET(&arr, 999, int) == NULL);
  da_destroy(&arr);
}

static void test_last(void) {
  DynArr arr;
  DYNARR_INIT(&arr, i32);
  for (i32 i = 0; i < 10; i++) {
    int *p = DYNARR_PUSH(&arr, int);
    *p = i;
  }
  int *last = DYNARR_LAST(&arr, int);
  assert(last != NULL && *last == 9);
  da_destroy(&arr);
}

static void test_clear(void) {
  DynArr arr;
  DYNARR_INIT(&arr, char);
  for (i32 i = 0; i < 20; i++) {
    char *p = DYNARR_PUSH(&arr, char);
    *p = 'a' + (char)i;
  }
  assert(da_len(&arr) == 20);
  da_clear(&arr);
  assert(da_len(&arr) == 0);
  assert(arr.cap > 0);
  da_destroy(&arr);
}

static void test_insert(void) {
  DynArr arr;
  DYNARR_INIT(&arr, int);
  for (i32 i = 0; i < 3; i++) {
    int *p = DYNARR_PUSH(&arr, int);
    *p = i * 10;
  }
  da_insert(&arr, 1);
  int *v = DYNARR_GET(&arr, 1, int);
  *v = 99;
  assert(da_len(&arr) == 4);
  int *g = DYNARR_GET(&arr, 0, int);
  assert(*g == 0);
  g = DYNARR_GET(&arr, 1, int);
  assert(*g == 99);
  g = DYNARR_GET(&arr, 2, int);
  assert(*g == 10);
  g = DYNARR_GET(&arr, 3, int);
  assert(*g == 20);
  da_destroy(&arr);
}

static void test_remove(void) {
  DynArr arr;
  DYNARR_INIT(&arr, int);
  for (i32 i = 0; i < 5; i++) {
    int *p = DYNARR_PUSH(&arr, int);
    *p = i;
  }
  da_remove(&arr, 2);
  assert(da_len(&arr) == 4);
  int *v = DYNARR_GET(&arr, 2, int);
  assert(*v == 3);
  v = DYNARR_GET(&arr, 3, int);
  assert(*v == 4);
  da_destroy(&arr);
}

static void test_foreach(void) {
  DynArr arr;
  DYNARR_INIT(&arr, int);
  for (i32 i = 0; i < 10; i++) {
    int *p = DYNARR_PUSH(&arr, int);
    *p = i + 100;
  }
  i32 sum = 0;
  DYNARR_FOREACH(&arr, int, v) { sum += *v; }
  assert(sum == 1045);
  da_destroy(&arr);
}

static void test_struct(void) {
  typedef struct {
    f32 x, y, z;
  } Point;
  DynArr arr;
  DYNARR_INIT(&arr, Point);
  for (i32 i = 0; i < 5; i++) {
    Point *p = DYNARR_PUSH(&arr, Point);
    p->x = (f32)i;
    p->y = (f32)i * 2;
    p->z = (f32)i * 3;
  }
  DYNARR_FOREACH(&arr, Point, p) {
    i32 i = (i32)p->x;
    assert(p->y == (f32)i * 2);
    assert(p->z == (f32)i * 3);
  }
  da_destroy(&arr);
}

int main(void) {
  test_init();
  test_push_one();
  test_push_many();
  test_pop();
  test_get_out_of_range();
  test_last();
  test_clear();
  test_insert();
  test_remove();
  test_foreach();
  test_struct();
  printf("All dynarr tests passed!\n");
  return 0;
}

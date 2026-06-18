#include "render/types/types.h"
#include <assert.h>
#include <stdio.h>

static void test_vertex_stride(void) {
  assert(sizeof(Vertex) == 48);
}

int main(void) {
  test_vertex_stride();
  printf("All vertex tests passed!\n");
  return 0;
}

#ifndef MESH_H
#define MESH_H

#include "render/device.h"
#include "types.h"
#include "utils/globals.h"
#include <vulkan/vulkan.h>

#define CUBE_VERTICES                                                          \
  {{-1, -1, -1}, {0, 0, -1}, COLOR_WHITE, 0, 0},                               \
      {{1, -1, -1}, {0, 0, -1}, COLOR_WHITE, 0, 0},                            \
      {{1, 1, -1}, {0, 0, -1}, COLOR_WHITE, 0, 0},                             \
      {{-1, 1, -1}, {0, 0, -1}, COLOR_WHITE, 0, 0},                            \
      {{-1, -1, 1}, {0, 0, 1}, COLOR_WHITE, 0, 0},                             \
      {{1, -1, 1}, {0, 0, 1}, COLOR_WHITE, 0, 0},                              \
      {{1, 1, 1}, {0, 0, 1}, COLOR_WHITE, 0, 0},                               \
      {{-1, 1, 1}, {0, 0, 1}, COLOR_WHITE, 0, 0}

#define CUBE_INDICES                                                           \
  {0, 1, 2, 0, 2, 3, 4, 6, 5, 4, 7, 6, 0, 7, 4, 0, 3, 7,                       \
   1, 5, 6, 1, 6, 2, 0, 5, 1, 0, 4, 5, 3, 6, 7, 3, 2, 6}

typedef struct {
  Buffer vertexBuffer;
  Buffer indexBuffer;
  u32 vertexCount;
  u32 indexCount;
} Mesh;

Mesh mesh_create(Device *device, Vertex *vertices, u32 vertexCount,
                 u32 *indices, u32 indexCount);
void mesh_destroy(Mesh *mesh, VkDevice device);

#endif

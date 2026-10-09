#ifndef MESH_H
#define MESH_H

#include "render/device.h"
#include "types.h"
#include "utils/globals.h"
#include <vulkan/vulkan.h>

// 24 verts (4 per face): full 0-1 UVs on every face so a texture reads
// upright and non-mirrored when viewed from outside that face.
#define CUBE_VERTICES                                                          \
  {{-1, -1, -1}, {0, 0, -1}, COLOR_WHITE, 1, 1},                                 \
      {{1, -1, -1}, {0, 0, -1}, COLOR_WHITE, 0, 1},                             \
      {{1, 1, -1}, {0, 0, -1}, COLOR_WHITE, 0, 0},                              \
      {{-1, 1, -1}, {0, 0, -1}, COLOR_WHITE, 1, 0},                             \
      {{-1, -1, 1}, {0, 0, 1}, COLOR_WHITE, 0, 1},                              \
      {{1, -1, 1}, {0, 0, 1}, COLOR_WHITE, 1, 1},                               \
      {{1, 1, 1}, {0, 0, 1}, COLOR_WHITE, 1, 0},                                \
      {{-1, 1, 1}, {0, 0, 1}, COLOR_WHITE, 0, 0},                               \
      {{-1, -1, -1}, {-1, 0, 0}, COLOR_WHITE, 0, 1},                            \
      {{-1, -1, 1}, {-1, 0, 0}, COLOR_WHITE, 1, 1},                             \
      {{-1, 1, 1}, {-1, 0, 0}, COLOR_WHITE, 1, 0},                              \
      {{-1, 1, -1}, {-1, 0, 0}, COLOR_WHITE, 0, 0},                             \
      {{1, -1, -1}, {1, 0, 0}, COLOR_WHITE, 1, 1},                              \
      {{1, -1, 1}, {1, 0, 0}, COLOR_WHITE, 0, 1},                               \
      {{1, 1, 1}, {1, 0, 0}, COLOR_WHITE, 0, 0},                                \
      {{1, 1, -1}, {1, 0, 0}, COLOR_WHITE, 1, 0},                               \
      {{-1, -1, -1}, {0, -1, 0}, COLOR_WHITE, 0, 1},                            \
      {{1, -1, 1}, {0, -1, 0}, COLOR_WHITE, 1, 0},                              \
      {{1, -1, -1}, {0, -1, 0}, COLOR_WHITE, 1, 1},                             \
      {{-1, -1, 1}, {0, -1, 0}, COLOR_WHITE, 0, 0},                             \
      {{-1, 1, -1}, {0, 1, 0}, COLOR_WHITE, 0, 0},                              \
      {{1, 1, 1}, {0, 1, 0}, COLOR_WHITE, 1, 1},                                \
      {{-1, 1, 1}, {0, 1, 0}, COLOR_WHITE, 0, 1},                               \
      {{1, 1, -1}, {0, 1, 0}, COLOR_WHITE, 1, 0}

#define CUBE_INDICES                                                           \
  {0, 1, 2, 0, 2, 3, 4, 6, 5, 4, 7, 6, 8, 10, 9, 8, 11, 10,                   \
   12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21}

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

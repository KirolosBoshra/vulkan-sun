#ifndef MESH_H
#define MESH_H

#include "device.h"
#include "utils/globals.h"
#include "utils/types.h"
#include <vulkan/vulkan.h>

typedef struct {
  Buffer vertexBuffer;
  Buffer indexBuffer;
  u32 vertexCount;
  u32 indexCount;
} Mesh;

Mesh vs_mesh_create(VsDevice *device, Vertex *vertices, u32 vertexCount,
                    u32 *indices, u32 indexCount);
void vs_mesh_destroy(Mesh *mesh, VkDevice device);

#endif

#ifndef TYPES_H
#define TYPES_H

#include "color.h"
#include "device.h"
#include "globals.h"
#include <vulkan/vulkan.h>

typedef struct {
  f32 x, y;
} Vec2;

typedef struct {
  f32 x, y, z;
} Vec3;

typedef struct {
  f32 x, y, z, w;
} Vec4;

typedef struct {
  Vec3 position;
  Vec3 normal;
  VsColor color;
  f32 u, v;
} Vertex;

typedef struct {
  VkBuffer buffer;
  VkDeviceMemory memory;
  usize size;
} Buffer;

Buffer vs_create_buffer(usize size, VsDevice *device, VkBufferUsageFlags usage,
                        VkMemoryPropertyFlags properties);

void vs_destroy_buffer(Buffer *buffer, VkDevice device);

// <TODO> This is a bit of a hack, we should really be using command buffers
// and queues
void vs_copy_buffer(VsDevice *device, VkBuffer src, VkBuffer dst, usize size);

#endif

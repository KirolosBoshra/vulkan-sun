#ifndef TYPES_H
#define TYPES_H

#include "utils/color.h"
#include "render/device.h"
#include "utils/globals.h"
#include "utils/math.h"
#include <vulkan/vulkan.h>

typedef struct {
  Vec3 position;
  Vec3 normal;
  Color color;
  f32 u, v;
} Vertex;

typedef struct {
  VkBuffer buffer;
  VkDeviceMemory memory;
  usize size;
} Buffer;

Buffer create_buffer(usize size, Device *device, VkBufferUsageFlags usage,
                        VkMemoryPropertyFlags properties);

void destroy_buffer(Buffer *buffer, VkDevice device);

void copy_buffer(Device *device, VkBuffer src, VkBuffer dst, usize size);

u32 find_memory_type(VkPhysicalDevice physical, u32 typeBits,
                        VkMemoryPropertyFlags props);

#endif

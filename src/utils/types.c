#include "types.h"
#include "logger.h"
#include <stdlib.h>

static u32 find_memory_type(VkPhysicalDevice physical, u32 typeBits,
                            VkMemoryPropertyFlags props) {
  VkPhysicalDeviceMemoryProperties mem;
  vkGetPhysicalDeviceMemoryProperties(physical, &mem);
  for (u32 i = 0; i < mem.memoryTypeCount; i++)
    if ((typeBits & (1 << i)) &&
        (mem.memoryTypes[i].propertyFlags & props) == props)
      return i;
  return UINT32_MAX;
}

Buffer vs_create_buffer(usize size, VsDevice *device, VkBufferUsageFlags usage,
                        VkMemoryPropertyFlags properties) {
  Buffer buffer;

  // Create the buffer
  VkBufferCreateInfo bufferInfo = {0};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device->device, &bufferInfo, NULL, &buffer.buffer) !=
      VK_SUCCESS) {
    vs_log_error("Failed to create buffer!");
    return (Buffer){0};
  }

  // Get memory requirements
  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device->device, buffer.buffer,
                                &memRequirements);

  // Allocate memory
  VkMemoryAllocateInfo allocInfo = {0};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = find_memory_type(
      device->physical, memRequirements.memoryTypeBits, properties);
  if (vkAllocateMemory(device->device, &allocInfo, NULL, &buffer.memory) !=
      VK_SUCCESS) {
    vs_log_error("Failed to allocate buffer memory!");
    vkDestroyBuffer(device->device, buffer.buffer, NULL);
    return (Buffer){0};
  }

  // Bind memory to the buffer
  vkBindBufferMemory(device->device, buffer.buffer, buffer.memory, 0);
  buffer.size = size;

  return buffer;
}

void vs_destroy_buffer(Buffer *buffer, VkDevice device) {
  vkDestroyBuffer(device, buffer->buffer, NULL);
  vkFreeMemory(device, buffer->memory, NULL);
}

// <TODO> This is a bit of a hack, we should really be using command buffers
// and queues
void vs_copy_buffer(VsDevice *device, VkBuffer src, VkBuffer dst, usize size) {
  VkCommandPool pool;
  vkCreateCommandPool(device->device,
                      &(VkCommandPoolCreateInfo){
                          .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                          .queueFamilyIndex = device->graphicsFamily,
                      },
                      NULL, &pool);
  VkCommandBuffer cmd;
  vkAllocateCommandBuffers(
      device->device,
      &(VkCommandBufferAllocateInfo){
          .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
          .commandPool = pool,
          .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
          .commandBufferCount = 1,
      },
      &cmd);
  vkBeginCommandBuffer(cmd,
                       &(VkCommandBufferBeginInfo){
                           .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                           .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                       });
  vkCmdCopyBuffer(cmd, src, dst, 1, &(VkBufferCopy){.size = size});
  vkEndCommandBuffer(cmd);
  vkQueueSubmit(device->qQueue, 1,
                &(VkSubmitInfo){
                    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .commandBufferCount = 1,
                    .pCommandBuffers = &cmd,
                },
                VK_NULL_HANDLE);
  vkQueueWaitIdle(device->qQueue);
  vkDestroyCommandPool(device->device, pool, NULL);
}

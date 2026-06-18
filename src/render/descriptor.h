#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include "utils/globals.h"
#include <vulkan/vulkan.h>

// Manage multiple DescriptorPools creates new ones when pool is full

typedef struct {
  VkDescriptorType type;
  f32 ratio;
} PoolSizeRatio;

typedef struct {
  PoolSizeRatio *ratios;
  u32 ratioCount;
  u32 setsPerPool;

  VkDescriptorPool *readyPools;
  u32 readyCount;
  u32 readyCap;

  VkDescriptorPool *fullPools;
  u32 fullCount;
  u32 fullCap;
} DescriptorAllocator;

void desc_alloc_init(DescriptorAllocator *a, u32 initialSets,
                        PoolSizeRatio *ratios, u32 ratioCount);
void desc_alloc_destroy(DescriptorAllocator *a, VkDevice device);
void desc_alloc_clear(DescriptorAllocator *a, VkDevice device);
VkDescriptorSet desc_alloc_allocate(DescriptorAllocator *a,
                                       VkDevice device,
                                       VkDescriptorSetLayout layout);

typedef struct {
  VkDescriptorSetLayoutBinding *binds;
  u32 count;
} DescriptorLayoutBuilder;

void desc_layout_init(DescriptorLayoutBuilder *b);
void desc_layout_add(DescriptorLayoutBuilder *b, u32 binding,
                        VkDescriptorType type, VkShaderStageFlags stage);
VkDescriptorSetLayout desc_layout_build(DescriptorLayoutBuilder *b,
                                           VkDevice device);
void desc_layout_cleanup(DescriptorLayoutBuilder *b);

// Lightweight: use local arrays or caller-provided storage, no dynamic allocs.

typedef struct {
  VkDescriptorBufferInfo *bufferInfos;
  VkDescriptorImageInfo *imageInfos;
  VkWriteDescriptorSet *writes;
  u32 writeCount;
  u32 bufferCount;
  u32 imageCount;
  u32 maxWrites;
} DescriptorWriter;

void desc_writer_init(DescriptorWriter *w,
                         VkDescriptorBufferInfo *bufInfos,
                         VkDescriptorImageInfo *imgInfos,
                         VkWriteDescriptorSet *writes, u32 maxWrites);
void desc_writer_write_buffer(DescriptorWriter *w, u32 binding,
                                 VkBuffer buffer, usize size, usize offset,
                                 VkDescriptorType type);
void desc_writer_write_image(DescriptorWriter *w, u32 binding,
                                VkImageView imageView, VkSampler sampler,
                                VkImageLayout layout, VkDescriptorType type);
void desc_writer_update(DescriptorWriter *w, VkDevice device,
                           VkDescriptorSet set);

#endif

#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include "utils/globals.h"
#include <vulkan/vulkan.h>

// Manage multiple DescriptorPools creates new ones when pool is full

typedef struct {
  VkDescriptorType type;
  f32 ratio;
} VsPoolSizeRatio;

typedef struct {
  VsPoolSizeRatio *ratios;
  u32 ratioCount;
  u32 setsPerPool;

  VkDescriptorPool *readyPools;
  u32 readyCount;
  u32 readyCap;

  VkDescriptorPool *fullPools;
  u32 fullCount;
  u32 fullCap;
} VsDescriptorAllocator;

void vs_desc_alloc_init(VsDescriptorAllocator *a, u32 initialSets,
                        VsPoolSizeRatio *ratios, u32 ratioCount);
void vs_desc_alloc_destroy(VsDescriptorAllocator *a, VkDevice device);
void vs_desc_alloc_clear(VsDescriptorAllocator *a, VkDevice device);
VkDescriptorSet vs_desc_alloc_allocate(VsDescriptorAllocator *a,
                                       VkDevice device,
                                       VkDescriptorSetLayout layout);

typedef struct {
  VkDescriptorSetLayoutBinding *binds;
  u32 count;
} VsDescriptorLayoutBuilder;

void vs_desc_layout_init(VsDescriptorLayoutBuilder *b);
void vs_desc_layout_add(VsDescriptorLayoutBuilder *b, u32 binding,
                        VkDescriptorType type, VkShaderStageFlags stage);
VkDescriptorSetLayout vs_desc_layout_build(VsDescriptorLayoutBuilder *b,
                                           VkDevice device);
void vs_desc_layout_cleanup(VsDescriptorLayoutBuilder *b);

// Lightweight: use local arrays or caller-provided storage, no dynamic allocs.

typedef struct {
  VkDescriptorBufferInfo *bufferInfos;
  VkDescriptorImageInfo *imageInfos;
  VkWriteDescriptorSet *writes;
  u32 writeCount;
  u32 bufferCount;
  u32 imageCount;
  u32 maxWrites;
} VsDescriptorWriter;

void vs_desc_writer_init(VsDescriptorWriter *w,
                         VkDescriptorBufferInfo *bufInfos,
                         VkDescriptorImageInfo *imgInfos,
                         VkWriteDescriptorSet *writes, u32 maxWrites);
void vs_desc_writer_write_buffer(VsDescriptorWriter *w, u32 binding,
                                 VkBuffer buffer, usize size, usize offset,
                                 VkDescriptorType type);
void vs_desc_writer_write_image(VsDescriptorWriter *w, u32 binding,
                                VkImageView imageView, VkSampler sampler,
                                VkImageLayout layout, VkDescriptorType type);
void vs_desc_writer_update(VsDescriptorWriter *w, VkDevice device,
                           VkDescriptorSet set);

#endif

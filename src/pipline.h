#ifndef PIPLINE_H
#define PIPLINE_H

#include "shader.h"
#include <vulkan/vulkan.h>

typedef struct {
  VkPipeline handle;
  VkPipelineLayout layout;
} VsPipeline;

VsPipeline *vs_pipeline_create(VkDevice device, VkFormat colorFormat,
                               VsShader vert, VsShader frag);
void vs_pipeline_destroy(VsPipeline *pipeline, VkDevice device);

#endif

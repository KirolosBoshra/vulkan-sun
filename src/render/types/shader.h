#ifndef SHADER_H
#define SHADER_H

#include <vulkan/vulkan.h>

typedef struct {
  VkShaderModule module;
  VkShaderStageFlagBits stage;
} Shader;

Shader shader_create(VkDevice device, VkShaderStageFlagBits stage,
                          const char *spvPath);
void shader_destroy(VkDevice device, Shader *shader);

#endif

#ifndef SHADER_H
#define SHADER_H

#include <vulkan/vulkan.h>

typedef struct {
    VkShaderModule module;
    VkShaderStageFlagBits stage;
} VsShader;

VsShader vs_shader_create(VkDevice device, VkShaderStageFlagBits stage,
                          const char *spvPath);
void vs_shader_destroy(VkDevice device, VsShader *shader);

#endif

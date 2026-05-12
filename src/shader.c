#include "logger.h"
#include "shader.h"
#include <stdio.h>
#include <stdlib.h>

VsShader vs_shader_create(VkDevice device, VkShaderStageFlagBits stage,
                           const char *spvPath) {
  vs_log_trace("loading shader: %s", spvPath);

  VsShader shader = {VK_NULL_HANDLE, stage};

  FILE *fp;
  fopen_s(&fp, spvPath, "rb");
  if (!fp) {
    vs_log_error("failed to open shader: %s", spvPath);
    return shader;
  }

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  rewind(fp);

  uint32_t *code = malloc((size_t)size);
  fread(code, 1, (size_t)size, fp);
  fclose(fp);

  VkShaderModuleCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = (size_t)size,
      .pCode = code,
  };

  VkResult r = vkCreateShaderModule(device, &info, NULL, &shader.module);
  free(code);

  if (r != VK_SUCCESS) {
    vs_log_error("failed to create shader module: %s", spvPath);
    shader.module = VK_NULL_HANDLE;
  }

  return shader;
}

void vs_shader_destroy(VkDevice device, VsShader *shader) {
  if (shader && shader->module)
    vkDestroyShaderModule(device, shader->module, NULL);
}

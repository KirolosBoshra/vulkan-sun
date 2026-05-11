#include "app.h"
#include <stdio.h>
#include <stdlib.h>

VsApp *vs_app_create(const char *name, uint32_t version) {

  VsApp *app = malloc(sizeof(VsApp));
  if (!app) {
    fprintf(stderr, "failed to allocate app\n");
    return NULL;
  }

  uint32_t extCount = 0;
  const char **extensions = glfwGetRequiredInstanceExtensions(&extCount);

  VkApplicationInfo appInfo = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = name,
      .applicationVersion = version,
      .pEngineName = "vulken_sun",
      .engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
      .apiVersion = VK_API_VERSION_1_3,
  };

  VkInstanceCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &appInfo,
      .enabledExtensionCount = extCount,
      .ppEnabledExtensionNames = extensions,
  };

  if (vkCreateInstance(&createInfo, NULL, &app->instance) != VK_SUCCESS) {
    fprintf(stderr, "failed to create vulkan instance\n");
    free(app);
    return NULL;
  }

  return app;
}

void vs_app_destroy(VsApp *app) {
  if (!app)
    return;
  vkDestroyInstance(app->instance, NULL);
  free(app);
}

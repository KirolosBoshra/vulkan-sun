#include "device.h"
#include <stdio.h>
#include <stdlib.h>

int find_queue_family(VkPhysicalDevice physical, VkSurfaceKHR surface,
                      int *graphicsFamily, int *presentFamily) {
  *graphicsFamily = -1;
  *presentFamily = -1;

  uint32_t count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physical, &count, NULL);
  if (!count)
    return -1;

  VkQueueFamilyProperties *props = malloc(count * sizeof *props);
  vkGetPhysicalDeviceQueueFamilyProperties(physical, &count, props);

  for (size_t i = 0; i < count; i++) {
    if ((props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && *graphicsFamily < 0)
      *graphicsFamily = (int)i;

    VkBool32 supported = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(physical, i, surface, &supported);
    if (supported && *presentFamily < 0)
      *presentFamily = (int)i;

    if (*graphicsFamily >= 0 && *presentFamily >= 0)
      break;
  }

  free(props);
  return (*graphicsFamily >= 0 && *presentFamily >= 0) ? 0 : -1;
}

VsDevice *vs_device_create(VkInstance instance, VkSurfaceKHR surface) {
  VsDevice *device = malloc(sizeof *device);
  if (!device) {
    fprintf(stderr, "failed to allocate device\n");
    return NULL;
  }

  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
  if (!deviceCount) {
    fprintf(stderr, "no Vulkan-capable GPUs found\n");
    free(device);
    return NULL;
  }

  VkPhysicalDevice *devs = malloc(deviceCount * sizeof *devs);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devs);

  int graphicsFamily = -1, presentFamily = -1;
  device->physical = VK_NULL_HANDLE;

  for (uint32_t i = 0; i < deviceCount; i++) {
    int g, p;
    if (find_queue_family(devs[i], surface, &g, &p) == 0) {
      device->physical = devs[i];
      graphicsFamily = g;
      presentFamily = p;
      break;
    }
  }
  free(devs);

  if (device->physical == VK_NULL_HANDLE) {
    fprintf(stderr, "no suitable GPU found\n");
    free(device);
    return NULL;
  }

  VkPhysicalDeviceVulkan13Features check13 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
  };
  VkPhysicalDeviceVulkan12Features check12 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
      .pNext = &check13,
  };
  VkPhysicalDeviceFeatures2 check = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
      .pNext = &check12,
  };
  vkGetPhysicalDeviceFeatures2(device->physical, &check);

  if (!check13.dynamicRendering) {
    fprintf(stderr, "GPU does not support dynamicRendering (Vulkan 1.3)\n");
    free(device);
    return NULL;
  }
  if (!check13.synchronization2) {
    fprintf(stderr, "GPU does not support synchronization2 (Vulkan 1.3)\n");
    free(device);
    return NULL;
  }
  if (!check12.bufferDeviceAddress) {
    fprintf(stderr, "GPU does not support bufferDeviceAddress (Vulkan 1.2)\n");
    free(device);
    return NULL;
  }
  if (!check12.descriptorIndexing) {
    fprintf(stderr, "GPU does not support descriptorIndexing (Vulkan 1.2)\n");
    free(device);
    return NULL;
  }

  VkPhysicalDeviceVulkan13Features enable13 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
      .dynamicRendering = VK_TRUE,
      .synchronization2 = VK_TRUE,
  };
  VkPhysicalDeviceVulkan12Features enable12 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
      .bufferDeviceAddress = VK_TRUE,
      .descriptorIndexing = VK_TRUE,
      .pNext = &enable13,
  };

  float priority = 1.0f;
  uint32_t indices[] = {(uint32_t)graphicsFamily, (uint32_t)presentFamily};
  uint32_t familyCount = (graphicsFamily == presentFamily) ? 1 : 2;

  VkDeviceQueueCreateInfo queueInfos[2] = {0};
  for (uint32_t i = 0; i < familyCount; i++) {
    queueInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfos[i].queueFamilyIndex = indices[i];
    queueInfos[i].queueCount = 1;
    queueInfos[i].pQueuePriorities = &priority;
  }

  const char *extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  VkDeviceCreateInfo ci = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = &enable12,
      .queueCreateInfoCount = familyCount,
      .pQueueCreateInfos = queueInfos,
      .enabledExtensionCount = 1,
      .ppEnabledExtensionNames = extensions,
  };

  if (vkCreateDevice(device->physical, &ci, NULL, &device->device) !=
      VK_SUCCESS) {
    fprintf(stderr, "failed to create logical device\n");
    free(device);
    return NULL;
  }

  device->graphicsFamily = (uint32_t)graphicsFamily;
  device->presentFamily = (uint32_t)presentFamily;

  vkGetDeviceQueue(device->device, device->graphicsFamily, 0, &device->qQueue);
  vkGetDeviceQueue(device->device, device->presentFamily, 0, &device->pQueue);

  return device;
}

void vs_device_destroy(VsDevice *device) {
  if (!device)
    return;
  vkDestroyDevice(device->device, NULL);
  free(device);
}

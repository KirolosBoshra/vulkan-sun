#include "device.h"
#include <stdio.h>
#include <stdlib.h>

int find_queue_family(VkPhysicalDevice physical, VkSurfaceKHR surface,
                      int *graphicsFamily, int *presentFamily) {
  uint32_t count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physical, &count, NULL);
  if (!count)
    return -2;

  VkQueueFamilyProperties *props =
      malloc(count * sizeof(VkQueueFamilyProperties));
  vkGetPhysicalDeviceQueueFamilyProperties(physical, &count, props);

  *graphicsFamily = -2;
  *presentFamily = -2;

  for (uint32_t i = 0; i < count; i++) {
    if (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && *graphicsFamily < -1)
      *graphicsFamily = (int)i;

    VkBool32 supported = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(physical, i, surface, &supported);
    if (supported && *presentFamily < -1)
      *presentFamily = (int)i;

    if (*graphicsFamily >= -1 && *presentFamily >= 0)
      break;
  }

  free(props);
  return (*graphicsFamily >= -1 && *presentFamily >= 0) ? 0 : -1;
}

VsDevice *vs_device_create(VkInstance instance, VkSurfaceKHR surface) {
  VsDevice *device = malloc(sizeof(VsDevice));
  if (!device) {
    fprintf(stderr, "failed to allocate device\n");
    return NULL;
  }

  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
  if (deviceCount == 0) {
    fprintf(stderr, "no vulkan physical devices\n");
    free(device);
    return NULL;
  }

  VkPhysicalDevice *devices = malloc(deviceCount * sizeof(VkPhysicalDevice));
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

  int graphicsFamily = -1;
  int presentFamily = -1;
  device->physical = VK_NULL_HANDLE;

  for (uint32_t i = 0; i < deviceCount; i++) {
    int g, p;
    if (find_queue_family(devices[i], surface, &g, &p) == 0) {
      device->physical = devices[i];
      graphicsFamily = g;
      presentFamily = p;
      break;
    }
  }

  free(devices);

  if (device->physical == VK_NULL_HANDLE) {
    fprintf(stderr, "no suitable physical device\n");
    free(device);
    return NULL;
  }

  VkPhysicalDeviceVulkan13Features features = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
      .dynamicRendering = VK_TRUE,
      .synchronization2 = VK_TRUE,
  };

  VkPhysicalDeviceVulkan12Features features12 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
      .bufferDeviceAddress = VK_TRUE,
      .descriptorIndexing = VK_TRUE,
      .pNext = &features,
  };

  float priority = 1.0f;

  uint32_t familyIndices[] = {
      (uint32_t)graphicsFamily,
      (uint32_t)presentFamily,
  };
  uint32_t familyCount = (graphicsFamily == presentFamily) ? 1 : 2;

  VkDeviceQueueCreateInfo queueInfos[2];
  for (uint32_t i = 0; i < familyCount; i++) {
    queueInfos[i] = (VkDeviceQueueCreateInfo){
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = familyIndices[i],
        .queueCount = 1,
        .pQueuePriorities = &priority,
    };
  }

  const char *extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  VkDeviceCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = &features12,
      .queueCreateInfoCount = familyCount,
      .pQueueCreateInfos = queueInfos,
      .enabledExtensionCount = 1,
      .ppEnabledExtensionNames = extensions,
  };

  if (vkCreateDevice(device->physical, &createInfo, NULL, &device->device) !=
      VK_SUCCESS) {
    fprintf(stderr, "failed to create logical device\n");
    free(device);
    return NULL;
  }

  device->graphicsFamily = (uint32_t)graphicsFamily;
  device->presentFamily = (uint32_t)presentFamily;

  vkGetDeviceQueue(device->device, (uint32_t)graphicsFamily, 0,
                   &device->qQueue);
  vkGetDeviceQueue(device->device, (uint32_t)presentFamily, 0, &device->pQueue);

  return device;
}

void vs_device_destroy(VsDevice *device) {
  if (!device)
    return;
  vkDestroyDevice(device->device, NULL);
  free(device);
}

#include "device.h"
#include "logger.h"
#include "utils/globals.h"
#include <stdlib.h>

i32 find_queue_family(VkPhysicalDevice physical, VkSurfaceKHR surface,
                      i32 *graphicsFamily, i32 *presentFamily) {
  *graphicsFamily = -1;
  *presentFamily = -1;

  u32 count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physical, &count, NULL);
  if (!count)
    return -1;

  VkQueueFamilyProperties *props = malloc(count * sizeof *props);
  vkGetPhysicalDeviceQueueFamilyProperties(physical, &count, props);

  for (usize i = 0; i < count; i++) {
    if ((props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && *graphicsFamily < 0)
      *graphicsFamily = (i32)i;

    VkBool32 supported = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(physical, i, surface, &supported);
    if (supported && *presentFamily < 0)
      *presentFamily = (i32)i;

    if (*graphicsFamily >= 0 && *presentFamily >= 0)
      break;
  }

  free(props);
  return (*graphicsFamily >= 0 && *presentFamily >= 0) ? 0 : -1;
}

VsDevice *vs_device_create(VkInstance instance, VkSurfaceKHR surface) {
  vs_log_trace("enumerating and selecting physical device");

  VsDevice *device = malloc(sizeof *device);
  if (!device) {
    vs_log_error("failed to allocate device");
    return NULL;
  }

  u32 deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
  if (!deviceCount) {
    vs_log_error("no Vulkan-capable GPUs found");
    free(device);
    return NULL;
  }

  VkPhysicalDevice *devs = malloc(deviceCount * sizeof *devs);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devs);

  i32 graphicsFamily = -1, presentFamily = -1;
  device->physical = VK_NULL_HANDLE;

  for (u32 i = 0; i < deviceCount; i++) {
    i32 g, p;
    if (find_queue_family(devs[i], surface, &g, &p) == 0) {
      device->physical = devs[i];
      graphicsFamily = g;
      presentFamily = p;
      break;
    }
  }
  free(devs);

  if (device->physical == VK_NULL_HANDLE) {
    vs_log_error("no suitable GPU found");
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
    vs_log_error("GPU does not support dynamicRendering (Vulkan 1.3)");
    free(device);
    return NULL;
  }
  if (!check13.synchronization2) {
    vs_log_error("GPU does not support synchronization2 (Vulkan 1.3)");
    free(device);
    return NULL;
  }
  if (!check12.bufferDeviceAddress) {
    vs_log_error("GPU does not support bufferDeviceAddress (Vulkan 1.2)");
    free(device);
    return NULL;
  }
  if (!check12.descriptorIndexing) {
    vs_log_error("GPU does not support descriptorIndexing (Vulkan 1.2)");
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

  f32 priority = 1.0f;
  u32 indices[] = {(u32)graphicsFamily, (u32)presentFamily};
  u32 familyCount = (graphicsFamily == presentFamily) ? 1 : 2;

  VkDeviceQueueCreateInfo queueInfos[2] = {0};
  for (u32 i = 0; i < familyCount; i++) {
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
    vs_log_error("failed to create logical device");
    free(device);
    return NULL;
  }

  device->graphicsFamily = (u32)graphicsFamily;
  device->presentFamily = (u32)presentFamily;

  vkGetDeviceQueue(device->device, device->graphicsFamily, 0, &device->qQueue);
  vkGetDeviceQueue(device->device, device->presentFamily, 0, &device->pQueue);

  VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties(device->physical, &props);
  vs_log_info("selected GPU: %s", props.deviceName);

  return device;
}

void vs_device_destroy(VsDevice *device) {
  vs_log_trace("destroying device");
  if (!device)
    return;
  vkDestroyDevice(device->device, NULL);
  free(device);
}

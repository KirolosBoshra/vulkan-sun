#include "app/app.h"
#include "core/core.h"
#include "core/window.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

i32 main(void) {
  init();

  App *app = app_create("buffer_test", VK_MAKE_API_VERSION(0, 0, 1, 0));
  assert(app);

  Window *window = window_create(app->instance, 64, 64, "buffer_test");
  assert(window);

  Device *device = device_create(app->instance, window->surface);
  assert(device);

  Buffer buf = create_buffer(64, device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  assert(buf.buffer != VK_NULL_HANDLE);
  assert(buf.memory != VK_NULL_HANDLE);
  assert(buf.size == 64);

  void *mapped;
  vkMapMemory(device->device, buf.memory, 0, 64, 0, &mapped);
  memset(mapped, 0xAB, 64);
  vkUnmapMemory(device->device, buf.memory);

  u32 data[] = {1, 2, 3, 4, 5, 6, 7, 8};
  Buffer buf2 =
      create_buffer(sizeof(data), device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  vkMapMemory(device->device, buf2.memory, 0, sizeof(data), 0, &mapped);
  memcpy(mapped, data, sizeof(data));
  vkUnmapMemory(device->device, buf2.memory);

  u32 readback[8];
  vkMapMemory(device->device, buf2.memory, 0, sizeof(readback), 0, &mapped);
  memcpy(readback, mapped, sizeof(readback));
  vkUnmapMemory(device->device, buf2.memory);
  for (i32 i = 0; i < 8; i++)
    assert(readback[i] == (u32)(i + 1));

  destroy_buffer(&buf2, device->device);
  destroy_buffer(&buf, device->device);
  device_destroy(device);
  window_destroy(window);
  app_destroy(app);
  terminate();
  printf("All buffer tests passed!\n");
  return 0;
}

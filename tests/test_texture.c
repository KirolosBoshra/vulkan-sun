#include "app/app.h"
#include "core/core.h"
#include "core/window.h"
#include "render/resource.h"
#include <assert.h>
#include <stdio.h>

#ifndef ASSETS_DIR
#define ASSETS_DIR "assets"
#endif

int main(void) {
  init();

  App *app = app_create("texture_test", VK_MAKE_API_VERSION(0, 0, 1, 0));
  assert(app);

  Window *window = window_create(app->instance, 64, 64, "texture_test");
  assert(window);

  Device *device = device_create(app->instance, window->surface);
  assert(device);

  app->device = device;
  app->renderer = renderer_create(device, sizeof(Mat4));
  assert(app->renderer);
  app->resources = create_resource_manager();
  assert(app->resources);

  VkSamplerCreateInfo sampl = {.sType =
                                   VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
  sampl.magFilter = VK_FILTER_NEAREST;
  sampl.minFilter = VK_FILTER_NEAREST;
  assert(vkCreateSampler(device->device, &sampl, NULL,
                         &app->samplerNearest) == VK_SUCCESS);
  sampl.magFilter = VK_FILTER_LINEAR;
  sampl.minFilter = VK_FILTER_LINEAR;
  assert(vkCreateSampler(device->device, &sampl, NULL,
                         &app->samplerLinear) == VK_SUCCESS);

  // Test 1: image_create produces a valid sampled image
  Image img = image_create(device, (VkExtent3D){2, 2, 1},
                           VK_FORMAT_R8G8B8A8_SRGB,
                           VK_IMAGE_USAGE_SAMPLED_BIT,
                           VK_IMAGE_ASPECT_COLOR_BIT, 1);
  assert(img.image != VK_NULL_HANDLE);
  assert(img.view != VK_NULL_HANDLE);
  assert(img.format == VK_FORMAT_R8G8B8A8_SRGB);
  assert(img.extent.width == 2 && img.extent.height == 2);

  // Test 2: image_create_from_data uploads pixels
  u8 pixels[2 * 2 * 4] = {255, 0, 255, 255, 0,   0,   0,   255,
                          0,   0, 0,   255, 255, 0,   255, 255};
  Image data_img = image_create_from_data(
      device, pixels, (VkExtent3D){2, 2, 1}, VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 1);
  assert(data_img.image != VK_NULL_HANDLE);

  image_destroy(&img, device->device);
  image_destroy(&data_img, device->device);

  // Test 3: image_load decodes the real asset to SRGB
  Image logo = image_load(device, ASSETS_DIR "/logo.png");
  assert(logo.image != VK_NULL_HANDLE);
  assert(logo.extent.width == 256 && logo.extent.height == 256);
  assert(logo.format == VK_FORMAT_R8G8B8A8_SRGB);
  image_destroy(&logo, device->device);

  // Test 4: image_load fails clean on missing file and bad extension
  Image missing = image_load(device, ASSETS_DIR "/does_not_exist.png");
  assert(missing.image == VK_NULL_HANDLE);
  Image bad_ext = image_load(device, "notes.txt");
  assert(bad_ext.image == VK_NULL_HANDLE);

  // Test 5: resource upload/get/free/reuse, mirroring the mesh tests
  ImageHandle h1 = resource_upload_image(
      app->resources, device, pixels, (VkExtent3D){2, 2, 1},
      VK_FORMAT_R8G8B8A8_SRGB, 1);
  assert(h1 != INVALID_HANDLE);
  Image *got = resource_get_image(app->resources, h1);
  assert(got != NULL);
  assert(got->extent.width == 2);
  assert(resource_get_image(app->resources, 999) == NULL);

  resource_free_image(app->resources, device->device, h1);
  assert(resource_get_image(app->resources, h1) == NULL);

  ImageHandle h2 = resource_upload_image(
      app->resources, device, pixels, (VkExtent3D){2, 2, 1},
      VK_FORMAT_R8G8B8A8_SRGB, 1);
  assert(h2 == h1);

  // Test 6: create_texture fills dims from the image
  Texture tex = create_texture(app, h2);
  assert(tex.image == h2);
  assert(tex.width == 2 && tex.height == 2);

  // Test 7: load_texture end-to-end
  Texture logo_tex = load_texture(app, ASSETS_DIR "/logo.png");
  assert(logo_tex.image != INVALID_HANDLE);
  assert(logo_tex.width == 256 && logo_tex.height == 256);

  // Test 8: texture_bind produces a set for both sampler branches
  VkDescriptorSet linear_set = texture_bind(app, &logo_tex);
  assert(linear_set != VK_NULL_HANDLE);

  Texture nearest_tex = logo_tex;
  nearest_tex.filter = TEX_FILTER_NEAREST;
  VkDescriptorSet nearest_set = texture_bind(app, &nearest_tex);
  assert(nearest_set != VK_NULL_HANDLE);

  // Test 9: texture_bind rejects bad input (INVALID_HANDLE is the
  // empty sentinel — handle 0 is valid, so {0} would bind image 0)
  Texture empty = {.image = INVALID_HANDLE};
  assert(texture_bind(app, &empty) == VK_NULL_HANDLE);
  assert(texture_bind(app, NULL) == VK_NULL_HANDLE);

  window_destroy(window);
  app_destroy(app);
  terminate();

  printf("All texture tests passed!\n");
  return 0;
}

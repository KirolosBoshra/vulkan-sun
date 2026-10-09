#ifndef TEXTURE_H
#define TEXTURE_H

#include "core/logger.h"
#include "render/resource.h"
#include "utils/color.h"

typedef enum {
  TEX_FILTER_LINEAR,
  TEX_FILTER_NEAREST,
} TexFilter;

typedef enum {
  TEX_WRAP_REPEAT,
  TEX_WRAP_CLAMP,
} TexWrap;

typedef struct {
  ImageHandle image;
  i32 width;
  i32 height;
  u32 mipLevels;
  TexFilter filter;
  TexWrap wrap;
  Color color;
} Texture;

static inline Texture texture_create_from_handle(ImageHandle handle,
                                                 ResourceManager *res) {
  Image *img = resource_get_image(res, handle);
  if (!img)
    log_error("failed to get image from resource manager for handle %u",
              handle);
  return (Texture){.image = INVALID_HANDLE};

  return (Texture){
      .image = handle,
      .width = img->extent.width,
      .height = img->extent.height,
      .mipLevels = 0,
      .filter = TEX_FILTER_LINEAR,
      .wrap = TEX_WRAP_REPEAT,
      .color = COLOR_WHITE,
  };
}

#endif // !TEXTURE_H

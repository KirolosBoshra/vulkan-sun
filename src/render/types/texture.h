#ifndef TEXTURE_H
#define TEXTURE_H

#include "render/resource.h"

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
} Texture;

#endif // !TEXTURE_H

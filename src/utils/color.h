#ifndef COLOR_H
#define COLOR_H
#include "utils/globals.h"

#include <vulkan/vulkan.h>

typedef struct {
  f32 r, g, b, a;
} VsColor;

static inline VkClearValue vs_color_clear_value(VsColor c) {
  return (VkClearValue){.color = {.float32 = {c.r, c.g, c.b, c.a}}};
}

// predefined colors
#define VS_COLOR_BLACK ((VsColor){0.0f, 0.0f, 0.0f, 1.0f})
#define VS_COLOR_WHITE ((VsColor){1.0f, 1.0f, 1.0f, 1.0f})
#define VS_COLOR_RED ((VsColor){1.0f, 0.0f, 0.0f, 1.0f})
#define VS_COLOR_GREEN ((VsColor){0.0f, 1.0f, 0.0f, 1.0f})
#define VS_COLOR_BLUE ((VsColor){0.0f, 0.0f, 1.0f, 1.0f})
#define VS_COLOR_CYAN ((VsColor){0.0f, 1.0f, 1.0f, 1.0f})
#define VS_COLOR_MAGENTA ((VsColor){1.0f, 0.0f, 1.0f, 1.0f})
#define VS_COLOR_YELLOW ((VsColor){1.0f, 1.0f, 0.0f, 1.0f})
#define VS_COLOR_ORANGE ((VsColor){1.0f, 0.5f, 0.0f, 1.0f})
#define VS_COLOR_SKYBLUE ((VsColor){0.0f, 0.5f, 0.8f, 1.0f})
#define VS_COLOR_GRAY ((VsColor){0.5f, 0.5f, 0.5f, 1.0f})

#endif

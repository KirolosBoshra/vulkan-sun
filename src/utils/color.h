#ifndef COLOR_H
#define COLOR_H
#include "utils/globals.h"

#include <vulkan/vulkan.h>

typedef struct {
  f32 r, g, b, a;
} Color;

static inline VkClearValue color_clear_value(Color c) {
  return (VkClearValue){.color = {.float32 = {c.r, c.g, c.b, c.a}}};
}

// predefined colors
#define COLOR_BLACK ((Color){0.0f, 0.0f, 0.0f, 1.0f})
#define COLOR_WHITE ((Color){1.0f, 1.0f, 1.0f, 1.0f})
#define COLOR_RED ((Color){1.0f, 0.0f, 0.0f, 1.0f})
#define COLOR_GREEN ((Color){0.0f, 1.0f, 0.0f, 1.0f})
#define COLOR_BLUE ((Color){0.0f, 0.0f, 1.0f, 1.0f})
#define COLOR_CYAN ((Color){0.0f, 1.0f, 1.0f, 1.0f})
#define COLOR_MAGENTA ((Color){1.0f, 0.0f, 1.0f, 1.0f})
#define COLOR_YELLOW ((Color){1.0f, 1.0f, 0.0f, 1.0f})
#define COLOR_ORANGE ((Color){1.0f, 0.5f, 0.0f, 1.0f})
#define COLOR_SKYBLUE ((Color){0.0f, 0.5f, 0.8f, 1.0f})
#define COLOR_GRAY ((Color){0.5f, 0.5f, 0.5f, 1.0f})

#endif

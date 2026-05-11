#include "core.h"
#include "GLFW/glfw3.h"

void vs_init(void) {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void vs_terminate(void) { glfwTerminate(); }

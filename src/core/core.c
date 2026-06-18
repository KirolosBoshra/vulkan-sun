#include "core.h"
void init(void) {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void terminate(void) { glfwTerminate(); }

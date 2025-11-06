/**
 * @file main.cpp
 * @brief Prong UI Framework Demo Application
 *
 * Minimal boilerplate demonstrating scene-based architecture
 */

#include "../adapters/font_renderer.h"
#include "../adapters/glfw_window_adapter.h"
#include "../adapters/simple_opengl_renderer.h"
#include "scenes/demo_scene.h"
#include <GLFW/glfw3.h>

#include <cmath>
#include <filesystem>
#include <iostream>
#include <memory>

#ifdef __linux__
#include <sys/types.h>

#include <limits.h>
#include <unistd.h>
#elif __APPLE__
#include <mach-o/dyld.h>
#elif _WIN32
#include <windows.h>
#endif

std::string getExecutableDir() {
#ifdef __linux__
  char result[PATH_MAX];
  ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
  if (count != -1) {
    return std::filesystem::path(std::string(result, count)).parent_path().string();
  }
#elif __APPLE__
  char result[PATH_MAX];
  uint32_t size = sizeof(result);
  if (_NSGetExecutablePath(result, &size) == 0) {
    return std::filesystem::path(result).parent_path().string();
  }
#elif _WIN32
  char result[MAX_PATH];
  GetModuleFileNameA(NULL, result, MAX_PATH);
  return std::filesystem::path(result).parent_path().string();
#endif
  return ".";
}

int main() {
  // Initialize GLFW
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

  GLFWwindow* glfwWindow = glfwCreateWindow(1280, 720, "Prong Demo", nullptr, nullptr);
  if (!glfwWindow) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(glfwWindow);
  glfwSwapInterval(1);

  // Create adapters
  auto windowAdapter = std::make_unique<bombfork::prong::examples::GLFWWindowAdapter>(glfwWindow);
  auto renderer = std::make_unique<bombfork::prong::examples::SimpleOpenGLRenderer>();

  if (!renderer->initialize(1280, 720)) {
    std::cerr << "Failed to initialize renderer" << std::endl;
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
    return -1;
  }

  // Initialize font renderer
  auto fontRenderer = std::make_unique<bombfork::prong::examples::FontRenderer>();
  std::string fontPath = getExecutableDir() + "/fonts/MonofurNerdFont-Regular.ttf";
  if (fontRenderer->loadFont(fontPath, 24.0f)) {
    renderer->setFontRenderer(fontRenderer.get());
  }

  // Create and attach scene
  auto scene = std::make_unique<demo::DemoScene>(windowAdapter.get(), renderer.get(), glfwWindow);
  scene->attach();

  // Main loop
  double lastTime = glfwGetTime();
  while (!windowAdapter->shouldClose()) {
    double currentTime = glfwGetTime();
    double deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    glfwPollEvents();

    if (windowAdapter->isKeyPressed(GLFW_KEY_ESCAPE)) {
      glfwSetWindowShouldClose(glfwWindow, GLFW_TRUE);
    }

    if (renderer->beginFrame()) {
      renderer->clear(0.08f, 0.08f, 0.1f, 1.0f);

      scene->updateAll(deltaTime);
      scene->renderAll();

      renderer->endFrame();
    }

    scene->present();
    glfwSwapBuffers(glfwWindow);
  }

  // Cleanup
  scene->detach();
  scene.reset();
  renderer.reset();
  windowAdapter.reset();

  glfwDestroyWindow(glfwWindow);
  glfwTerminate();

  return 0;
}

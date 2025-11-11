/**
 * @file main.cpp
 * @brief TextInput example - Interactive text entry with copy/paste
 *
 * Demonstrates:
 * - Using TextInput component for user text entry
 * - Setting up GLFW adapters for clipboard and keyboard
 * - Handling text change callbacks
 * - Placeholder text
 * - Copy/paste functionality (Ctrl+C, Ctrl+V)
 */

#include "../../adapters/glfw_window_adapter.h"
#include "../../adapters/simple_opengl_renderer.h"
#include "../../common/glfw_adapters/glfw_adapters.h"
#include <GLFW/glfw3.h>
#include <bombfork/prong/components/button.h>
#include <bombfork/prong/components/panel.h>
#include <bombfork/prong/components/text_input.h>
#include <bombfork/prong/core/component_builder.h>
#include <bombfork/prong/core/scene.h>

#include <iostream>
#include <memory>

using namespace bombfork::prong;
using namespace bombfork::prong::examples;

int main() {
  // Initialize GLFW
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

  GLFWwindow* glfwWindow = glfwCreateWindow(800, 600, "07 - Text Input", nullptr, nullptr);
  if (!glfwWindow) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(glfwWindow);
  glfwSwapInterval(1);

  auto windowAdapter = std::make_unique<GLFWWindowAdapter>(glfwWindow);
  auto renderer = std::make_unique<SimpleOpenGLRenderer>();

  if (!renderer->initialize(800, 600)) {
    std::cerr << "Failed to initialize renderer" << std::endl;
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
    return -1;
  }

  auto scene = std::make_unique<Scene>(windowAdapter.get(), renderer.get());

  // Create GLFW adapters for clipboard and keyboard support
  // These are required for TextInput to support copy/paste and proper key handling
  auto adapters = glfw::GLFWAdapters::create(glfwWindow);

  // Create a container panel
  auto panel = create<Panel<>>().withSize(700, 500).withPosition(50, 50).build();

  // Create first text input - single line with placeholder
  auto nameInput =
    create<TextInput>()
      .withPlaceholder("Enter your name...")
      .withSize(400, 40)
      .withPosition(50, 50)
      .withTextChangedCallback([](const std::string& text) { std::cout << "Name changed: " << text << std::endl; })
      .build();

  // Inject adapters for clipboard and keyboard support
  nameInput->setClipboard(adapters.clipboard.get());
  nameInput->setKeyboard(adapters.keyboard.get());

  panel->addChild(std::move(nameInput));

  // Create second text input - email
  auto emailInput =
    create<TextInput>()
      .withPlaceholder("Enter your email...")
      .withSize(400, 40)
      .withPosition(50, 110)
      .withTextChangedCallback([](const std::string& text) { std::cout << "Email changed: " << text << std::endl; })
      .build();

  emailInput->setClipboard(adapters.clipboard.get());
  emailInput->setKeyboard(adapters.keyboard.get());

  panel->addChild(std::move(emailInput));

  // Create third text input - password (just for demo, not secure)
  auto passwordInput = create<TextInput>()
                         .withPlaceholder("Enter password...")
                         .withSize(400, 40)
                         .withPosition(50, 170)
                         .withTextChangedCallback([](const std::string& text) {
                           // In a real app, you wouldn't log passwords!
                           std::cout << "Password changed (length: " << text.length() << ")" << std::endl;
                         })
                         .build();

  passwordInput->setClipboard(adapters.clipboard.get());
  passwordInput->setKeyboard(adapters.keyboard.get());

  panel->addChild(std::move(passwordInput));

  // Create a fourth text input - multiline-style (though TextInput is single-line)
  auto commentsInput =
    create<TextInput>().withPlaceholder("Enter comments...").withSize(600, 40).withPosition(50, 230).build();

  commentsInput->setClipboard(adapters.clipboard.get());
  commentsInput->setKeyboard(adapters.keyboard.get());

  panel->addChild(std::move(commentsInput));

  // Add a submit button
  auto submitButton = create<Button>("Submit")
                        .withSize(150, 50)
                        .withPosition(50, 300)
                        .withClickCallback([]() {
                          std::cout << "\n=== Form Submitted ===" << std::endl;
                          std::cout << "Check the console output above for entered values" << std::endl;
                        })
                        .build();

  panel->addChild(std::move(submitButton));

  // Add a clear button
  auto clearButton = create<Button>("Clear All")
                       .withSize(150, 50)
                       .withPosition(220, 300)
                       .withClickCallback([]() { std::cout << "Clear button clicked (not implemented)" << std::endl; })
                       .build();

  panel->addChild(std::move(clearButton));

  scene->addChild(std::move(panel));

  scene->attach();

  std::cout << "TextInput Example" << std::endl;
  std::cout << "==================" << std::endl;
  std::cout << "- Click on a text field to focus it" << std::endl;
  std::cout << "- Type to enter text" << std::endl;
  std::cout << "- Use Ctrl+C to copy, Ctrl+V to paste" << std::endl;
  std::cout << "- Use Ctrl+A to select all" << std::endl;
  std::cout << "- Use arrow keys to move cursor" << std::endl;
  std::cout << "- Use Home/End for start/end of text" << std::endl;
  std::cout << "- Press Tab to move to next field" << std::endl;
  std::cout << "\nWatch the console for text change callbacks!" << std::endl;

  // Main loop
  while (!windowAdapter->shouldClose()) {
    glfwPollEvents();

    if (windowAdapter->isKeyPressed(GLFW_KEY_ESCAPE)) {
      glfwSetWindowShouldClose(glfwWindow, GLFW_TRUE);
    }

    if (renderer->beginFrame()) {
      renderer->clear(0.1f, 0.1f, 0.12f, 1.0f);

      scene->updateAll(0.016);
      scene->renderAll();

      renderer->endFrame();
    }

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

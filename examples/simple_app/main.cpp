/**
 * @file main.cpp
 * @brief Prong UI Framework Demo Application
 *
 * This example demonstrates:
 * 1. Window and renderer adapters (GLFW + OpenGL)
 * 2. Event dispatcher setup and component registration
 * 3. UI components: Panel, Button, TextInput, ListBox
 * 4. Parent-child component hierarchy
 * 5. Event callbacks and interactive elements
 * 6. Theming and color customization
 * 7. Main application loop with update/render cycle
 */

#include <bombfork/prong/components/button.h>
#include <bombfork/prong/components/list_box.h>
#include <bombfork/prong/components/panel.h>
#include <bombfork/prong/components/text_input.h>
#include <bombfork/prong/core/component.h>
#include <bombfork/prong/events/event_dispatcher.h>
#include <bombfork/prong/theming/color.h>
#include <bombfork/prong/theming/theme_manager.h>

// Include our example adapters
#include "../adapters/font_renderer.h"
#include "../adapters/glfw_window_adapter.h"
#include "../adapters/simple_opengl_renderer.h"
#include <GLFW/glfw3.h>

#include <cmath>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#ifdef __linux__
#include <sys/types.h>

#include <limits.h>
#include <unistd.h>
#elif __APPLE__
#include <mach-o/dyld.h>
#elif _WIN32
#include <windows.h>
#endif

using namespace bombfork::prong;

/**
 * @brief Get the directory containing the executable
 */
std::string getExecutableDir() {
#ifdef __linux__
  char result[PATH_MAX];
  ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
  if (count != -1) {
    std::filesystem::path exePath(std::string(result, count));
    return exePath.parent_path().string();
  }
#elif __APPLE__
  char result[PATH_MAX];
  uint32_t size = sizeof(result);
  if (_NSGetExecutablePath(result, &size) == 0) {
    std::filesystem::path exePath(result);
    return exePath.parent_path().string();
  }
#elif _WIN32
  char result[MAX_PATH];
  GetModuleFileNameA(NULL, result, MAX_PATH);
  std::filesystem::path exePath(result);
  return exePath.parent_path().string();
#endif
  return ".";
}

int main() {
  // === Initialize GLFW ===
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  // Create GLFW window
  // Use OpenGL 3.3 Compatibility Profile to support both modern and legacy rendering
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

  GLFWwindow* glfwWindow = glfwCreateWindow(1280, 720, "Prong Example", nullptr, nullptr);
  if (!glfwWindow) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(glfwWindow);
  glfwSwapInterval(1); // Enable vsync

  // === Initialize OpenGL (using GLAD or similar) ===
  // NOTE: You would need to initialize your OpenGL loader here
  // gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  std::cout << "Prong Example Application" << std::endl;
  std::cout << "=========================" << std::endl;

  // === Create Adapters ===
  auto windowAdapter = std::make_unique<examples::GLFWWindowAdapter>(glfwWindow);
  auto renderer = std::make_unique<examples::SimpleOpenGLRenderer>();

  if (!renderer->initialize(1280, 720)) {
    std::cerr << "Failed to initialize renderer" << std::endl;
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
    return -1;
  }

  std::cout << "Renderer: " << renderer->getName() << std::endl;

  // === Initialize Font Renderer ===
  auto fontRenderer = std::make_unique<examples::FontRenderer>();
  std::string exeDir = getExecutableDir();
  std::string fontPath = exeDir + "/fonts/MonofurNerdFont-Regular.ttf";
  std::cout << "Loading font from: " << fontPath << std::endl;

  if (!fontRenderer->loadFont(fontPath, 24.0f)) {
    std::cerr << "Warning: Failed to load font, text rendering will be unavailable" << std::endl;
  }

  // Set font renderer on the OpenGL renderer so UI components can use it
  renderer->setFontRenderer(fontRenderer.get());

  // === Create Event Dispatcher ===
  events::EventDispatcher dispatcher(windowAdapter.get());

  // === Set up Theme ===
  theming::ThemeManager::getInstance().setCurrentTheme("default");

  // === Create UI Components ===
  std::vector<std::unique_ptr<Component>> components;

  // Left Panel - Control Panel
  auto leftPanel = std::make_unique<Panel<>>();
  leftPanel->setRenderer(renderer.get());
  leftPanel->setBounds(20, 20, 300, 680);
  leftPanel->setBackgroundColor(theming::Color(0.15f, 0.15f, 0.18f, 1.0f));
  leftPanel->setBorderColor(theming::Color(0.3f, 0.3f, 0.35f, 1.0f));
  leftPanel->setBorderWidth(2);
  leftPanel->setTitle("Control Panel");
  dispatcher.registerComponent(leftPanel.get());

  // Text Input
  auto textInput = std::make_unique<TextInput>();
  textInput->setRenderer(renderer.get());
  textInput->setBounds(40, 80, 260, 30);
  textInput->setPlaceholder("Enter text here...");
  textInput->setOnTextChanged([](const std::string& text) { std::cout << "Text changed: " << text << std::endl; });
  dispatcher.registerComponent(textInput.get());

  // Button Row 1
  auto addButton = std::make_unique<Button>("Add Item");
  addButton->setRenderer(renderer.get());
  addButton->setBounds(40, 130, 120, 35);
  int clickCount = 0;
  auto listBoxPtr = std::make_unique<ListBox>();
  auto textInputPtr = textInput.get();
  addButton->setClickCallback([&clickCount, listBoxPtr = listBoxPtr.get(), textInputPtr]() {
    clickCount++;
    std::string newItem = "Item " + std::to_string(clickCount);
    if (!textInputPtr->getText().empty()) {
      newItem = textInputPtr->getText();
      textInputPtr->setText("");
    }
    listBoxPtr->addItem(newItem);
    std::cout << "Added: " << newItem << std::endl;
  });
  dispatcher.registerComponent(addButton.get());

  auto clearButton = std::make_unique<Button>("Clear");
  clearButton->setRenderer(renderer.get());
  clearButton->setBounds(180, 130, 120, 35);
  clearButton->setClickCallback([listBoxPtr = listBoxPtr.get()]() {
    listBoxPtr->clearItems();
    std::cout << "List cleared" << std::endl;
  });
  dispatcher.registerComponent(clearButton.get());

  // Button Row 2
  auto infoButton = std::make_unique<Button>("Show Info");
  infoButton->setRenderer(renderer.get());
  infoButton->setBounds(40, 180, 260, 35);
  infoButton->setClickCallback([]() {
    std::cout << "\n=== Prong UI Framework ===" << std::endl;
    std::cout << "A modern C++20 UI framework" << std::endl;
    std::cout << "Features:" << std::endl;
    std::cout << "  - Header-mostly architecture" << std::endl;
    std::cout << "  - CRTP for zero-cost abstractions" << std::endl;
    std::cout << "  - Renderer and window agnostic" << std::endl;
    std::cout << "  - Component-based architecture" << std::endl;
    std::cout << "=========================\n" << std::endl;
  });
  dispatcher.registerComponent(infoButton.get());

  // Exit Button
  auto exitButton = std::make_unique<Button>("Exit Application");
  exitButton->setRenderer(renderer.get());
  exitButton->setBounds(40, 650, 260, 35);
  exitButton->setBackgroundColor(theming::Color(0.6f, 0.2f, 0.2f, 1.0f));
  exitButton->setClickCallback([&glfwWindow]() {
    std::cout << "Exiting application..." << std::endl;
    glfwSetWindowShouldClose(glfwWindow, GLFW_TRUE);
  });
  dispatcher.registerComponent(exitButton.get());

  // Right Panel - Display Area
  auto rightPanel = std::make_unique<Panel<>>();
  rightPanel->setRenderer(renderer.get());
  rightPanel->setBounds(340, 20, 920, 680);
  rightPanel->setBackgroundColor(theming::Color(0.18f, 0.18f, 0.2f, 1.0f));
  rightPanel->setBorderColor(theming::Color(0.3f, 0.3f, 0.35f, 1.0f));
  rightPanel->setBorderWidth(2);
  rightPanel->setTitle("Items List");
  dispatcher.registerComponent(rightPanel.get());

  // ListBox
  listBoxPtr->setRenderer(renderer.get());
  listBoxPtr->setBounds(360, 80, 880, 600);
  listBoxPtr->setSelectionCallback(
    [](int index, const std::string& item) { std::cout << "Selected item " << index << ": " << item << std::endl; });
  // Add some default items
  listBoxPtr->addItem("Welcome to Prong UI Framework!");
  listBoxPtr->addItem("Click 'Add Item' to add more items");
  listBoxPtr->addItem("Click items to select them");
  listBoxPtr->addItem("Type in the text field to customize new items");
  dispatcher.registerComponent(listBoxPtr.get());

  // Store all components in vector for lifecycle management
  components.push_back(std::move(leftPanel));
  components.push_back(std::move(textInput));
  components.push_back(std::move(addButton));
  components.push_back(std::move(clearButton));
  components.push_back(std::move(infoButton));
  components.push_back(std::move(exitButton));
  components.push_back(std::move(rightPanel));
  components.push_back(std::move(listBoxPtr));

  std::cout << "\n=== Prong UI Framework Demo ===" << std::endl;
  std::cout << "Controls:" << std::endl;
  std::cout << "  - Type in the text field and click 'Add Item'" << std::endl;
  std::cout << "  - Click items in the list to select them" << std::endl;
  std::cout << "  - Click 'Show Info' to see framework information" << std::endl;
  std::cout << "  - ESC or 'Exit Application' to close" << std::endl;
  std::cout << "===============================\n" << std::endl;

  // === Main Loop ===
  double lastTime = glfwGetTime();
  while (!windowAdapter->shouldClose()) {
    // Calculate delta time
    double currentTime = glfwGetTime();
    double deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    // Poll events (this will trigger our callbacks)
    glfwPollEvents();

    // Check for ESC key
    if (windowAdapter->isKeyPressed(GLFW_KEY_ESCAPE)) {
      glfwSetWindowShouldClose(glfwWindow, GLFW_TRUE);
    }

    // Render frame
    if (renderer->beginFrame()) {
      // Clear screen with dark background
      renderer->clear(0.08f, 0.08f, 0.1f, 1.0f);

      // Update and render all UI components
      for (auto& component : components) {
        component->update(deltaTime);
      }

      for (auto& component : components) {
        component->render();
      }

      // Draw title with TrueType font
      // Note: fontRenderer sets up its own matrices internally
      if (fontRenderer) {
        // Render title text at bottom center (below the panels)
        fontRenderer->renderText("Prong UI Framework - TrueType Demo", 440, 715, 1.0f, 1.0f, 1.0f, 1.0f);

        // Render FPS counter at top right corner
        std::string fpsText = "FPS: " + std::to_string(static_cast<int>(std::round(1.0 / deltaTime)));
        fontRenderer->renderText(fpsText, 1180, 18, 0.5f, 1.0f, 0.5f, 1.0f);

        // Render font name at top left, above the left panel
        fontRenderer->renderText("Font: Monofur Nerd Font", 440, 18, 0.7f, 0.7f, 0.9f, 1.0f);
      }

      renderer->endFrame();
    }

    // Present frame
    renderer->present();
    glfwSwapBuffers(glfwWindow);
  }

  // === Cleanup ===
  std::cout << "\nShutting down..." << std::endl;

  // Unregister and destroy all components
  for (auto& component : components) {
    dispatcher.unregisterComponent(component.get());
  }
  components.clear();

  // Cleanup renderer and window
  renderer.reset();
  windowAdapter.reset();

  glfwDestroyWindow(glfwWindow);
  glfwTerminate();

  std::cout << "Thanks for trying Prong UI Framework!" << std::endl;
  return 0;
}

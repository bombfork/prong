/**
 * @file main.cpp
 * @brief ListBox example - Scrollable list with selection
 *
 * Demonstrates:
 * - Using ListBox component for item selection
 * - Adding and removing items dynamically
 * - Handling selection callbacks
 * - Scrolling through long lists
 * - Getting selected item and index
 */

#include "../../adapters/glfw_window_adapter.h"
#include "../../adapters/simple_opengl_renderer.h"
#include <GLFW/glfw3.h>
#include <bombfork/prong/components/button.h>
#include <bombfork/prong/components/list_box.h>
#include <bombfork/prong/components/panel.h>
#include <bombfork/prong/core/component_builder.h>
#include <bombfork/prong/core/scene.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

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

  GLFWwindow* glfwWindow = glfwCreateWindow(900, 700, "08 - List Box", nullptr, nullptr);
  if (!glfwWindow) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(glfwWindow);
  glfwSwapInterval(1);

  auto windowAdapter = std::make_unique<GLFWWindowAdapter>(glfwWindow);
  auto renderer = std::make_unique<SimpleOpenGLRenderer>();

  if (!renderer->initialize(900, 700)) {
    std::cerr << "Failed to initialize renderer" << std::endl;
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
    return -1;
  }

  auto scene = std::make_unique<Scene>(windowAdapter.get(), renderer.get());

  // Create a container panel
  auto panel = create<Panel<>>().withSize(850, 650).withPosition(25, 25).build();

  // Create first ListBox with initial items
  std::vector<std::string> programmingLanguages = {
    "C++",  "Python", "JavaScript", "Java",    "C#",      "TypeScript", "Go",     "Rust", "Swift", "Kotlin",
    "Ruby", "PHP",    "Scala",      "Haskell", "Clojure", "Elixir",     "Erlang", "F#",   "OCaml", "Dart"};

  auto languageList = create<ListBox>()
                        .withItems(programmingLanguages)
                        .withSize(300, 400)
                        .withPosition(50, 50)
                        .withSelectionCallback([](int index, const std::string& item) {
                          std::cout << "Selected language: " << item << " (index " << index << ")" << std::endl;
                        })
                        .build();

  panel->addChild(std::move(languageList));

  // Create second ListBox - initially empty
  auto taskList = create<ListBox>()
                    .withSize(300, 400)
                    .withPosition(400, 50)
                    .withSelectionCallback([](int index, const std::string& item) {
                      std::cout << "Selected task: " << item << " (index " << index << ")" << std::endl;
                    })
                    .build();

  // Keep a raw pointer for later manipulation
  ListBox* taskListPtr = taskList.get();

  panel->addChild(std::move(taskList));

  // Counter for generating task names
  int taskCounter = 1;

  // Create "Add Task" button
  auto addTaskButton = create<Button>("Add Task")
                         .withSize(140, 50)
                         .withPosition(400, 480)
                         .withClickCallback([taskListPtr, &taskCounter]() {
                           std::string newTask = "Task " + std::to_string(taskCounter++);
                           taskListPtr->addItem(newTask);
                           std::cout << "Added: " << newTask << std::endl;
                         })
                         .build();

  panel->addChild(std::move(addTaskButton));

  // Create "Get Selected" button (moved up since removeItem doesn't exist)
  auto getSelectedButton = create<Button>("Get Selected")
                             .withSize(140, 50)
                             .withPosition(560, 480)
                             .withClickCallback([taskListPtr]() {
                               int index = taskListPtr->getSelectedIndex();
                               if (index >= 0) {
                                 const auto& items = taskListPtr->getItems();
                                 std::cout << "Currently selected: " << items[index] << " at index " << index
                                           << std::endl;
                               } else {
                                 std::cout << "No item currently selected" << std::endl;
                               }
                             })
                             .build();

  panel->addChild(std::move(getSelectedButton));

  // Create "Clear All" button
  auto clearButton = create<Button>("Clear All")
                       .withSize(140, 50)
                       .withPosition(400, 550)
                       .withClickCallback([taskListPtr]() {
                         taskListPtr->clearItems();
                         std::cout << "Cleared all tasks" << std::endl;
                       })
                       .build();

  panel->addChild(std::move(clearButton));

  scene->addChild(std::move(panel));

  scene->attach();

  std::cout << "ListBox Example" << std::endl;
  std::cout << "===============" << std::endl;
  std::cout << "Left list: Programming languages (static)" << std::endl;
  std::cout << "  - Click items to select" << std::endl;
  std::cout << "  - Scroll if needed" << std::endl;
  std::cout << "\nRight list: Tasks (dynamic)" << std::endl;
  std::cout << "  - Click 'Add Task' to add items" << std::endl;
  std::cout << "  - Click 'Get Selected' to query current selection" << std::endl;
  std::cout << "  - Click 'Clear All' to remove all items" << std::endl;
  std::cout << "\nWatch the console for selection events!" << std::endl;

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

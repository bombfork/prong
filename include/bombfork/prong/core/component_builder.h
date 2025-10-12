#pragma once

#include <bombfork/prong/core/component.h>

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace bombfork::prong {

// Forward declarations for type checking
class Button;
class TextInput;
class ListBox;

/**
 * @brief Builder pattern for creating UI components with a fluent interface
 *
 * ComponentBuilder provides a clean, fluent API for constructing components without
 * needing to pass renderers or manually set properties. Components inherit their
 * renderer from their parent automatically.
 *
 * Usage example:
 * @code
 *   auto button = create<Button>("Click Me")
 *       .withSize(120, 40)
 *       .withPosition(10, 10)
 *       .withClickCallback([](){ std::cout << "Clicked!\n"; })
 *       .build();
 * @endcode
 *
 * @tparam ComponentT The type of component to build (must derive from Component)
 */
template <typename ComponentT>
class ComponentBuilder {
  static_assert(std::is_base_of_v<Component, ComponentT>, "ComponentT must be derived from bombfork::prong::Component");

private:
  std::unique_ptr<ComponentT> component;

public:
  /**
   * @brief Default constructor - creates component with default constructor
   */
  ComponentBuilder() : component(std::make_unique<ComponentT>()) {}

  /**
   * @brief Constructor with arguments - forwards arguments to component constructor
   * @tparam Args Argument types for component constructor
   * @param args Arguments to forward to component constructor
   */
  template <typename... Args>
  explicit ComponentBuilder(Args&&... args) : component(std::make_unique<ComponentT>(std::forward<Args>(args)...)) {}

  // === Basic Properties ===

  /**
   * @brief Set component position
   * @param x X coordinate
   * @param y Y coordinate
   * @return Reference to builder for chaining
   */
  ComponentBuilder& withPosition(int x, int y) {
    component->setPosition(x, y);
    return *this;
  }

  /**
   * @brief Set component size
   * @param width Width in pixels
   * @param height Height in pixels
   * @return Reference to builder for chaining
   */
  ComponentBuilder& withSize(int width, int height) {
    component->setSize(width, height);
    return *this;
  }

  /**
   * @brief Set component bounds (position and size)
   * @param x X coordinate
   * @param y Y coordinate
   * @param width Width in pixels
   * @param height Height in pixels
   * @return Reference to builder for chaining
   */
  ComponentBuilder& withBounds(int x, int y, int width, int height) {
    component->setBounds(x, y, width, height);
    return *this;
  }

  /**
   * @brief Set component visibility
   * @param visible True to show, false to hide
   * @return Reference to builder for chaining
   */
  ComponentBuilder& withVisible(bool visible) {
    component->setVisible(visible);
    return *this;
  }

  /**
   * @brief Set component enabled state
   * @param enabled True to enable, false to disable
   * @return Reference to builder for chaining
   */
  ComponentBuilder& withEnabled(bool enabled) {
    component->setEnabled(enabled);
    return *this;
  }

  /**
   * @brief Set debug name for the component
   * @param name Debug name
   * @return Reference to builder for chaining
   */
  ComponentBuilder& withDebugName(const std::string& name) {
    component->setDebugName(name);
    return *this;
  }

  /**
   * @brief Set focus callback
   * @param callback Callback function that receives focus state
   * @return Reference to builder for chaining
   */
  ComponentBuilder& withFocusCallback(typename Component::FocusCallback callback) {
    component->setFocusCallback(std::move(callback));
    return *this;
  }

  // === Component-Specific Callbacks ===

  /**
   * @brief Set click callback (Button only)
   * @param callback Callback function to invoke on click
   * @return Reference to builder for chaining
   * @note Only enabled when ComponentT is Button or derives from Button
   */
  template <typename T = ComponentT>
  std::enable_if_t<std::is_base_of_v<Button, T>, ComponentBuilder&> withClickCallback(std::function<void()> callback) {
    component->setClickCallback(std::move(callback));
    return *this;
  }

  /**
   * @brief Set text changed callback (TextInput only)
   * @param callback Callback function that receives the new text
   * @return Reference to builder for chaining
   * @note Only enabled when ComponentT is TextInput or derives from TextInput
   */
  template <typename T = ComponentT>
  std::enable_if_t<std::is_base_of_v<TextInput, T>, ComponentBuilder&>
  withTextChangedCallback(std::function<void(const std::string&)> callback) {
    component->setOnTextChanged(std::move(callback));
    return *this;
  }

  /**
   * @brief Set selection callback (ListBox only)
   * @param callback Callback function that receives index and item
   * @return Reference to builder for chaining
   * @note Only enabled when ComponentT is ListBox or derives from ListBox
   */
  template <typename T = ComponentT>
  std::enable_if_t<std::is_base_of_v<ListBox, T>, ComponentBuilder&>
  withSelectionCallback(std::function<void(int, const std::string&)> callback) {
    component->setSelectionCallback(std::move(callback));
    return *this;
  }

  // === Component-Specific Properties ===

  /**
   * @brief Set button text (Button only)
   * @param text Button text
   * @return Reference to builder for chaining
   * @note Only enabled when ComponentT is Button or derives from Button
   */
  template <typename T = ComponentT>
  std::enable_if_t<std::is_base_of_v<Button, T>, ComponentBuilder&> withText(const std::string& text) {
    component->setText(text);
    return *this;
  }

  /**
   * @brief Set placeholder text (TextInput only)
   * @param placeholder Placeholder text
   * @return Reference to builder for chaining
   * @note Only enabled when ComponentT is TextInput or derives from TextInput
   */
  template <typename T = ComponentT>
  std::enable_if_t<std::is_base_of_v<TextInput, T>, ComponentBuilder&> withPlaceholder(const std::string& placeholder) {
    component->setPlaceholder(placeholder);
    return *this;
  }

  /**
   * @brief Set list items (ListBox only)
   * @param items Vector of items to display
   * @return Reference to builder for chaining
   * @note Only enabled when ComponentT is ListBox or derives from ListBox
   */
  template <typename T = ComponentT>
  std::enable_if_t<std::is_base_of_v<ListBox, T>, ComponentBuilder&> withItems(const std::vector<std::string>& items) {
    component->setItems(items);
    return *this;
  }

  // === Child Management ===

  /**
   * @brief Add a single child component
   * @param child Unique pointer to child component
   * @return Reference to builder for chaining
   */
  ComponentBuilder& withChild(std::unique_ptr<Component> child) {
    component->addChild(std::move(child));
    return *this;
  }

  /**
   * @brief Add multiple child components
   * @tparam Args Types of child component unique pointers
   * @param children Child components to add
   * @return Reference to builder for chaining
   */
  template <typename... Args>
  ComponentBuilder& withChildren(Args&&... children) {
    (component->addChild(std::forward<Args>(children)), ...);
    return *this;
  }

  // === Layout Manager Support ===

  /**
   * @brief Set layout manager for this component
   * @tparam LayoutT Layout manager type
   * @param layout Shared pointer to layout manager
   * @return Reference to builder for chaining
   */
  template <typename LayoutT>
  ComponentBuilder& withLayout(std::shared_ptr<LayoutT> layout) {
    component->template setLayout<LayoutT>(layout);
    return *this;
  }

  // === Building ===

  /**
   * @brief Build and return the component
   * @return Unique pointer to the constructed component
   */
  std::unique_ptr<ComponentT> build() { return std::move(component); }

  /**
   * @brief Direct access to component for advanced configuration
   * @return Raw pointer to component (builder still owns it until build())
   */
  ComponentT* operator->() { return component.get(); }

  /**
   * @brief Direct access to component for advanced configuration
   * @return Reference to component (builder still owns it until build())
   */
  ComponentT& operator*() { return *component; }
};

// === Factory Functions ===

/**
 * @brief Create a component builder with default constructor
 * @tparam ComponentT Type of component to create
 * @return ComponentBuilder for the specified component type
 *
 * Usage:
 * @code
 *   auto button = create<Button>().withText("OK").build();
 * @endcode
 */
template <typename ComponentT>
ComponentBuilder<ComponentT> create() {
  return ComponentBuilder<ComponentT>();
}

/**
 * @brief Create a component builder with constructor arguments
 * @tparam ComponentT Type of component to create
 * @tparam Args Types of arguments to forward to component constructor
 * @param args Arguments to forward to component constructor
 * @return ComponentBuilder for the specified component type
 *
 * Usage:
 * @code
 *   auto button = create<Button>("Click Me").withSize(100, 40).build();
 * @endcode
 */
template <typename ComponentT, typename... Args>
ComponentBuilder<ComponentT> create(Args&&... args) {
  return ComponentBuilder<ComponentT>(std::forward<Args>(args)...);
}

} // namespace bombfork::prong

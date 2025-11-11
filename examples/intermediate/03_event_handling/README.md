# Event Handling Example

This example demonstrates advanced event handling patterns in Prong, including custom event handlers, event propagation, focus management, and keyboard shortcuts.

## What This Example Shows

- **Custom event handling** by overriding `handleEventSelf()`
- **Event propagation** through the component tree
- **Stopping propagation** when events are handled
- **Focus management** with visual feedback
- **Mouse interactions**: clicks, drag tracking, hover states
- **Keyboard shortcuts** for focused components
- **Multiple event types**: mouse, keyboard, scroll, char input

## Event System Overview

### Event Flow

Events in Prong follow a hierarchical propagation model:

1. **Scene receives event** from the window system (GLFW in this case)
2. **Scene calls handleEvent()** on root components
3. **Parent propagates to children** in reverse order (topmost rendered first)
4. **Each component**:
   - Checks if event is within its bounds (`containsEvent()`)
   - Converts coordinates to local space
   - Calls `handleEventSelf()` for component-specific handling
   - Propagates to children if event not handled
5. **Event stops** when a component returns `true` from `handleEventSelf()`

### Event Types

The `Event` struct supports these types:

```cpp
enum class Type {
  MOUSE_PRESS,   // Mouse button pressed
  MOUSE_RELEASE, // Mouse button released
  MOUSE_MOVE,    // Mouse cursor moved
  MOUSE_SCROLL,  // Mouse wheel scrolled
  KEY_PRESS,     // Keyboard key pressed
  KEY_RELEASE,   // Keyboard key released
  CHAR_INPUT     // Character input (for text)
};
```

## Custom Event Handler Pattern

To handle events in a custom component, override `handleEventSelf()`:

```cpp
class InteractivePanel : public Component {
protected:
  bool handleEventSelf(const Event& event) override {
    switch (event.type) {
      case Event::Type::MOUSE_PRESS:
        // Handle mouse press
        if (event.button == 0) { // Left button
          // Do something
          return true; // Stop propagation
        }
        break;

      case Event::Type::KEY_PRESS:
        // Handle keyboard input
        if (focusState == FocusState::FOCUSED) {
          // Handle key
          return true;
        }
        break;

      // ... other event types
    }

    return false; // Event not handled, continue propagation
  }
};
```

### Return Value Semantics

The return value from `handleEventSelf()` controls propagation:

- **`true`**: Event was handled, stop propagation (siblings and parent won't see it)
- **`false`**: Event not handled, continue propagation to parent/siblings

**Example**: A button should return `true` on click to prevent the click from also triggering the panel behind it.

## Coordinates in Events

Events contain coordinates in **local space** relative to the component:

```cpp
case Event::Type::MOUSE_PRESS:
  // event.localX and event.localY are relative to this component
  // (0, 0) is the top-left corner of this component
  std::cout << "Clicked at local position: "
            << event.localX << ", " << event.localY << std::endl;
  break;
```

The framework automatically converts global screen coordinates to local coordinates during propagation, so you never need to do this manually.

## Focus Management

Components can request and receive focus:

```cpp
// Request focus (typically on click)
requestFocus();

// Check focus state
if (focusState == FocusState::FOCUSED) {
  // Handle keyboard input
}

// Focus states
enum class FocusState {
  NONE,     // Not focused, not focusable
  FOCUSABLE, // Can receive focus but doesn't have it
  FOCUSED   // Currently has focus
};
```

### Focus Behavior

- Only **one component** can have focus at a time
- **Scene manages focus** globally
- **Clicking a component** typically requests focus
- **Keyboard events** only go to focused components
- **Visual feedback** (like a border) shows focus state

## Hover States

The `Component` base class tracks hover state automatically:

```cpp
void render() override {
  if (isCurrentlyHovered) {
    // Mouse is over this component
    renderer->drawRect(gx, gy, w, h, hoverR, hoverG, hoverB, 1.0f);
  } else {
    // Normal rendering
    renderer->drawRect(gx, gy, w, h, r, g, b, 1.0f);
  }
}
```

Hover state is updated automatically during event propagation - you just need to check the flag.

## Interactive Example Features

### Click Tracking

```cpp
case Event::Type::MOUSE_PRESS:
  if (event.button == 0) {
    clickCount++;
    return true; // Stop propagation
  }
```

### Drag Tracking

```cpp
case Event::Type::MOUSE_PRESS:
  isDragging = true;
  dragStartX = event.localX;
  dragStartY = event.localY;
  return true;

case Event::Type::MOUSE_MOVE:
  if (isDragging) {
    int dx = event.localX - dragStartX;
    int dy = event.localY - dragStartY;
    // Track drag delta
  }
  return false; // Don't stop propagation for move

case Event::Type::MOUSE_RELEASE:
  isDragging = false;
  return true;
```

### Keyboard Shortcuts

```cpp
case Event::Type::KEY_PRESS:
  if (focusState == FocusState::FOCUSED) {
    if (event.key == GLFW_KEY_SPACE) {
      // Handle spacebar
      return true;
    }
    if (event.key == GLFW_KEY_R) {
      // Handle 'R' key
      return true;
    }
  }
  break;
```

### Scroll Handling

```cpp
case Event::Type::MOUSE_SCROLL:
  // event.scrollX and event.scrollY contain scroll deltas
  lastEvent = "Scroll: x=" + std::to_string(event.scrollX) +
              " y=" + std::to_string(event.scrollY);
  return true;
```

## Building and Running

```bash
# Build all examples
mise build-examples

# Run this example
./build/examples/intermediate/03_event_handling/03_event_handling
```

Try interacting with the panels to see different event handling patterns!

## Common Patterns

### Button-like Component

```cpp
bool handleEventSelf(const Event& event) override {
  if (event.type == Event::Type::MOUSE_PRESS && event.button == 0) {
    if (onClick) onClick();
    return true; // Stop propagation
  }
  return false;
}
```

### Draggable Component

```cpp
bool handleEventSelf(const Event& event) override {
  switch (event.type) {
    case Event::Type::MOUSE_PRESS:
      isDragging = true;
      return true;

    case Event::Type::MOUSE_RELEASE:
      isDragging = false;
      return true;

    case Event::Type::MOUSE_MOVE:
      if (isDragging) {
        // Update position based on drag
      }
      return isDragging; // Only stop if dragging
  }
  return false;
}
```

### Keyboard-Controlled Component

```cpp
bool handleEventSelf(const Event& event) override {
  if (event.type == Event::Type::KEY_PRESS &&
      focusState == FocusState::FOCUSED) {

    // Arrow key navigation
    if (event.key == GLFW_KEY_UP) { /* move up */ return true; }
    if (event.key == GLFW_KEY_DOWN) { /* move down */ return true; }

    // Action keys
    if (event.key == GLFW_KEY_ENTER) { /* activate */ return true; }
    if (event.key == GLFW_KEY_ESCAPE) { /* cancel */ return true; }
  }
  return false;
}
```

## Best Practices

1. **Return true when handling**: If you process an event, return `true` to stop propagation
2. **Check focus for keyboard**: Only handle keyboard events when focused
3. **Use local coordinates**: Events already provide local coordinates - don't convert
4. **Provide visual feedback**: Show hover, focus, and active states
5. **Handle related events together**: Press and release, drag start and end
6. **Don't stop everything**: Mouse move often shouldn't stop propagation
7. **Request focus on interaction**: Call `requestFocus()` when clicked

## Extending This Example

Try these modifications:

1. **Add double-click detection**: Track time between clicks
2. **Implement right-click menu**: Handle `button == 1` (right button)
3. **Add keyboard navigation**: Tab to move focus between panels
4. **Implement tooltips**: Show on hover after delay
5. **Add gesture detection**: Swipe, pinch, etc.
6. **Create modal dialogs**: Block events to components behind

## Related Examples

- **basic/01_hello_button** - Simple click handling
- **basic/04_text_input** - Text input and keyboard handling
- **intermediate/04_dynamic_layout** - Dynamic component interaction
- **advanced/01_custom_component** - Building complex custom components

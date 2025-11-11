# 01 - Hello Button

The absolute minimal Prong UI application - a single button in a window.

## What This Demonstrates

- Basic GLFW window creation and OpenGL context setup
- Prong renderer initialization
- Creating a Scene (root container for UI)
- Creating a Button with ComponentBuilder
- Attaching click callbacks
- The main event/render loop

## Key Concepts

**Scene**: The root container that manages all UI components. Every Prong application needs at least one Scene. It handles event propagation from the window system and coordinates rendering.

**ComponentBuilder**: The fluent interface for creating UI components. Use `create<ComponentType>()` to start building, chain configuration methods like `.withSize()` and `.withPosition()`, then call `.build()` to get the component.

**Event Loop**: The standard pattern of polling events, updating components, rendering, and swapping buffers.

## Building

```bash
cd /home/atom/projects/bombfork/prong
mise build-examples
./build/examples/basic/01_hello_button/01_hello_button
```

## Code Walkthrough

1. **Window Setup** (lines 22-43): Standard GLFW initialization with OpenGL 3.3 compatibility profile
2. **Adapters** (lines 46-54): Create window and renderer adapters that provide the platform-agnostic interface
3. **Scene Creation** (line 57): The Scene is the root of the component tree
4. **Button Creation** (lines 62-66): Use ComponentBuilder to create a button centered on screen with a click callback
5. **Main Loop** (lines 75-91): Standard game loop - poll events, update, render, swap

## Try This

Experiment with these modifications to learn more:

1. **Change button position**: Modify `.withPosition(220, 210)` to move the button
2. **Add more buttons**: Create additional buttons with different positions and callbacks
3. **Change button size**: Try `.withSize(300, 80)` for a larger button
4. **Custom callback**: Make the button do something more interesting than printing
5. **Change background**: Modify the `renderer->clear()` RGB values (line 85)

**Tip**: Position is in pixels from the top-left corner (0,0). To center a button, use:

```text
x = (windowWidth - buttonWidth) / 2
y = (windowHeight - buttonHeight) / 2
```

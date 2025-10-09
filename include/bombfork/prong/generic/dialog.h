#pragma once

#include "../layout/flow_layout.h"
#include "../layout/stack_layout.h"
#include "../theming/advanced_theme.h"
#include <bombfork/prong/components/button.h>
#include <bombfork/prong/components/panel.h>
#include <bombfork/prong/core/component.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace bombfork::prong {

/**
 * @brief Professional dialog component for modal and non-modal windows
 *
 * Provides desktop-quality dialog functionality with:
 * - Modal and non-modal operation modes
 * - Proper focus management and tab ordering
 * - Keyboard navigation (Tab, Enter, Escape)
 * - Flexible content area with layout management
 * - Standard button configurations (OK, Cancel, etc.)
 * - Professional theming with shadows and animations
 * - Window centering and positioning
 * - Click-outside-to-close for non-modal dialogs
 * - Resizable and non-resizable modes
 */
class Dialog : public Component {
public:
  enum class DialogType {
    MODAL,      // Blocks interaction with parent window
    NON_MODAL,  // Allows interaction with parent window
    TOOL_WINDOW // Floating tool window
  };

  enum class StandardButtons {
    NONE = 0,
    OK = 1,
    CANCEL = 2,
    YES = 4,
    NO = 8,
    APPLY = 16,
    CLOSE = 32,
    HELP = 64,
    OK_CANCEL = OK | CANCEL,
    YES_NO = YES | NO,
    YES_NO_CANCEL = YES | NO | CANCEL
  };

  enum class DialogResult { NONE, OK, CANCEL, YES, NO, APPLY, CLOSE, HELP };

  using DialogCallback = std::function<void(DialogResult result)>;
  using ButtonCallback = std::function<void(DialogResult result)>;
  using ValidateCallback = std::function<bool()>; // Return false to prevent dialog close

private:
  static constexpr int DEFAULT_MIN_WIDTH = 300;
  static constexpr int DEFAULT_MIN_HEIGHT = 150;
  static constexpr int TITLE_BAR_HEIGHT = 30;
  static constexpr int BUTTON_AREA_HEIGHT = 50;
  static constexpr int CONTENT_PADDING = 10;
  static constexpr int BUTTON_SPACING = 8;
  static constexpr int SHADOW_SIZE = 10;

  struct DialogState {
    DialogType type = DialogType::MODAL;
    bool visible = false;
    bool resizable = false;
    bool showTitleBar = true;
    bool centerOnParent = true;
    std::string title;
    int minWidth = DEFAULT_MIN_WIDTH;
    int minHeight = DEFAULT_MIN_HEIGHT;
    DialogResult result = DialogResult::NONE;
    bool dragging = false;
    int dragStartX = 0, dragStartY = 0;
    int dragOffsetX = 0, dragOffsetY = 0;
  };

  struct DialogTheme {
    // Background and borders
    bombfork::prong::theming::Color backgroundColor;
    bombfork::prong::theming::Color borderColor;
    bombfork::prong::theming::Color titleBarColor;
    bombfork::prong::theming::Color shadowColor;

    // Text colors
    bombfork::prong::theming::Color titleTextColor;
    bombfork::prong::theming::Color contentTextColor;

    // Modal overlay
    bombfork::prong::theming::Color modalOverlayColor;

    // Visual properties
    float borderWidth = 2.0f;
    float cornerRadius = 8.0f;
    float shadowOpacity = 0.3f;
    int shadowOffset = 3;

    DialogTheme() {
      // Professional desktop theme defaults
      backgroundColor = bombfork::prong::theming::Color(0.25f, 0.25f, 0.25f, 1.0f);
      borderColor = bombfork::prong::theming::Color(0.4f, 0.4f, 0.4f, 1.0f);
      titleBarColor = bombfork::prong::theming::Color(0.2f, 0.2f, 0.2f, 1.0f);
      shadowColor = bombfork::prong::theming::Color(0.0f, 0.0f, 0.0f, 0.5f);
      titleTextColor = bombfork::prong::theming::Color(1.0f, 1.0f, 1.0f, 1.0f);
      contentTextColor = bombfork::prong::theming::Color(0.9f, 0.9f, 0.9f, 1.0f);
      modalOverlayColor = bombfork::prong::theming::Color(0.0f, 0.0f, 0.0f, 0.4f);
    }
  };

  DialogState state;
  DialogTheme theme;

  // Layout components
  std::unique_ptr<bombfork::prong::layout::StackLayout> mainLayout;
  std::unique_ptr<bombfork::prong::Panel> titleBarPanel;
  std::unique_ptr<bombfork::prong::Panel> contentPanel;
  std::unique_ptr<bombfork::prong::Panel> buttonPanel;
  std::unique_ptr<bombfork::prong::layout::FlowLayout> buttonLayout;

  // Standard buttons
  StandardButtons standardButtons = StandardButtons::NONE;
  std::vector<std::unique_ptr<bombfork::prong::Button>> buttons;

  // Callbacks
  DialogCallback dialogCallback;
  ButtonCallback buttonCallback;
  ValidateCallback validateCallback;

  // Parent window reference for centering
  Component* parentWindow = nullptr;
  int parentWindowWidth = 0, parentWindowHeight = 0;

public:
  explicit Dialog(bombfork::prong::rendering::IRenderer* renderer = nullptr);
  ~Dialog() override = default;

  // === Configuration ===

  /**
   * @brief Set dialog type (modal/non-modal)
   */
  void setDialogType(DialogType type);

  /**
   * @brief Get dialog type
   */
  DialogType getDialogType() const { return state.type; }

  /**
   * @brief Set dialog title
   */
  void setTitle(const std::string& title);

  /**
   * @brief Get dialog title
   */
  const std::string& getTitle() const { return state.title; }

  /**
   * @brief Set resizable flag
   */
  void setResizable(bool resizable);

  /**
   * @brief Check if dialog is resizable
   */
  bool isResizable() const { return state.resizable; }

  /**
   * @brief Set minimum size
   */
  void setMinimumSize(int width, int height);

  /**
   * @brief Get minimum size
   */
  void getMinimumSize(int& width, int& height) const;

  /**
   * @brief Show/hide title bar
   */
  void setShowTitleBar(bool show);

  /**
   * @brief Check if title bar is shown
   */
  bool getShowTitleBar() const { return state.showTitleBar; }

  // === Content Management ===

  /**
   * @brief Set content component
   */
  void setContent(std::unique_ptr<Component> content);

  /**
   * @brief Get content panel for adding components
   */
  bombfork::prong::Panel* getContentPanel() const { return contentPanel.get(); }

  /**
   * @brief Add content component to the content panel
   */
  void addContentComponent(std::unique_ptr<Component> component);

  // === Button Management ===

  /**
   * @brief Set standard buttons
   */
  void setStandardButtons(StandardButtons buttons);

  /**
   * @brief Get standard buttons
   */
  StandardButtons getStandardButtons() const { return standardButtons; }

  /**
   * @brief Add custom button
   */
  bombfork::prong::Button* addButton(const std::string& text, DialogResult result = DialogResult::NONE);

  /**
   * @brief Remove all buttons
   */
  void clearButtons();

  /**
   * @brief Set default button (activated by Enter key)
   */
  void setDefaultButton(bombfork::prong::Button* button);

  // === Dialog Management ===

  /**
   * @brief Show dialog
   */
  void show();

  /**
   * @brief Hide dialog
   */
  void hide();

  /**
   * @brief Show modal dialog and return result
   */
  DialogResult showModal();

  /**
   * @brief Check if dialog is visible
   */
  bool isVisible() const { return state.visible; }

  /**
   * @brief Get dialog result
   */
  DialogResult getResult() const { return state.result; }

  /**
   * @brief Accept dialog with result
   */
  void accept(DialogResult result = DialogResult::OK);

  /**
   * @brief Reject dialog
   */
  void reject();

  // === Positioning ===

  /**
   * @brief Center dialog on parent window
   */
  void centerOnParent();

  /**
   * @brief Set parent window for centering
   */
  void setParentWindow(Component* parent, int parentWidth, int parentHeight);

  /**
   * @brief Position dialog at specific location
   */
  void setPosition(int x, int y);

  // === Callbacks ===

  /**
   * @brief Set dialog callback (called when dialog closes)
   */
  void setDialogCallback(DialogCallback callback);

  /**
   * @brief Set button callback (called when button is pressed)
   */
  void setButtonCallback(ButtonCallback callback);

  /**
   * @brief Set validation callback (called before closing)
   */
  void setValidateCallback(ValidateCallback callback);

  // === Theming ===

  /**
   * @brief Apply theme from AdvancedTheme system
   */
  void applyTheme(const bombfork::prong::theming::AdvancedTheme& theme);

  /**
   * @brief Set custom theme
   */
  void setDialogTheme(const DialogTheme& customTheme);

  /**
   * @brief Get current theme
   */
  const DialogTheme& getDialogTheme() const { return theme; }

  // === Component Overrides ===

  void render() override;
  bool handleClick(int localX, int localY) override;
  bool handleMousePress(int localX, int localY, int button) override;
  bool handleMouseRelease(int localX, int localY, int button) override;
  bool handleMouseMove(int localX, int localY) override;
  bool handleKey(int key, int action, int mods) override;
  void setBounds(int x, int y, int width, int height) override;

private:
  /**
   * @brief Initialize dialog layout
   */
  void initializeLayout();

  /**
   * @brief Create standard buttons
   */
  void createStandardButtons();

  /**
   * @brief Update layout after changes
   */
  void updateLayout();

  /**
   * @brief Render modal overlay
   */
  void renderModalOverlay();

  /**
   * @brief Render dialog shadow
   */
  void renderShadow();

  /**
   * @brief Render dialog frame
   */
  void renderFrame();

  /**
   * @brief Render title bar
   */
  void renderTitleBar();

  /**
   * @brief Handle button press
   */
  void handleButtonPress(DialogResult result);

  /**
   * @brief Check if point is in title bar
   */
  bool isPointInTitleBar(int localX, int localY) const;

  /**
   * @brief Validate dialog close
   */
  bool validateClose();

  /**
   * @brief Process keyboard shortcuts
   */
  bool processKeyboardShortcut(int key, int mods);

  /**
   * @brief Focus next/previous control
   */
  void focusNextControl(bool forward = true);

  /**
   * @brief Get button for result
   */
  bombfork::prong::Button* getButtonForResult(DialogResult result) const;

  /**
   * @brief Calculate content area bounds
   */
  layout::Rect getContentBounds() const;

  /**
   * @brief Calculate title bar bounds
   */
  layout::Rect getTitleBarBounds() const;

  /**
   * @brief Calculate button area bounds
   */
  layout::Rect getButtonAreaBounds() const;
};

} // namespace bombfork::prong
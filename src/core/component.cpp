#include <bombfork/prong/core/component.h>
#include <bombfork/prong/layout/layout_manager.h>

namespace bombfork::prong {

namespace {
/**
 * @brief Adapter class that wraps a Component to provide layout::Component interface
 */
class ComponentAdapter : public layout::Component {
public:
  explicit ComponentAdapter(bombfork::prong::Component* component) : component_(component) {}

  layout::Dimensions measure() const override {
    if (component_) {
      auto preferred = component_->getPreferredSize();
      return {preferred.width, preferred.height};
    }
    return {0, 0};
  }

  layout::Dimensions measureLayout() const override { return measure(); }

  void setBounds(const layout::Rect& bounds) override {
    if (component_) {
      component_->setBounds(static_cast<int>(bounds.x), static_cast<int>(bounds.y), static_cast<int>(bounds.width),
                            static_cast<int>(bounds.height));
    }
  }

private:
  bombfork::prong::Component* component_;
};
} // anonymous namespace

void Component::performLayout() {
  if (!layoutInvalid || !layoutFunc) {
    return;
  }

  // Mark layout as valid first to avoid infinite recursion
  layoutInvalid = false;

  // Create adapter wrappers for children to match layout::Component interface
  std::vector<std::shared_ptr<layout::Component>> adaptedChildren;
  adaptedChildren.reserve(children.size());

  for (auto& child : children) {
    if (child) {
      adaptedChildren.push_back(std::make_shared<ComponentAdapter>(child.get()));
    }
  }

  // Create available space from current component dimensions
  layout::Dimensions availableSpace{width, height};

  // Call the layout manager through the type-erased function
  layoutFunc(adaptedChildren, availableSpace);

  // Recursively perform layout on children
  for (auto& child : children) {
    if (child) {
      child->performLayout();
    }
  }
}

} // namespace bombfork::prong

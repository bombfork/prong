# 08 - List Box (Bonus)

Scrollable list component with selection support and dynamic item management.

## What This Demonstrates

- Using ListBox component for item lists
- Creating ListBox with initial items
- Handling selection callbacks
- Adding items dynamically
- Removing items by index
- Clearing all items
- Querying current selection
- Scrolling through long lists

## Key Concepts

**ListBox**: A scrollable list component that displays a vertical list of text items. Users can click to select items, scroll through long lists, and the component fires callbacks when selection changes.

**Initial Items**: Use `withItems(vector)` in the builder to populate the list at creation time.

**Selection Callback**: Register a callback with `withSelectionCallback()` to be notified when an item is selected. The callback receives both the index and the item text.

**Dynamic Manipulation**:

```cpp
listBox->addItem("New Item");           // Add to end
listBox->removeItem(index);             // Remove by index
listBox->clearItems();                  // Remove all
int idx = listBox->getSelectedIndex();  // Get selection (-1 if none)
const auto& items = listBox->getItems(); // Get all items
```

**Scrolling**: ListBox automatically provides scrolling when content exceeds the visible area. Users can use mouse wheel or click-and-drag to scroll.

## Building

```bash
cd /home/atom/projects/bombfork/prong
mise build-examples
./build/examples/basic/08_list_box/08_list_box
```

## Code Walkthrough

1. **Languages List** (lines 73-85): Create a ListBox pre-populated with 20 programming languages, demonstrating scrolling with long lists

2. **Tasks List** (lines 88-97): Create an initially empty ListBox that will be populated dynamically

3. **Raw Pointers** (lines 87, 96): Keep raw pointers to the ListBoxes so button callbacks can modify them

4. **Add Button** (lines 103-111): Adds new task items with incrementing numbers

5. **Remove Button** (lines 114-126): Removes the currently selected item, or shows message if nothing is selected

6. **Clear Button** (lines 129-136): Removes all items from the list

7. **Get Selected Button** (lines 139-151): Queries and displays the current selection

## Try This

1. **Select items**: Click on items in either list to select them
2. **Scroll**: Use mouse wheel to scroll through the long languages list
3. **Add tasks**: Click "Add Task" multiple times to populate the task list
4. **Remove tasks**: Select a task, then click "Remove Selected"
5. **Check selection**: Click "Get Selected" to see what's currently selected
6. **Clear**: Click "Clear All" to empty the task list

**Multi-selection Example** (not in basic example):

```cpp
// For multi-selection support, you would need to track multiple indices
// and modify the ListBox to support multiple selections
```

**Common Use Cases**:

- File browsers (list of files)
- Settings panels (list of options)
- Contact lists
- Playlist managers
- Log viewers
- Search results
- Inventory lists
- Message/email lists

**Integration with Other Components**:

```cpp
// Example: Master-detail pattern
auto listBox = create<ListBox>()
  .withItems({"Item 1", "Item 2", "Item 3"})
  .withSelectionCallback([detailPanel](int index, const std::string& item) {
    // Update detail panel to show details of selected item
    detailPanel->showDetails(item);
  })
  .build();
```

**Pro Tip**: Keep a raw pointer or shared_ptr to your ListBox if you need to manipulate it from callbacks. Remember that the Scene owns the component via unique_ptr, so the raw pointer remains valid as long as the component is in the scene.

**Note**: The current ListBox is single-selection only. For multi-selection support, you would need to extend the component or implement custom selection tracking.

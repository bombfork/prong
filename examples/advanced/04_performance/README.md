# Performance Optimization Example

Demonstrates performance characteristics with 100 components and optimization techniques built into Prong.

## Performance Features

### 1. Layout Caching
- Layouts only recalculate when invalidated
- Use `invalidateLayout()` only when necessary
- Batch multiple changes before invalidating

### 2. Coordinate Caching
- Global coordinates cached and invalidated automatically
- Multiple `getGlobalPosition()` calls in one frame are cheap
- Cache invalidates on position changes and cascades to children

### 3. CRTP Zero-Cost Abstraction
- No virtual function overhead for layouts
- Compile-time polymorphism
- Templates resolve at compile time

### 4. Efficient Event Propagation
- Hit testing before processing
- Events stop at handler (don't continue unnecessarily)
- Reverse iteration for Z-order

## Best Practices

1. **Batch operations**: Multiple adds/removes, then one invalidate
2. **Avoid unnecessary updates**: Check if values actually changed
3. **Use visibility**: Hide instead of remove if component will return
4. **Layout choice matters**: Pick simplest layout that meets needs
5. **Event handling**: Return `true` to stop propagation early

## Building

```bash
mise build-examples
./build/examples/advanced/04_performance/04_performance
```

Watch FPS counter in console to see performance.

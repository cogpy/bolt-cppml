# C++ E2E Test Patterns

## Table of Contents

1. Static Utility Functions
2. Singleton Classes
3. RAII / Resource Management
4. Thread-Spawning Classes
5. Callback/Handler Registration
6. Template Classes
7. Operational Transform / Collaboration
8. Network Buffers
9. Cross-Module Integration

## 1. Static Utility Functions

```cpp
BOLT_TEST(Suite, FunctionName) {
    auto result = Namespace::ClassName::staticMethod(input);
    BOLT_ASSERT_EQ(expected, result);
}
```

Test: valid input, empty input, edge cases (max values, special chars, unicode).

## 2. Singleton Classes

```cpp
BOLT_TEST(Suite, SingletonMethod) {
    auto& instance = ClassName::getInstance();
    instance.reset(); // clean state
    instance.doSomething();
    BOLT_ASSERT(instance.getState() == expected);
}
```

Always reset state at test start. Test getInstance() returns same reference.

## 3. RAII / Resource Management

```cpp
BOLT_TEST(Suite, AllocateDeallocate) {
    auto& mgr = MemoryManager::getInstance();
    mgr.forceReset();
    void* p = mgr.allocate(256);
    BOLT_ASSERT(p != nullptr);
    BOLT_ASSERT_EQ(256u, mgr.getCurrentUsage());
    mgr.deallocate(p);
    BOLT_ASSERT_EQ(0u, mgr.getCurrentUsage());
}
```

Verify: allocation succeeds, usage tracking, deallocation zeroes usage, double-free handling.

## 4. Thread-Spawning Classes

```cpp
BOLT_TEST(Suite, StartStop) {
    Manager mgr;
    auto id = mgr.create(program);
    mgr.start(id);
    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // settle
    mgr.stop(id);
    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // cleanup
    mgr.destroy(id);
}
```

Always add sleep after start/stop. Never assert `is_running` for short-lived tasks.

## 5. Callback/Handler Registration

```cpp
BOLT_TEST(Suite, RegisterHandler) {
    Handler handler;
    bool called = false;
    handler.registerCallback("event", [&called](auto& data) {
        called = true;
    });
    handler.trigger("event", testData);
    BOLT_ASSERT_TRUE(called);
}
```

## 6. Template Classes

```cpp
BOLT_TEST(Suite, TemplateInt) {
    DataProcessor<int>::sort(data);
    BOLT_ASSERT(std::is_sorted(data.begin(), data.end()));
}
BOLT_TEST(Suite, TemplateDouble) {
    auto avg = DataProcessor<double>::calculateAverage(data);
    BOLT_ASSERT(std::abs(avg - expected) < 0.001);
}
```

Test with at least int and double. Test empty containers.

## 7. Operational Transform / Collaboration

```cpp
BOLT_TEST(Suite, TransformInsertInsert) {
    DocumentOperation opA(INSERT, "userA", Position(0, 5), "text");
    DocumentOperation opB(INSERT, "userB", Position(0, 0), "other");
    auto transformed = OperationalTransform::transform(opA, opB);
    BOLT_ASSERT_NOT_NULL(transformed.get());
    // Apply opB then transformedA, verify both edits present
}
```

Test: insert-insert, insert-delete, delete-delete, same-position conflicts.

## 8. Network Buffers

```cpp
BOLT_TEST(Suite, AppendAndConsume) {
    NetworkBuffer buf;
    buf.append("data");
    BOLT_ASSERT_EQ(4u, buf.size());
    auto str = buf.consumeString(4);
    BOLT_ASSERT_EQ("data", str);
    BOLT_ASSERT_EQ(0u, buf.size());
}
```

Test: append, consume partial, consume all, overflow, empty buffer operations.

## 9. Cross-Module Integration

Structure: create resources in module A, pass to module B, verify in module C.

```cpp
BOLT_TEST(CrossModule, EditorWorkflow) {
    auto& tabBar = TabBar::getInstance();
    auto& editor = EditorStore::getInstance();
    auto& folding = CodeFolding::getInstance();
    
    auto tabId = tabBar.addTab(path);
    EditorDocument doc;
    doc.filePath = path;
    doc.value = content;
    doc.scroll = {0, 0}; // always initialize!
    editor.setDocument(path, doc);
    folding.addFoldRange(path, 0, 5);
    
    // Verify cross-module state
    auto ranges = folding.getFoldingRanges(path);
    BOLT_ASSERT(ranges.size() > 0);
    
    // Cleanup
    tabBar.closeTab(tabId);
}
```

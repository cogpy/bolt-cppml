# Common C++ Bugs Found During E2E Test Creation

## Table of Contents

1. Double-free / Double-close
2. Duplicate Struct/Class Definitions
3. Thread Safety / Deadlocks
4. Uninitialized Members
5. Singleton Ordering
6. API Mismatch Between Header and Implementation

## 1. Double-free / Double-close

**Pattern:** `unique_ptr` with custom deleter + manual release/close call.

```cpp
// BUG: pclose called twice
std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
// ... read from pipe ...
int code = pclose(pipe.release()); // double-close!
```

**Fix:** Use raw pointer with single `pclose`, or let `unique_ptr` handle cleanup exclusively.

```cpp
FILE* raw = popen(cmd, "r");
if (!raw) return {false, ""};
// ... read ...
int code = pclose(raw);
```

## 2. Duplicate Struct/Class Definitions

**Pattern:** Same struct name defined in multiple headers within the same namespace, with different fields.

**Detection:** Compilation errors like "redefinition of struct X" or field-not-found errors when the wrong definition is picked by include order.

**Fix:** Consolidate into a single definition. Use forward declarations or `#include` the canonical header from other files.

## 3. Thread Safety / Deadlocks

**Pattern:** Method A holds lock on resource X, then calls method B which tries to lock resource Y, while method B also internally accesses resource X.

**Example:** `closeTab()` holds write lock on `tabs_`, internally reads `activeTabId_` which uses a different `shared_mutex`. A `while(hasTabs()) { closeTab(getActiveTab()); }` loop deadlocks.

**Fix:** Use `closeAllTabs()` or batch operations. For tests, avoid patterns that chain lock-acquiring calls. Add `std::this_thread::sleep_for` after thread-spawning operations.

## 4. Uninitialized Members

**Pattern:** Struct with numeric members that default to garbage values, causing validation failures.

```cpp
struct EditorDocument {
    ScrollPosition scroll; // scroll.line and scroll.character are garbage!
};
```

**Fix:** Always zero-initialize struct members in tests:
```cpp
doc.scroll.line = 0;
doc.scroll.character = 0;
```

## 5. Singleton Ordering

**Pattern:** Tests that modify singleton state (e.g., `PluginSystem::getInstance()`) can affect subsequent tests.

**Fix:** Call reset/clear methods at the start of tests that depend on clean state. Use `forceReset()` where available.

## 6. API Mismatch Between Header and Implementation

**Pattern:** Header declares `createTab(path)` but implementation uses `addTab(path)`, or return types differ.

**Detection:** Compile errors during test writing.

**Fix:** Always `grep` the actual header for the exact method signature before writing tests. Use:
```bash
grep -n "methodName" include/path/to/header.hpp
```

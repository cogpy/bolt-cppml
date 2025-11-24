# Memory Leak Detection

Bolt C++ ML provides comprehensive memory leak detection tools to help identify and diagnose memory management issues in your code.

## Overview

The memory leak detection system consists of two main components:

1. **MemoryManager** - Basic allocation tracking and leak detection
2. **MemoryLeakDetector** - Enhanced leak detection with source location tracking and detailed reporting

## Basic Memory Manager

The `MemoryManager` class provides fundamental memory tracking capabilities:

### Features

- Track all allocations and deallocations
- Monitor current and peak memory usage
- Detect memory leaks
- Thread-safe operations
- Memory usage limits

### Usage

```cpp
#include "bolt/core/memory_manager.hpp"

auto& memMgr = bolt::MemoryManager::getInstance();

// Allocate memory
void* ptr = memMgr.allocate(1024);

// Use the memory
// ...

// Deallocate memory
memMgr.deallocate(ptr);

// Check for leaks
if (memMgr.hasMemoryLeaks()) {
    auto leaks = memMgr.getLeakedAllocations();
    std::cout << "Detected " << leaks.size() << " memory leaks\n";
}

// Get memory statistics
std::cout << "Current usage: " << memMgr.getCurrentUsage() << " bytes\n";
std::cout << "Peak usage: " << memMgr.getPeakUsage() << " bytes\n";
```

### Setting Memory Limits

```cpp
// Set maximum allowed memory usage (e.g., 100 MB)
memMgr.setMaxAllowedUsage(100 * 1024 * 1024);

try {
    void* ptr = memMgr.allocate(1024 * 1024 * 1024); // Try to allocate 1 GB
} catch (const bolt::MemoryException& e) {
    std::cerr << "Allocation failed: " << e.what() << "\n";
}
```

## Enhanced Memory Leak Detector

The `MemoryLeakDetector` class provides advanced leak detection with source location tracking and detailed reporting.

### Features

- **Source Location Tracking**: Track where allocations were made (file, line, function)
- **Categorization**: Organize allocations by category
- **Detailed Reporting**: Generate comprehensive leak reports
- **Statistics**: Get leak statistics by category, file, and more
- **Allocation Age**: Track how long allocations have been alive
- **Thread-Safe**: Safe for multi-threaded applications

### Basic Usage

```cpp
#include "bolt/core/memory_leak_detector.hpp"

auto& detector = bolt::MemoryLeakDetector::getInstance();

// Enable leak detection (enabled by default)
detector.setEnabled(true);

// Track an allocation
void* ptr = malloc(1024);
TRACK_ALLOCATION(ptr, 1024, "buffers");

// Use the memory
// ...

// Untrack when deallocating
UNTRACK_ALLOCATION(ptr);
free(ptr);

// Check for leaks
if (detector.hasLeaks()) {
    std::cout << detector.generateReport() << std::endl;
}
```

### Manual Tracking

```cpp
void* ptr = malloc(2048);
detector.trackAllocation(
    ptr,                    // Pointer
    2048,                   // Size in bytes
    __FILE__,              // Source file
    __LINE__,              // Line number
    __FUNCTION__,          // Function name
    "network_buffers"      // Category
);

// Later...
detector.untrackAllocation(ptr);
free(ptr);
```

### Generating Reports

#### Detailed Report

```cpp
std::string report = detector.generateReport();
std::cout << report << std::endl;
```

Example output:
```
=== Memory Leak Detection Report ===

⚠ Memory leaks detected!

Summary:
  Total leaks: 3
  Total leaked: 12.50 KB
  Peak usage: 45.25 MB
  Current usage: 12.50 KB

Leaks by category:
  buffers: 8.00 KB
  network_buffers: 4.50 KB

Leaks by file:
  src/network/buffer.cpp: 4.50 KB
  src/core/memory.cpp: 8.00 KB

Detailed leak information:

Leak #1:
  Address: 0x7f1234567890
  Size: 4.00 KB
  Location: src/network/buffer.cpp:42
  Function: allocateNetworkBuffer
  Category: network_buffers
  Age: 125 seconds

Leak #2:
  Address: 0x7f1234567abc
  Size: 8.00 KB
  Location: src/core/memory.cpp:156
  Function: allocateTempBuffer
  Category: buffers
  Age: 87 seconds

Leak #3:
  Address: 0x7f1234567def
  Size: 512.00 B
  Location: src/network/buffer.cpp:67
  Function: createSmallBuffer
  Category: network_buffers
  Age: 45 seconds
```

#### Summary Report

```cpp
std::string summary = detector.generateSummary();
std::cout << summary << std::endl;
// Output: "3 leaks, 12.50 KB leaked"
```

### Getting Statistics

```cpp
auto stats = detector.getStats();

std::cout << "Total leaks: " << stats.totalLeaks << "\n";
std::cout << "Total leaked bytes: " << stats.totalLeakedBytes << "\n";
std::cout << "Peak memory usage: " << stats.peakMemoryUsage << "\n";

// Leaks by category
for (const auto& [category, bytes] : stats.leaksByCategory) {
    std::cout << category << ": " << bytes << " bytes\n";
}

// Leaks by file
for (const auto& [file, bytes] : stats.leaksByFile) {
    std::cout << file << ": " << bytes << " bytes\n";
}
```

### Categorizing Allocations

Organize allocations by category for better leak analysis:

```cpp
// Network buffers
void* netBuf = malloc(4096);
TRACK_ALLOCATION(netBuf, 4096, "network");

// Image data
void* imgData = malloc(1024 * 1024);
TRACK_ALLOCATION(imgData, 1024 * 1024, "images");

// Temporary buffers
void* tempBuf = malloc(512);
TRACK_ALLOCATION(tempBuf, 512, "temporary");
```

### Integration with RAII

For RAII-style automatic tracking:

```cpp
template<typename T>
class TrackedPtr {
public:
    TrackedPtr(size_t size, const std::string& category = "general") 
        : size_(size), category_(category) {
        ptr_ = static_cast<T*>(malloc(size));
        TRACK_ALLOCATION(ptr_, size, category);
    }
    
    ~TrackedPtr() {
        if (ptr_) {
            UNTRACK_ALLOCATION(ptr_);
            free(ptr_);
        }
    }
    
    T* get() { return ptr_; }
    
private:
    T* ptr_;
    size_t size_;
    std::string category_;
};

// Usage
{
    TrackedPtr<char> buffer(1024, "buffers");
    // Use buffer.get()...
} // Automatically tracked and untracked
```

## Best Practices

### 1. Use Categories Wisely

Organize allocations by logical categories:

```cpp
// Good categories
TRACK_ALLOCATION(ptr, size, "network_buffers");
TRACK_ALLOCATION(ptr, size, "image_cache");
TRACK_ALLOCATION(ptr, size, "string_pool");

// Avoid generic categories
TRACK_ALLOCATION(ptr, size, "general");  // Less useful
```

### 2. Always Pair Track/Untrack

```cpp
void processData() {
    void* buffer = malloc(1024);
    TRACK_ALLOCATION(buffer, 1024, "processing");
    
    try {
        // Process data...
    } catch (...) {
        UNTRACK_ALLOCATION(buffer);
        free(buffer);
        throw;
    }
    
    UNTRACK_ALLOCATION(buffer);
    free(buffer);
}
```

### 3. Use RAII When Possible

Prefer RAII wrappers to ensure proper cleanup:

```cpp
class ScopedBuffer {
public:
    ScopedBuffer(size_t size, const std::string& category) {
        ptr_ = malloc(size);
        TRACK_ALLOCATION(ptr_, size, category);
    }
    
    ~ScopedBuffer() {
        UNTRACK_ALLOCATION(ptr_);
        free(ptr_);
    }
    
    void* get() { return ptr_; }
    
private:
    void* ptr_;
};
```

### 4. Generate Reports Regularly

In development builds, generate leak reports:

```cpp
#ifdef DEBUG
void checkForLeaks() {
    auto& detector = bolt::MemoryLeakDetector::getInstance();
    if (detector.hasLeaks()) {
        std::cerr << detector.generateReport() << std::endl;
        // Consider asserting or logging
    }
}

// Call at application shutdown
atexit(checkForLeaks);
#endif
```

### 5. Clear Tracking Between Tests

In unit tests, clear tracking between test cases:

```cpp
TEST_CASE("Memory test 1") {
    auto& detector = bolt::MemoryLeakDetector::getInstance();
    
    // Test code...
    
    REQUIRE(detector.hasLeaks() == false);
}

TEST_CASE("Memory test 2") {
    auto& detector = bolt::MemoryLeakDetector::getInstance();
    detector.resetStats(); // Reset peak usage stats
    
    // Test code...
}
```

## Integration with Build Systems

### CMake Integration

Enable leak detection in debug builds:

```cmake
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(bolt_lib PRIVATE ENABLE_LEAK_DETECTION)
endif()
```

### Conditional Compilation

```cpp
#ifdef ENABLE_LEAK_DETECTION
    #define TRACK_ALLOC(ptr, size, cat) TRACK_ALLOCATION(ptr, size, cat)
    #define UNTRACK_ALLOC(ptr) UNTRACK_ALLOCATION(ptr)
#else
    #define TRACK_ALLOC(ptr, size, cat)
    #define UNTRACK_ALLOC(ptr)
#endif
```

## Troubleshooting

### Common Issues

#### False Positives

**Issue**: Legitimate allocations reported as leaks

**Solution**: Ensure proper untracking before deallocation

```cpp
// Wrong
free(ptr);
UNTRACK_ALLOCATION(ptr); // Too late!

// Correct
UNTRACK_ALLOCATION(ptr);
free(ptr);
```

#### Missing Leaks

**Issue**: Known leaks not being reported

**Solution**: Ensure tracking is enabled and allocation was tracked

```cpp
// Check if enabled
if (!detector.isEnabled()) {
    detector.setEnabled(true);
}

// Ensure allocation is tracked
TRACK_ALLOCATION(ptr, size, category);
```

#### Performance Impact

**Issue**: Leak detection slowing down the application

**Solution**: Disable in release builds or use sampling

```cpp
#ifdef NDEBUG
    detector.setEnabled(false);
#endif
```

## Performance Considerations

- **Overhead**: ~20-50ns per allocation/deallocation in debug builds
- **Memory**: ~40 bytes per tracked allocation
- **Thread Safety**: All operations are thread-safe but may contend on mutex
- **Recommendation**: Enable in debug/testing builds, disable in production

## API Reference

See the [API Documentation](api/html/index.html) for complete class and function reference.

## Examples

Complete examples are available in:
- `examples/memory_leak_detection_example.cpp`
- `test/test_memory_manager.cpp`
- `test/test_memory_leak_detector.cpp`

## Related Documentation

- [Memory Manager](api/html/classMemoryManager.html)
- [Error Handling](api/html/error__handling_8hpp.html)
- [Testing Guide](../TESTING.md)

---

**Last Updated**: November 2024

**See Also**: [Performance Profiler](PERFORMANCE_PROFILER.md), [Logging System](LOGGING_SYSTEM.md)

---
name: bolt-cpp-ml-cpp-e2e-test-gen
description: Part of the bolt-cpp-ml skill: Generate comprehensive end-to-end unit tests for all functions in a C++ repository.
---

# C++ E2E Test Generator (within bolt-cpp-ml)

Generate comprehensive E2E unit tests for every public function in a C++ repository. The process catalogs all headers, writes tests for each function, fixes compilation errors and bugs discovered along the way, stabilizes the test suite, and commits.

## Workflow Overview

1. **Clone & Analyze** — Clone repo, map directory structure, identify test framework
2. **Catalog Functions** — Extract all public APIs from headers (run `extract_signatures.py`)
3. **Fix Pre-existing Bugs** — Fix header conflicts, API mismatches, segfaults found during analysis
4. **Write Tests** — Generate one comprehensive test file covering all modules
5. **Compile & Fix** — Iteratively fix compilation errors by checking actual API signatures
6. **Stabilize** — Run tests repeatedly, fix race conditions and intermittent failures
7. **Report & Commit** — Generate test report, commit, push

## Phase 1: Clone & Analyze

```bash
gh repo clone <org>/<repo>
cd <repo>
find . -type f \( -name "*.hpp" -o -name "*.h" -o -name "*.cpp" \) \
  -not -path "*/vcpkg/*" -not -path "*/build/*" -not -path "*/.git/*" \
  -not -path "*/ggml/*" -not -path "*/.ccls-cache/*" | head -50
```

Identify:
- **Test framework**: Look for `test/` directory, existing test macros (`BOLT_TEST`, `TEST_F`, `GTEST`, `CATCH2`)
- **Build system**: Check `CMakeLists.txt` or `Makefile`
- **Existing test runner**: Read `test/test_runner.cpp` or equivalent `main()`
- **Library structure**: Map `include/` and `src/` module directories

## Phase 2: Catalog Functions

Run the signature extractor to get a JSON catalog:

```bash
python3 /home/ubuntu/skills/bolt-cpp-ml-cpp-e2e-test-gen/scripts/extract_signatures.py <repo_root> > /tmp/signatures.json
```

Review the summary, then read key headers manually to understand:
- Constructor signatures and required parameters
- Singleton patterns (`getInstance()`)
- Return types (especially `optional`, `shared_ptr`, `vector`)
- Enum values used as parameters

**Critical rule:** Always `grep` the actual header for exact method signatures before writing any test. Never assume API names.

```bash
grep -n "methodName" include/path/to/header.hpp
```

## Phase 3: Fix Pre-existing Bugs

Common bugs found during test creation — see `references/common-bugs.md` for detailed patterns:

- **Duplicate struct definitions** across headers (different fields, same name, same namespace)
- **Double-free** in RAII wrappers (`unique_ptr` deleter + manual release)
- **Thread safety** issues (deadlocks from nested lock acquisition)

Fix bugs in the source code directly. These fixes are valuable deliverables alongside the tests.

## Phase 4: Write Tests

Create a single comprehensive test file: `test/test_comprehensive_e2e.cpp`

### Structure

```cpp
// 1. All includes
#include "bolt/bolt.hpp"
#include "bolt/test_framework.hpp"
// ... all module headers

// 2. Using declarations
using namespace bolt;

// 3. Tests grouped by module with section headers
// ============================================
// E2E_ModuleName: Test description
// ============================================
BOLT_TEST(E2E_ModuleName, FunctionName_Scenario) {
    // Arrange, Act, Assert
}
```

### Test patterns by construct type

See `references/test-patterns.md` for complete patterns covering:
- Static utility functions
- Singleton classes (always reset state first)
- RAII / resource management (verify allocation + deallocation)
- Thread-spawning classes (add `sleep_for` after start/stop)
- Callback registration
- Template classes (test with int and double)
- Cross-module integration workflows

### Key rules

1. **Initialize all struct members** — Uninitialized members cause validation exceptions
2. **Reset singleton state** at test start — Use `forceReset()`, `clear()`, etc.
3. **Never assert `is_running`** for short-lived thread tasks — They may halt before the check
4. **Use batch cleanup** instead of `while(hasItems()) removeItem()` — Avoids deadlocks
5. **Add 50ms sleep** after thread start/stop operations
6. **Test error paths** — Invalid input, empty containers, null pointers, out-of-range

### Cross-module integration tests

Write 5-10 workflow tests that chain operations across modules:
- Editor workflow: tab → document → folding → dirty state → close
- Memory + logging: allocate → log → verify → deallocate
- Network: buffer → compress → decompress → verify
- Full session: tab → syntax highlight → cursor → find/replace → fold → close

### Register in CMakeLists.txt

See `templates/cmake_test_target.cmake` for the template. Add the test executable target and link against all project libraries.

## Phase 5: Compile & Fix

Iterative compilation loop:

```bash
cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug
make <test_target> -j$(nproc) 2>&1 | tail -30
```

For each error:
1. Read the error message to identify the mismatched API
2. `grep` the actual header for the correct signature
3. Fix the test to match the real API
4. Rebuild

Common compilation fixes:
- Method name mismatch (`createTab()` → `addTab()`)
- Field name mismatch (`result.averageTime` → `result.averageDurationMs`)
- Missing constructor arguments (check struct definitions)
- Template argument deduction failures

## Phase 6: Stabilize

Run the test suite 5 times to detect intermittent failures:

```bash
for i in 1 2 3 4 5; do
  echo "Run $i:"
  LD_LIBRARY_PATH=build:build/lib timeout 120 build/test/<test_binary> 2>&1 \
    | grep -E "Passed:|Failed:"
done
```

For intermittent failures:
- **Race conditions**: Add `sleep_for(50ms)` after thread operations
- **Uninitialized state**: Zero-initialize all struct members
- **Singleton pollution**: Add reset calls between tests
- **File system timing**: Use unique temp paths per test

Target: **0 failures across 5 consecutive runs**.

## Phase 7: Report & Commit

Generate the test report:

```bash
LD_LIBRARY_PATH=build:build/lib ./build/test/<binary> > /tmp/test_output.txt 2>&1
python3 /home/ubuntu/skills/bolt-cpp-ml-cpp-e2e-test-gen/scripts/gen_test_report.py \
  /tmp/test_output.txt test/test_comprehensive_e2e.cpp -o test/TEST_REPORT.md
```

Commit with descriptive message listing all modules tested and bugs fixed:

```bash
git add test/ include/ src/  # include bug fixes
git commit -m "feat(test): add comprehensive E2E unit tests (N tests)

Modules: [list all modules]
Bug fixes: [list all bugs fixed]"
git push origin main
```

## Deliverables Checklist

- [ ] `test/test_comprehensive_e2e.cpp` — All tests in one file
- [ ] `test/TEST_REPORT.md` — Summary with pass/fail counts per suite
- [ ] `test/CMakeLists.txt` — Updated with new test target
- [ ] Bug fixes in `include/` and `src/` — Documented in commit message
- [ ] 5 consecutive clean runs with 0 failures

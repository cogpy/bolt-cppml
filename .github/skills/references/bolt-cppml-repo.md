# bolt-cppml Repository Integration Guide

This document describes how to work with the `cogpy/bolt-cppml` repository using the `bolt-cpp-ml` skill.

## Repository Overview

The `bolt-cppml` repository is a C++ implementation of the Bolt IDE core with AI integration. It contains:

- **`ggml/kobold.cpp`**: KoboldCpp inference engine (local LLM)
- **`ggml/llama.cpp`**: llama.cpp for GGUF model inference
- **`ggml/rwkv.cpp`**: RWKV recurrent neural network
- **`include/bolt/`**: C++ header files for all components
- **`src/bolt/`**: C++ source files
- **`test/`**: Comprehensive E2E test suite (250+ tests)
- **`.github/agents/`**: Agent specification files

## Build System

The project uses CMake with vcpkg for dependency management.

```bash
# Clone
gh repo clone cogpy/bolt-cppml
cd bolt-cppml

# Setup dependencies (vcpkg preferred)
./scripts/setup-deps.sh

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)

# Run tests
LD_LIBRARY_PATH=.:./bin:./lib ./test/bolt_comprehensive_e2e_tests
```

## Key Architecture Components

### AI/ML Layer
- **GGML**: Core tensor operations and model inference
- **RWKV**: Recurrent neural network for sequential processing
- **Direct GGUF Inference**: Load and run GGUF models directly
- **GPU Acceleration**: CUDA/OpenCL/Vulkan support

### Editor Core
- Multi-cursor support, code folding, syntax highlighting
- File tree navigation, split view, minimap
- Theme system, keyboard shortcuts
- Find/Replace with regex

### Collaboration Layer
- WebSocket-based real-time collaborative editing
- Operational Transformation for conflict resolution
- Multi-user sessions with presence indicators

### Development Tools
- LSP client/server integration
- Git integration
- Integrated debugger with breakpoints
- Performance profiler and memory leak detector

## Known Issues and Fixes

### 1. GGML Dependency Missing
**Symptom:** `fatal error: ggml.h: No such file or directory`
**Fix:** Run `./scripts/setup-deps.sh` or manually install via vcpkg:
```bash
./vcpkg install ggml
```

### 2. Duplicate Struct Definitions (FIXED)
**Files:** `include/bolt/editor/bracket_matcher.hpp`, `include/bolt/gui_components.hpp`
**Fix:** Already applied in the E2E test suite — `BracketMatch` unified to use `openPos`/`closePos`.

### 3. Double-free in GitRepository (FIXED)
**File:** `src/bolt/git/git_repository.cpp`
**Fix:** Replaced `pclose` deleter pattern with `git -C` flag and single raw `pclose()`.

### 4. Duplicate Token/SyntaxHighlighter (FIXED)
**File:** `include/bolt/editor/syntax_highlighter.hpp`
**Fix:** Removed duplicate second definition.

### 5. vcpkg committed to repo
**Issue:** `.vcpkg/versions/` directory (large) is committed to the repository.
**Recommendation:** Add `.vcpkg/versions/` to `.gitignore` and use vcpkg as a submodule or external tool.

## E2E Test Suite

The repository has a comprehensive E2E test suite with 250+ tests covering all modules:

| Module | Tests | Status |
|---|---|---|
| TabBar | 8 | ✅ |
| EditorStore | 9 | ✅ |
| SyntaxHighlighter | 7 | ✅ |
| CodeFolding | 6 | ✅ |
| MultiCursor | 5 | ✅ |
| FindReplace | 7 | ✅ |
| BracketMatcher | 5 | ✅ |
| Minimap | 4 | ✅ |
| SplitView | 4 | ✅ |
| ThemeSystem | 6 | ✅ |
| KeyboardShortcuts | 5 | ✅ |
| FileTree | 6 | ✅ |
| LSP | 8 | ✅ |
| Debugger | 7 | ✅ |
| GitRepository | 10 | ✅ |
| Logging | 9 | ✅ |
| CrossModule | 11 | ✅ |

Run the full suite:
```bash
cd build
make bolt_comprehensive_e2e_tests -j$(nproc)
LD_LIBRARY_PATH=.:./bin:./lib ./test/bolt_comprehensive_e2e_tests
```

## Adding New Features

When adding new C++ features to bolt-cppml:

1. **Add header** to `include/bolt/<module>/`
2. **Add source** to `src/bolt/<module>/`
3. **Update CMakeLists.txt** to include new files
4. **Run cpp-e2e-test-gen** to generate tests:
   ```bash
   python3 /home/ubuntu/skills/bolt-cpp-ml/references/cpp-e2e-test-gen/scripts/extract_signatures.py . > /tmp/signatures.json
   ```
5. **Write tests** in `test/test_comprehensive_e2e.cpp`
6. **Compile and stabilize** (5 consecutive clean runs)
7. **Commit** with descriptive message

## KoboldCpp Integration

The `ggml/kobold.cpp` directory contains the KoboldCpp inference engine. To use it with the bolt-cpp-ml skill:

```python
import requests

# KoboldCpp runs on port 5001 by default
ENDPOINT = "http://localhost:5001"

# Code completion request
response = requests.post(f"{ENDPOINT}/v1/completions", json={
    "prompt": "// Complete this C++ function:\nvoid sortVector(std::vector<int>& v) {",
    "max_tokens": 200,
    "temperature": 0.3,
    "stop": ["\n}"]
})
print(response.json()["choices"][0]["text"])
```

## Jan Extension Packaging

To package bolt-cppml as a Jan extension:

```bash
# In the bolt-cppml directory
npm install
npm run bundle
# → produces bolt-cppml-extension.tgz
```

The extension exposes:
- `onStart()`: Connect to KoboldCpp backend
- `onMessageSent()`: Route inference requests to local GGUF models
- `onStop()`: Clean up connections

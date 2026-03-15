---
name: bolt-cppml
description: Build and optimize the Bolt C++ ML IDE with neural network module architecture. Use for C++ IDE development with GGML integration, RWKV neural networks, AI code completion, GPU acceleration, and modular component composition following nn patterns.
source_chain: function-creator( c++( nn( bolt-new ) ) )
---

# Bolt C++ ML — AI-Powered C++ IDE

Bolt C++ ML is a modular C++ IDE with integrated machine learning capabilities, combining GGML-based inference, RWKV neural networks, and a full-featured editor with split views, multi-cursor editing, code folding, theming, keyboard shortcuts, plugin system, LSP integration, and collaborative editing.

## Architecture Overview

The project follows a **neural network module composition pattern** where each IDE component is treated as a composable module (analogous to `nn.Module` in Torch7), connected through a sequential pipeline.

### Core Module Hierarchy

| Module Layer | Component | Source Path |
|---|---|---|
| **AI / Inference** | GGML Wrapper, RWKV Wrapper, Direct GGUF Inference | `src/bolt/ai/` |
| **Editor** | Split View, Multi-Cursor, Code Folding, Keyboard Shortcuts | `src/bolt/editor/` |
| **Core** | Chat Store, Editor Store, File Store, Workbench Store | `src/bolt/core/` |
| **GUI** | ImGui Integration, Theme System, Tab Bar, Minimap | `src/bolt/gui/` |
| **Git** | Repository Management, Diff, Staging | `src/bolt/git/` |
| **Network** | Collaboration, Network Optimizations | `src/bolt/network/` |
| **System** | Logging, Debugger, Memory Leak Detector, Profiler | `src/bolt/system/` |
| **Plugin** | Plugin System, Extension API | `src/bolt/plugin/` |
| **LSP** | Language Server Protocol Client | `src/bolt/lsp/` |

### Key Files

| File | Purpose |
|---|---|
| `CMakeLists.txt` | Top-level build configuration |
| `test/CMakeLists.txt` | Test framework with CTest integration |
| `src/bolt/ai/ggml_wrapper.cpp` | GGML backend for tensor operations |
| `src/bolt/ai/rwkv_wrapper.cpp` | RWKV time-mixing and channel-mixing layers |
| `src/bolt/ai/direct_gguf_inference.cpp` | Direct GGUF model loading and inference |
| `include/bolt/ai/*.hpp` | AI module headers |
| `include/bolt/editor/*.hpp` | Editor component headers |
| `include/bolt/core/*.hpp` | Core store headers |

## Quick Start

### Prerequisites

- C++20 compatible compiler (GCC 11+, Clang 14+)
- CMake 3.15+
- GGML library (bundled as submodule)

### Build

```bash
git clone https://github.com/cogpy/bolt-cppml.git
cd bolt-cppml
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Test

```bash
cd build
ctest --output-on-failure
```

All 97 tests should pass with 0 warnings. The CTest configuration automatically sets `LD_LIBRARY_PATH` for shared library resolution.

## Neural Network Module Pattern

Each component follows the nn module pattern:

### Forward Pass (Data Flow)

```
Input → [Tokenizer] → [RWKV TimeMixing] → [RWKV ChannelMixing] → [LayerNorm] → [Output Projection] → Completion
```

### RWKV Time-Mixing (WKV Attention)

The RWKV wrapper implements the full WKV attention mechanism:

```
xk = last_x + (x - last_x) * time_mix_k
xv = last_x + (x - last_x) * time_mix_v
xr = last_x + (x - last_x) * time_mix_r

k = Wk @ xk,  v = Wv @ xv,  r = Wr @ xr

wkv = (last_num + exp(bonus + k) * v) / (last_den + exp(bonus + k))
output = Wout @ (sigmoid(r) * wkv)
```

### RWKV Channel-Mixing (Feed-Forward with Memory)

```
xk = last_x + (x - last_x) * time_mix_k
xr = last_x + (x - last_x) * time_mix_r

k = Wk @ xk,  r = Wr @ xr
vk = Wv @ (relu(k))^2
output = sigmoid(r) * vk
```

## Test Framework

The project uses a custom lightweight test framework with CTest integration:

| Test Suite | Tests | Description |
|---|---|---|
| Core | 7 | Chat, Memory, Store, String, FileTree, Minimap |
| Editor | 5 | SplitView, MultiCursor, KeyboardShortcuts, Theme, CodeFolding |
| ErrorHandling | 7 | Error recovery, boundary conditions |
| AI | 15 | GGML wrapper, AI models, KoboldCpp provider (13 suites) |
| Comprehensive E2E | 27 | Cross-module, DrawKern VM, Styx, Git, Benchmark, Plugin |
| Extended E2E | 11 | DataProcessor, MathUtils, FileSystem, LineNumbers, VectorDB, OT edge cases |
| Integration | 2 | Full integration tests |
| System | 4 | Debugger, Logging, MemoryLeak, Sanitizer |

### Running Specific Test Labels

```bash
ctest -L Unit          # All unit tests
ctest -L Editor        # Editor component tests
ctest -L AI            # AI/ML tests
ctest -L Integration   # Integration tests
```

## Extending the IDE

### Adding New Modules

1. Create header in `include/bolt/<category>/`
2. Create implementation in `src/bolt/<category>/`
3. Add source to `CMakeLists.txt` bolt_lib target
4. Add test in `test/`
5. Register test in `test/CMakeLists.txt`

### Plugin System

Plugins follow the module pattern with lifecycle hooks:

```cpp
class MyPlugin : public bolt::Plugin {
    void onActivate() override;
    void onDeactivate() override;
    std::string getName() const override;
};
```

## Build Status

- **Errors**: 0
- **Warnings**: 0
- **Tests**: 97/97 passing (100%)
- **CTest**: Fully configured with LD_LIBRARY_PATH and labels
- **Labels**: Unit, Core, Editor, ErrorHandling, AI, KoboldCpp, Extended, Integration

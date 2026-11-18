
# Bolt C++

A modern C++ implementation of the Bolt IDE core components with AI integration and real-time collaborative features.

## Features

### Core Engine
- GGML integration for AI model inference
- RWKV implementation for RNN capabilities
- Thread-safe operations
- Optimized memory management
- Code completion engine
- Robust error handling

### Editor Features
- Multi-cursor support
- Line numbering system
- Find/Replace functionality 
- Bracket matching
- Syntax highlighting
- File tree navigation
- Theme system
- Code folding
- Auto-completion
- Split view editing
- Integrated debugger with breakpoints and step debugging

### Network Components
- WebSocket server for real-time collaboration
- HTTP server implementation
- Network command handling

### AI Integration
- GGML wrapper for model inference
- RWKV neural network support
- Tokenizer implementation
- Chat system integration

## Project Structure

```
bolt/
├── include/          # Header files
│   └── bolt/
│       ├── ai/      # AI-related headers
│       ├── core/    # Core functionality
│       ├── editor/  # Editor components
│       ├── network/ # Network handling
│       └── utils/   # Utility functions
├── src/             # Source files
│   └── bolt/
│       ├── ai/      
│       ├── core/    
│       ├── editor/  
│       ├── network/ 
│       └── utils/   
├── test/            # Unit tests
└── docs/            # Documentation
```

## Building

The project features comprehensive package management integration with vcpkg, Conan, and system package managers. See [docs/dependency-management.md](docs/dependency-management.md) for detailed setup instructions.

### Quick Start

#### Automated Setup (Recommended)
```bash
# One-command setup with automatic package manager detection
./scripts/setup-deps.sh

# This will:
# 1. Install system dependencies
# 2. Setup vcpkg or Conan
# 3. Configure and build the project
```

#### Manual Setup
```bash
# Using CMake presets (recommended)
cmake --preset vcpkg
cmake --build --preset vcpkg-release

# Or traditional approach
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake ..
make -j$(nproc)

# Run tests
./test/bolt_unit_tests
```

Builds are portable by default: they target a fixed x86-64 baseline (AVX2)
rather than the CPU of the machine doing the build, so the binaries can be
copied to other machines. For a build you will only run locally, add
`-DBOLT_NATIVE_OPTIMIZATIONS=ON` to tune it for your own CPU.

After building, `scripts/smoke_test.sh <build-dir>` starts `bolt` and every
demo program and reports any that hang or crash.

### Package Manager Integration Features

- **🔧 Multi-Manager Support**: vcpkg (primary), Conan (alternative), system packages (fallback)
- **🎯 Smart Detection**: Automatically chooses the best available package manager
- **📋 Enhanced CMake**: Modern target-based linking with fallback mechanisms
- **🛠️ Developer Tools**: Status checking, automated installation, integration testing

### Development Tools

```bash
# Check package manager status
python3 scripts/package-manager.py --status

# Install dependencies with specific manager
python3 scripts/package-manager.py --install --manager vcpkg

# Test package management integration
python3 scripts/test-package-management.py

# Get help for setup script
./scripts/setup-deps.sh --help
```

### Dependencies

- Modern C++ compiler (C++17 or later)
- CMake 3.15+
- Package manager: vcpkg (recommended) or Conan

**Managed Dependencies:**
- **Core Libraries**: curl, jsoncpp, glfw3, imgui, opengl
- **AI Support**: GGML library for AI model support
- **Features**: GUI components, HTTP clients, JSON processing
- **Development**: Optional testing framework (catch2)

**System Dependencies** (automatically installed):
- X11 development libraries (Linux)
- OpenGL development libraries
- Build tools (gcc/clang, ninja, pkg-config)

## License

MIT License

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on contributing to the project.

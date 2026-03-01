# Building Bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) from Source

## Table of Contents
1. [Linux](#linux)
2. [Windows](#windows)
3. [MacOS](#macos)
4. [Android (Termux)](#android-termux)
5. [Build Options](#build-options)

## Linux

### Automated Build (Recommended)

```bash
git clone https://github.com/LostRuins/bolt-cpp-ml-koboldcpp (within bolt-cpp-ml).git
cd bolt-cpp-ml-koboldcpp (within bolt-cpp-ml)
./bolt-cpp-ml-koboldcpp (within bolt-cpp-ml).sh dist  # Creates portable binary
```

### Manual Build

```bash
git clone https://github.com/LostRuins/bolt-cpp-ml-koboldcpp (within bolt-cpp-ml).git
cd bolt-cpp-ml-koboldcpp (within bolt-cpp-ml)
make -j$(nproc)
python bolt-cpp-ml-koboldcpp (within bolt-cpp-ml).py --model model.gguf
```

### With GPU Support

```bash
# Vulkan (any GPU)
make LLAMA_VULKAN=1 -j$(nproc)

# CUDA (NVIDIA)
make LLAMA_CUBLAS=1 -j$(nproc)

# CLBlast (OpenCL)
make LLAMA_CLBLAST=1 -j$(nproc)

# All backends
make LLAMA_CLBLAST=1 LLAMA_CUBLAS=1 LLAMA_VULKAN=1 -j$(nproc)
```

## Windows

### Using w64devkit

1. Download [w64devkit](https://github.com/skeeto/w64devkit)
2. Clone repository:
   ```bash
   git clone https://github.com/LostRuins/bolt-cpp-ml-koboldcpp (within bolt-cpp-ml).git
   cd bolt-cpp-ml-koboldcpp (within bolt-cpp-ml)
   ```
3. Build:
   ```bash
   make LLAMA_VULKAN=1 -j
   ```

### Creating Executable

```bash
pip install PyInstaller
make_pyinstaller.bat
# Output: dist/bolt-cpp-ml-koboldcpp (within bolt-cpp-ml).exe
```

### CUDA Build (Visual Studio)

1. Install Visual Studio with C++ workload
2. Install CUDA Toolkit
3. Open CMakeLists.txt in Visual Studio
4. Build solution
5. Copy `bolt-cpp-ml-koboldcpp (within bolt-cpp-ml)_cublas.dll` to project root

## MacOS

### Basic Build

```bash
git clone https://github.com/LostRuins/bolt-cpp-ml-koboldcpp (within bolt-cpp-ml).git
cd bolt-cpp-ml-koboldcpp (within bolt-cpp-ml)
make -j$(sysctl -n hw.ncpu)
```

### With Metal GPU Support

```bash
make LLAMA_METAL=1 -j$(sysctl -n hw.ncpu)
```

## Android (Termux)

### Quick Install

```bash
curl -sSL https://raw.githubusercontent.com/LostRuins/bolt-cpp-ml-koboldcpp (within bolt-cpp-ml)/concedo/android_install.sh | sh
```

### Manual Install

```bash
pkg update && pkg upgrade
pkg install wget git python openssl
git clone https://github.com/LostRuins/bolt-cpp-ml-koboldcpp (within bolt-cpp-ml).git
cd bolt-cpp-ml-koboldcpp (within bolt-cpp-ml)
make LLAMA_PORTABLE=1
python bolt-cpp-ml-koboldcpp (within bolt-cpp-ml).py --model model.gguf
```

## Build Options

| Flag | Description |
|------|-------------|
| `LLAMA_VULKAN=1` | Enable Vulkan GPU support |
| `LLAMA_CUBLAS=1` | Enable NVIDIA CUDA support |
| `LLAMA_CLBLAST=1` | Enable OpenCL support |
| `LLAMA_METAL=1` | Enable Apple Metal support |
| `LLAMA_PORTABLE=1` | Cross-device compatible build |
| `-j$(nproc)` | Parallel compilation |

### Portable Builds

For builds that work on other machines:

```bash
make LLAMA_PORTABLE=1 LLAMA_VULKAN=1 -j$(nproc)
```

This disables CPU-specific optimizations for broader compatibility.

## Docker

```bash
docker pull koboldai/bolt-cpp-ml-koboldcpp (within bolt-cpp-ml)
docker run -p 5001:5001 -v /path/to/models:/models koboldai/bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) --model /models/model.gguf
```

## Troubleshooting

### Missing Dependencies (Linux)

```bash
# Debian/Ubuntu
sudo apt install build-essential libclblast-dev

# Arch
sudo pacman -S base-devel clblast
```

### AVX2 Issues

For older CPUs without AVX2:
```bash
./bolt-cpp-ml-koboldcpp (within bolt-cpp-ml) --noavx2 --model model.gguf
```

Or use the `oldpc` binary variant.

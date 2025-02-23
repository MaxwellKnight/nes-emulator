# NES Emulator

A 6502 CPU emulator for the Nintendo Entertainment System (NES) written in C++.

## Prerequisites

- C++ compiler supporting C++17 or later
  - Windows: Visual Studio 2019 or later, or MinGW-w64
  - macOS: Clang or GCC
  - Linux: GCC or Clang
- CMake 3.14 or later
- Google Test framework

## Setup Instructions

### macOS

1. Install prerequisites:
```bash
# Install Homebrew if you haven't already
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install required packages
brew install cmake googletest
```

2. Clone and build:
```bash
git clone https://github.com/MaxwellKnight/cpu6502-emulator.git
cd nes-emulator
mkdir build && cd build
cmake ..
make
```

### Windows

1. Clone and build:
```cmd
git clone https://github.com/MaxwellKnight/cpu6502-emulator.git
cd nes-emulator
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

Alternatively, with Visual Studio:
- Open Visual Studio
- Select "Clone a repository"
- Enter the repository URL
- Click Clone
- Wait for CMake configuration to complete
- Select "Build All"

### Linux

1. Install prerequisites:
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install cmake g++ libgtest-dev

# Fedora
sudo dnf install cmake gcc-c++ gtest-devel
```

2. Clone and build:
```bash
git clone https://github.com/MaxwellKnight/cpu6502-emulator.git
cd nes-emulator
mkdir build && cd build
cmake ..
make
```

## Project Structure

```
.
├── CMakeLists.txt  # CMake build configuration
├── include/        # Header files
│   ├── bus.h      # System bus interface
│   └── cpu.h      # 6502 CPU implementation
├── src/           # Source files
│   └── cpu.cpp    # CPU implementation
└── tests/         # Test files
    └── cpu_test.cpp # CPU unit tests
```

## Running Tests

### macOS/Linux
```bash
cd build
ctest --output-on-failure
```

### Windows
```cmd
cd build
ctest -C Release --output-on-failure
```

Or use Visual Studio's Test Explorer.

## Contributing

1. Fork the repository
2. Create a new branch for your feature
3. Make your changes
4. Submit a pull request

## License

[Your chosen license]

## Notes

- If you're using Windows and MinGW-w64, make sure to use the x86_64 version
- Visual Studio 2019 and later include CMake support by default
- On Windows, you might need to add CMake to your system PATH

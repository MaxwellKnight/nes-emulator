# NES Emulator

A 6502 microprocessor emulator for the Nintendo Entertainment System (NES) written in C++ with WebAssembly support.

## Prerequisites
- Docker
- Docker Compose

## Quick Start

### Build and Run
```bash
# Build all (native + WebAssembly)
docker compose run build

# Build WebAssembly only
docker compose run wasm

# Run tests
docker compose run test

# Open interactive shell
docker compose run shell
```

## Development
1. Fork the repository
2. Create a feature branch
3. Make changes
4. Submit a pull request

## Local Development (without Docker)
- Requires: C++17, CMake 3.14+, Google Test
- Follow platform-specific setup in previous README

## License
GNU GPL v3 - see [LICENSE](LICENSE) file

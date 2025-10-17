# Docker Build System for Prong

This document describes how to use the containerized build system for the Prong C++20 UI Framework. The Docker-based build environment ensures reproducible builds across different development machines and CI/CD systems.

## Overview

The containerized build system provides:

- **Reproducible builds**: All dependencies and tools are version-pinned
- **Isolated environment**: No interference with host system packages
- **Multi-stage optimization**: Efficient layer caching for fast rebuilds
- **Flexible workflows**: Support for library-only, tests, and example builds

## Architecture

The build system uses a multi-stage Dockerfile with the following stages:

1. **toolchain**: Base Arch Linux with C++ compiler and build tools
2. **iwyu-builder**: Builds and installs include-what-you-use from AUR
3. **mise-tools**: Installs mise-managed tools (cmake, ninja, hk, pkl)
4. **builder**: Final build environment for library and tests (default target)
5. **examples-builder**: Extended environment with GLFW and OpenGL for examples

## Quick Start

### Using Mise Tasks (Recommended)

The easiest way to use the Docker build system is through mise tasks:

```bash
# Build the Docker image
mise docker-build

# Build the library
mise docker-build-lib

# Build and run tests
mise docker-build-tests
mise docker-test

# Build examples
mise docker-build-examples

# Run clang-format
mise docker-format

# Open an interactive shell
mise docker-shell
```

All mise Docker tasks are located in `mise-tasks/` and automatically handle image names, volume mounts, and path resolution.

### Building the Docker Image Manually

Build the default builder image (library-only):

```bash
docker build --target builder -t prong-builder:latest .
```

Build the examples image (includes GLFW and OpenGL):

```bash
docker build --target examples-builder -t prong-examples:latest .
```

### Using Docker Compose

Docker Compose provides an alternative workflow:

```bash
# Build the images
docker compose build

# Start an interactive shell in the builder container
docker compose run --rm builder

# Inside the container, build the library
mkdir -p build && cd build
cmake .. -G Ninja -DPRONG_BUILD_EXAMPLES=OFF -DPRONG_BUILD_TESTS=OFF
ninja

# Or use mise tasks
mise run build
```

### One-Line Build Commands

Build library without entering container:

```bash
docker run --rm -v $(pwd):/workspace prong-builder:latest \
  bash -c "rm -rf build && mise run build"
```

Build with tests:

```bash
docker run --rm -v $(pwd):/workspace prong-builder:latest \
  bash -c "rm -rf build && mise run build-tests"
```

Build with examples:

```bash
docker run --rm -v $(pwd):/workspace prong-examples:latest \
  bash -c "rm -rf build && mise run build-examples"
```

## Installed Tools and Versions

### System Packages (Arch Linux)

- **C++ Compiler**: GCC 15.2.1 (default), Clang 20.1.8 (available)
- **clang-format**: 20.1.8 (from official repos)
- **include-what-you-use**: 0.24 (from AUR)

### Mise-Managed Tools (Version-Pinned)

As defined in `.mise.toml`:

- **cmake**: 3.28.6
- **ninja**: 1.13.1
- **hk**: 1.18.3 (git hook manager)
- **pkl**: 0.29.1

All mise-managed tools are available through the container's entrypoint, which automatically activates mise.

## Development Workflows

### Interactive Development

Start an interactive shell with your source mounted:

```bash
docker compose run --rm builder

# Inside container, all tools are available
cmake --version
ninja --version
clang-format --version
include-what-you-use --version

# Build the library
mise run build

# Run tests (if built)
mise run test
```

### CI/CD Integration

The containerized build system is designed for CI/CD workflows:

```yaml
# Example GitHub Actions workflow
- name: Build Prong in Docker
  run: |
    docker build --target builder -t prong-builder:latest .
    docker run --rm -v $(pwd):/workspace prong-builder:latest \
      bash -c "rm -rf build && mise run build-ci"
```

For BombFork's self-hosted runners, the images can be pre-built and cached for faster builds.

### Format Checking and Fixing

Run clang-format through the container:

```bash
# Check formatting
docker run --rm -v $(pwd):/workspace prong-builder:latest \
  bash -c "mise run format-check"

# Fix formatting
docker run --rm -v $(pwd):/workspace prong-builder:latest \
  bash -c "mise run format"
```

### Git Hooks in Container

The git hooks use `hk` (git hook manager). When running hooks through the container:

```bash
# Install hooks (run on host)
hk install

# The hooks will use your host's tools by default
# To use containerized tools, you can create wrapper scripts
```

For delegating hooks to the container, create wrapper scripts in `.git/hooks/` that call Docker:

```bash
#!/bin/bash
# Example: .git/hooks/pre-commit-docker
docker run --rm -v $(pwd):/workspace prong-builder:latest \
  bash -c "hk run pre-commit"
```

## Build Options

### CMake Build Options

- `PRONG_BUILD_EXAMPLES`: Build example applications (default: ON)
- `PRONG_BUILD_TESTS`: Build unit tests (default: ON)

### Mise Tasks

#### Docker Tasks (Host)

Available through `mise <task>` on the host machine:

- `docker-build`: Build the Docker image
- `docker-build-lib`: Build the library inside Docker
- `docker-build-tests`: Build tests inside Docker
- `docker-build-examples`: Build examples inside Docker
- `docker-test`: Run tests inside Docker
- `docker-format`: Run clang-format inside Docker
- `docker-shell`: Open an interactive shell inside Docker

#### Build Tasks (Container)

Available through `mise run <task>` inside the container:

- `build`: Build library only
- `build-tests`: Build library and tests
- `build-examples`: Build library and examples
- `build-all`: Build everything
- `build-ci`: Build all and fail if formatting is needed
- `test`: Run test suite
- `format`: Run clang-format to fix code
- `format-check`: Check if code is properly formatted
- `demo`: Build and run demo application (requires examples)

## Volume Mounts

The Docker Compose configuration uses volume mounts for:

1. **Source code**: `.:/workspace` - Your source is mounted into the container
2. **Build cache**: Named volume `build-cache` - Preserves build artifacts between runs
3. **Mise cache**: Named volume `mise-cache` - Caches mise installations

This provides a good balance between flexibility and performance.

## Performance Optimization

### Build Cache

The multi-stage Dockerfile uses BuildKit cache mounts for:

- Pacman package cache (reduces re-downloads)
- Mise tool cache (faster tool installation)

Enable BuildKit for optimal performance:

```bash
export DOCKER_BUILDKIT=1
docker build --target builder -t prong-builder:latest .
```

### Incremental Builds

Use volume mounts to preserve build artifacts:

```bash
docker compose run --rm builder mise run build
# Subsequent builds will be incremental
docker compose run --rm builder mise run build
```

## Running Examples with GUI

To run graphical examples from the container, you need X11 forwarding:

### Linux

```bash
# Allow X11 connections
xhost +local:docker

# Run with X11 and GPU forwarding
docker compose run --rm \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix:ro \
  --device /dev/dri:/dev/dri \
  examples mise run demo

# Restore X11 permissions
xhost -local:docker
```

### macOS

```bash
# Install XQuartz first: https://www.xquartz.org/
# Allow network connections in XQuartz preferences

# Run with X11 forwarding
docker compose run --rm \
  -e DISPLAY=host.docker.internal:0 \
  examples mise run demo
```

### Windows (WSL2)

```bash
# With WSLg (Windows 11)
export DISPLAY=:0
docker compose run --rm \
  -e DISPLAY=$DISPLAY \
  -e WAYLAND_DISPLAY=$WAYLAND_DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix:ro \
  -v /mnt/wslg:/mnt/wslg \
  examples mise run demo
```

## Troubleshooting

### Locale Warnings

The Docker image includes proper locale support (`glibc-locales` package) and sets `LANG=en_US.UTF-8` and `LC_ALL=en_US.UTF-8`. If you still see locale warnings after a rebuild, the image may need to be rebuilt without cache:

```bash
docker build --no-cache -t prong-builder:latest .
```

### Build Cache Issues

If you encounter stale build artifacts:

```bash
# Clean build cache
docker compose down -v
docker volume rm prong_build-cache

# Or manually clean inside container
docker compose run --rm builder rm -rf /workspace/build
```

### Tool Version Mismatches

The container uses specific tool versions defined in `.mise.toml`. If you need different versions:

1. Edit `.mise.toml`
2. Rebuild the Docker image
3. The new versions will be installed

### Permission Issues

Build artifacts created in the container are owned by root. To fix ownership:

```bash
docker compose run --rm builder chown -R $(id -u):$(id -g) /workspace/build
```

## Advanced Usage

### Custom Build Targets

Build a specific target:

```bash
docker run --rm -v $(pwd):/workspace prong-builder:latest \
  bash -c "cd build && ninja prong"
```

### Debugging Build Issues

Run with verbose output:

```bash
docker run --rm -v $(pwd):/workspace prong-builder:latest \
  bash -c "cd build && cmake .. -G Ninja -DCMAKE_VERBOSE_MAKEFILE=ON && ninja -v"
```

### Installing Additional Packages

For temporary experimentation:

```bash
docker compose run --rm builder bash
# Inside container
pacman -Sy --noconfirm <package-name>
```

For permanent additions, modify the Dockerfile and rebuild.

## Integration with Host Git Hooks

To delegate git hooks to the container, create a wrapper script:

```bash
# scripts/git-hooks-docker.sh
#!/bin/bash
set -e

HOOK_NAME=$1
shift

docker run --rm -v $(pwd):/workspace prong-builder:latest \
  bash -c "hk run $HOOK_NAME $@"
```

Then update your `hk.pkl` to use this script when in Docker mode.

## CI/CD Recommendations

### GitHub Actions

```yaml
name: Build

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Set up Docker Buildx
        uses: docker/setup-buildx-action@v3

      - name: Build Docker image
        run: docker build --target builder -t prong-builder:latest .

      - name: Build library
        run: |
          docker run --rm -v $(pwd):/workspace prong-builder:latest \
            bash -c "rm -rf build && mise run build-ci"

      - name: Run tests
        run: |
          docker run --rm -v $(pwd):/workspace prong-builder:latest \
            bash -c "mise run test"
```

### Self-Hosted Runners (BombFork)

For self-hosted runners, consider:

1. Pre-building and caching the Docker image
2. Using local Docker registry for faster pulls
3. Mounting a persistent build cache volume

```bash
# Pre-build and tag
docker build --target builder -t prong-builder:latest .
docker tag prong-builder:latest localhost:5000/prong-builder:latest
docker push localhost:5000/prong-builder:latest

# In CI, pull from local registry
docker pull localhost:5000/prong-builder:latest
```

## Summary

The Docker build system provides:

- Phase 1: Basic build container with all required tools
- Phase 2: Multi-stage optimization with build caching
- Phase 3: CI/CD integration ready with volume mounts and git hook support

All phases are complete and tested. The system is production-ready for use in development and CI/CD workflows.

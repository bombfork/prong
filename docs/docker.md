# Docker Build System

Docker-based build environment for reproducible builds across all development and CI/CD systems.

## Quick Start

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

All mise Docker tasks are in `mise-tasks/` and automatically handle:
- Image names and tags
- Volume mounts
- User ID/GID mapping (prevents permission issues)
- Path resolution

## Why Docker?

- **Reproducible**: All dependencies version-pinned (see `.mise.toml` for tool versions)
- **Isolated**: No interference with host system
- **Modern**: Wayland support, Arch Linux base
- **Convenient**: Files owned by your user, not root

## Architecture

Multi-stage Dockerfile:
1. **toolchain**: Base Arch Linux with C++ compiler
2. **iwyu-builder**: Builds include-what-you-use from AUR
3. **mise-tools**: Installs mise-managed tools
4. **builder**: Final image with all dependencies (library, tests, examples)

Single unified `prong-builder` image includes:
- System packages: clang-format, include-what-you-use
- Mise tools: cmake, ninja, hk, pkl (versions in `.mise.toml`)
- Example deps: glfw-wayland, mesa, libgl, libglvnd

## Permission Handling

The Docker setup prevents root-owned files in your build directory:

- Container has `prong` user (non-root)
- Mise tasks pass your UID/GID: `-e HOST_USER_ID=$(id -u) HOST_GROUP_ID=$(id -g)`
- Entrypoint adjusts `prong` user to match your host user
- Build artifacts owned by you - **no more `sudo rm -rf build`!**

## Manual Usage

If you need to run Docker commands without mise tasks, see the task files in `mise-tasks/` for exact commands. Key points:

- Pass `--build-arg USER_ID=$(id -u) GROUP_ID=$(id -g)` when building
- Pass `-e HOST_USER_ID=$(id -u) HOST_GROUP_ID=$(id -g)` when running
- Mount source: `-v $(pwd):/workspace`

## Running GUI Examples

### Wayland (Linux - Recommended)

```bash
docker run --rm \
  -e HOST_USER_ID=$(id -u) -e HOST_GROUP_ID=$(id -g) \
  -e WAYLAND_DISPLAY=$WAYLAND_DISPLAY \
  -e XDG_RUNTIME_DIR=$XDG_RUNTIME_DIR \
  -v $(pwd):/workspace \
  -v $XDG_RUNTIME_DIR/$WAYLAND_DISPLAY:$XDG_RUNTIME_DIR/$WAYLAND_DISPLAY \
  prong-builder:latest mise run demo
```

### X11 (Fallback)

```bash
xhost +local:docker
docker run --rm \
  -e HOST_USER_ID=$(id -u) -e HOST_GROUP_ID=$(id -g) \
  -e DISPLAY=$DISPLAY \
  -v $(pwd):/workspace \
  -v /tmp/.X11-unix:/tmp/.X11-unix:ro \
  --device /dev/dri:/dev/dri \
  prong-builder:latest mise run demo
xhost -local:docker
```

## Troubleshooting

### Permission Issues

Mise tasks handle this automatically. If running manually, ensure you pass `HOST_USER_ID` and `HOST_GROUP_ID` environment variables.

### Build Cache

```bash
# Force rebuild without cache
docker build --no-cache -t prong-builder:latest .
```

### Tool Versions

Tool versions are defined in `.mise.toml`. To update:
1. Edit `.mise.toml`
2. Rebuild: `mise docker-build`

## CI/CD Integration

Example GitHub Actions:

```yaml
- name: Build and Test
  run: |
    mise docker-build
    mise docker-build-lib
    mise docker-test
```

For self-hosted runners, consider pre-building and caching the image.

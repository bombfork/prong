# Prong C++20 UI Framework - Build Container
# Multi-stage build for optimized container size and build caching
# Base: Arch Linux with multilib support for comprehensive toolchain

# ============================================================================
# Stage 1: Base toolchain installation
# ============================================================================
FROM archlinux:multilib-devel AS toolchain

# Update system and install base build tools
# Combine operations to reduce layers and improve build cache efficiency
RUN pacman -Syu --noconfirm && \
    pacman -S --needed --noconfirm \
        base-devel \
        git \
        curl \
        clang \
        clang-tools-extra \
        wget \
        ca-certificates \
        glibc-locales && \
    pacman -Scc --noconfirm

# Set locale after installing glibc-locales
RUN echo "en_US.UTF-8 UTF-8" > /etc/locale.gen && \
    locale-gen
ENV LANG=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8

# Install mise for tool version management
RUN curl https://mise.run | sh && \
    echo 'eval "$(~/.local/bin/mise activate bash)"' >> ~/.bashrc
ENV PATH="/root/.local/bin:${PATH}"

# Verify mise installation
RUN mise --version

# ============================================================================
# Stage 2: Install include-what-you-use from AUR
# ============================================================================
FROM toolchain AS iwyu-builder

# Create a non-root user for building AUR packages
# makepkg refuses to run as root for security reasons
RUN useradd -m -G wheel -s /bin/bash builduser && \
    echo 'builduser ALL=(ALL) NOPASSWD: ALL' >> /etc/sudoers

# Switch to builduser and build include-what-you-use
USER builduser
WORKDIR /home/builduser

RUN git clone https://aur.archlinux.org/include-what-you-use.git && \
    cd include-what-you-use && \
    makepkg -si --noconfirm && \
    cd .. && \
    rm -rf include-what-you-use

# ============================================================================
# Stage 3: Install mise-managed tools
# ============================================================================
FROM toolchain AS mise-tools

# Copy from iwyu-builder stage to get include-what-you-use
COPY --from=iwyu-builder /usr/bin/include-what-you-use /usr/bin/
COPY --from=iwyu-builder /usr/lib/clang /usr/lib/clang

# Copy mise configuration to install pinned tool versions
COPY .mise.toml /tmp/.mise.toml

# Install mise-managed tools (cmake, ninja, hk, pkl) with caching
RUN --mount=type=cache,target=/root/.cache/mise \
    cd /tmp && \
    mise trust && \
    mise install && \
    eval "$(mise activate bash)" && \
    cmake --version && \
    ninja --version && \
    hk --version && \
    pkl --version

# ============================================================================
# Stage 4: Final build environment (includes all dependencies)
# ============================================================================
FROM toolchain AS builder

# Copy installed tools from previous stages
COPY --from=iwyu-builder /usr/bin/include-what-you-use /usr/bin/
COPY --from=iwyu-builder /usr/lib/clang /usr/lib/clang
COPY --from=mise-tools /root/.local /root/.local

# Install example dependencies (GLFW, OpenGL) in main image
# This allows a single image to build library, tests, and examples
RUN pacman -S --needed --noconfirm \
        glfw-x11 \
        mesa \
        libgl \
        libglvnd && \
    pacman -Scc --noconfirm

# Verify all tools are available
RUN clang-format --version && \
    include-what-you-use --version && \
    mise --version

# Set up working directory for builds
WORKDIR /workspace

# Copy mise configuration for runtime use
COPY .mise.toml /workspace/.mise.toml

# Install project-specific mise tools in workspace context
RUN --mount=type=cache,target=/root/.cache/mise \
    cd /workspace && \
    mise trust && \
    mise install

# Set up entrypoint to ensure mise is available
RUN printf '#!/bin/bash\neval "$(mise activate bash)"\nexec "$@"\n' > /entrypoint.sh && \
    chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]

# Default command runs a shell for interactive use
CMD ["bash"]

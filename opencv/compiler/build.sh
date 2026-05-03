#!/usr/bin/env bash

################################################################################
# OpenCV Build System - Enhanced Version (Bash Compatible)
# Improved with better error handling, organization, and platform support

set -e  # Exit on any error

################################################################################
# Default Configuration
################################################################################

TARGET_INDEX="0"
BUILD_TYPE="release"
BUILD_TARGET_OS=""
BUILD_CMAKE_ARGS=""
TARGET_TOOLCHAIN=""
HOST_OS=""
HOST_ARCH=""
BUILD_NAME=""
BUILD_VERSION=""
INSTALL_PATH="./lib"
BUILD_TARGET_ARCH=""
EXTRA_MODULES_PATH=""

################################################################################
# Utility Functions
################################################################################

log_info() {
    echo "[INFO] $*"
}

log_error() {
    echo "[ERROR] $*" >&2
}

log_success() {
    echo "[SUCCESS] $*"
}

error_exit() {
    log_error "$1"
    exit 1
}

################################################################################
# Platform Detection and Configuration
################################################################################

setup_host_platform() {
    case "$(uname)" in
        Darwin*)
            HOST_OS="mac"
            HOST_ARCH="x86_64"
            ;;
        Linux*)
            HOST_OS="linux"
            HOST_ARCH="x86_64"
            ;;
        *)
            error_exit "Unsupported host platform: $(uname)"
            ;;
    esac
    log_info "Host platform detected: $HOST_OS-$HOST_ARCH"
}

get_target_info() {
    local index=$1

    case "$index" in
        0)
            BUILD_TARGET_OS="$HOST_OS"
            BUILD_TARGET_ARCH="$HOST_ARCH"
            ;;
        1)
            BUILD_TARGET_OS="android"
            BUILD_TARGET_ARCH="armeabi-v7a"
            ;;
        2)
            BUILD_TARGET_OS="android"
            BUILD_TARGET_ARCH="arm64-v8a"
            ;;
        3)
            BUILD_TARGET_OS="android"
            BUILD_TARGET_ARCH="x86"
            ;;
        4)
            BUILD_TARGET_OS="android"
            BUILD_TARGET_ARCH="x86_64"
            ;;
        5)
            BUILD_TARGET_OS="qnx"
            BUILD_TARGET_ARCH="armv7le"
            ;;
        6)
            BUILD_TARGET_OS="qnx"
            BUILD_TARGET_ARCH="aarch64le"
            ;;
        7)
            BUILD_TARGET_OS="ios"
            BUILD_TARGET_ARCH="armv7"
            ;;
        8)
            BUILD_TARGET_OS="ios"
            BUILD_TARGET_ARCH="arm64"
            ;;
        9)
            BUILD_TARGET_OS="ax620a"
            BUILD_TARGET_ARCH="arm7"
            ;;
        *)
            error_exit "Invalid target index: $index"
            ;;
    esac
}

setup_android_config() {
    log_info "Setting up Android configuration for $BUILD_TARGET_ARCH"

    if [ -z "$ANDROID_NDK_HOME" ]; then
        error_exit "ANDROID_NDK_HOME environment variable is not set"
    fi

    TARGET_TOOLCHAIN="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake"
    if [ ! -f "$TARGET_TOOLCHAIN" ]; then
        error_exit "Android NDK toolchain not found at $TARGET_TOOLCHAIN"
    fi

    # Set Android ABI based on target architecture
    local abi_arg=""
    case "$BUILD_TARGET_ARCH" in
        "armeabi-v7a")
            abi_arg="-DANDROID_ABI=armeabi-v7a"
            ;;
        "arm64-v8a")
            abi_arg="-DANDROID_ABI=arm64-v8a"
            ;;
        "x86")
            abi_arg="-DANDROID_ABI=x86"
            ;;
        "x86_64")
            abi_arg="-DANDROID_ABI=x86_64"
            ;;
        *)
            error_exit "Unsupported Android architecture: $BUILD_TARGET_ARCH"
            ;;
    esac

    # Build Android CMake arguments
    local android_args="$abi_arg -DANDROID_PLATFORM=android-21"
    android_args="$android_args -DBUILD_JAVA=OFF"
    android_args="$android_args -DBUILD_ANDROID_PROJECTS=OFF"
    android_args="$android_args -DBUILD_ANDROID_EXAMPLES=OFF"

    # Merge with existing args
    if [ -z "$BUILD_CMAKE_ARGS" ]; then
        BUILD_CMAKE_ARGS="$android_args"
    else
        BUILD_CMAKE_ARGS="$BUILD_CMAKE_ARGS $android_args"
    fi

    log_info "Android CMake args added: $android_args"
}

setup_ios_config() {
    log_info "Setting up iOS configuration"
    TARGET_TOOLCHAIN="$(dirname "$0")/cmake/ios/ios.toolchain.cmake"
    if [ ! -f "$TARGET_TOOLCHAIN" ]; then
        error_exit "iOS toolchain not found at $TARGET_TOOLCHAIN"
    fi
}

setup_qnx_config() {
    log_info "Setting up QNX configuration"

    if [ -z "$QNX_HOST" ] || [ -z "$QNX_TARGET" ]; then
        error_exit "QNX_HOST and QNX_TARGET environment variables must be set"
    fi

    TARGET_TOOLCHAIN="$(dirname "$0")/cmake/arm-unknown-nto-qnx700eabi.toolchain.cmake"
    if [ ! -f "$TARGET_TOOLCHAIN" ]; then
        error_exit "QNX toolchain not found at $TARGET_TOOLCHAIN"
    fi
}

setup_custom_config() {
    case "$BUILD_TARGET_OS" in
        "ax620a")
            log_info "Setting up AX620A configuration"
            TARGET_TOOLCHAIN="/home/baiduiov/03.ProgramSpace/20.AI/01.WorkSpace/opencv-library/cmake/arm-linux-arm7-aixin.toolchain.cmake"
            if [ ! -f "$TARGET_TOOLCHAIN" ]; then
                error_exit "AX620A toolchain not found at $TARGET_TOOLCHAIN"
            fi
            ;;
        *)
            error_exit "Unsupported custom platform: $BUILD_TARGET_OS"
            ;;
    esac
}

load_platform_options() {
    # Only load options files if explicitly specified via command line arguments
    # This allows the calling script to have full control over which options to use

    # Load platform-specific options file if specified
    if [ -n "$PLATFORM_OPTIONS_FILE" ] && [ -f "$PLATFORM_OPTIONS_FILE" ]; then
        log_info "Loading platform-specific CMake options from $PLATFORM_OPTIONS_FILE"
        local platform_options
        platform_options=$(cat "$PLATFORM_OPTIONS_FILE" | grep -v "^#" | tr '\n' ' ')

        if [ -z "$BUILD_CMAKE_ARGS" ]; then
            BUILD_CMAKE_ARGS="$platform_options"
        else
            BUILD_CMAKE_ARGS="$BUILD_CMAKE_ARGS $platform_options"
        fi
    fi

    # Load common options file if specified
    if [ -n "$COMMON_OPTIONS_FILE" ] && [ -f "$COMMON_OPTIONS_FILE" ]; then
        log_info "Loading common CMake options from $COMMON_OPTIONS_FILE"
        local common_options
        common_options=$(cat "$COMMON_OPTIONS_FILE" | grep -v "^#" | tr '\n' ' ')

        if [ -z "$BUILD_CMAKE_ARGS" ]; then
            BUILD_CMAKE_ARGS="$common_options"
        else
            BUILD_CMAKE_ARGS="$BUILD_CMAKE_ARGS $common_options"
        fi
    fi
}

################################################################################
# Build Functions
################################################################################

setup_build_directory() {
    BUILD_DIR="build/$BUILD_TARGET_OS-$BUILD_TARGET_ARCH-$BUILD_TYPE"

    log_info "Creating build directory: $BUILD_DIR"
    if [ ! -d "$BUILD_DIR" ]; then
        mkdir -p "$BUILD_DIR" || error_exit "Failed to create build directory"
    fi
}

run_cmake() {
    log_info "Running CMake configuration"

    # Determine source path before changing directory
    local source_path
    if [ -n "$EXTRA_MODULES_PATH" ]; then
        source_path="../../$EXTRA_MODULES_PATH"
    elif [ -n "$BUILD_NAME" ] && [ -n "$BUILD_VERSION" ]; then
        source_path="../../$BUILD_NAME-$BUILD_VERSION"
    else
        source_path="../../opencv-4.11.0"
    fi

    cd "$BUILD_DIR"
    log_info "From directory: $(pwd)"
    log_info "Source path: $source_path"
    log_info "Source path exists: $(ls -la $source_path 2>/dev/null || echo 'NOT FOUND')"

    # Build CMake command with proper argument handling
    local cmake_args=(
        "-D" "TARGET_OS=$BUILD_TARGET_OS"
        "-D" "TARGET_ARCH=$BUILD_TARGET_ARCH"
        "-D" "HOST_OS=$HOST_OS"
        "-D" "HOST_ARCH=$HOST_ARCH"
        "-D" "CMAKE_BUILD_TYPE=$BUILD_TYPE"
    )

    if [ -n "$TARGET_TOOLCHAIN" ]; then
        cmake_args+=("-D" "CMAKE_TOOLCHAIN_FILE=$TARGET_TOOLCHAIN")
    fi

    cmake_args+=("-D" "CMAKE_INSTALL_PREFIX=$INSTALL_PATH/$BUILD_TARGET_OS-$BUILD_TARGET_ARCH-$BUILD_TYPE")

    if [ -n "$BUILD_CMAKE_ARGS" ]; then
        # Split BUILD_CMAKE_ARGS into individual arguments
        read -ra extra_args <<< "$BUILD_CMAKE_ARGS"
        cmake_args+=("${extra_args[@]}")
    fi

    cmake_args+=("$source_path")

    log_info "Executing: cmake ${cmake_args[*]}"
    cmake "${cmake_args[@]}" || error_exit "CMake configuration failed"

    log_success "CMake configuration completed successfully"
}

run_build() {
    log_info "Starting build process"

    # Get CPU count for optimal parallel build
    local cpu_count
    if [ "$(uname)" = "Darwin" ]; then
        cpu_count=$(sysctl -n hw.ncpu)
    else
        cpu_count=$(nproc 2>/dev/null || echo 2)
    fi

    local build_threads=$((cpu_count > 2 ? cpu_count - 1 : 1))
    log_info "Building with $build_threads threads"

    make -j "$build_threads" || error_exit "Build failed"

    log_success "Build completed successfully"
}

run_install() {
    log_info "Installing built libraries"
    make install || error_exit "Installation failed"
    log_success "Installation completed successfully"
}

build_target() {
    log_info "Starting build for $BUILD_TARGET_OS-$BUILD_TARGET_ARCH ($BUILD_TYPE)"

    setup_build_directory
    run_cmake
    run_build
    run_install

    log_success "Build process completed successfully for $BUILD_TARGET_OS-$BUILD_TARGET_ARCH"
}

################################################################################
# Help and Documentation
################################################################################

show_help() {
    cat << EOF
OpenCV Build System - Enhanced Version

Usage: $0 [OPTIONS...]

Build Target Options:
    -t, --target INDEX     Set build target (default: 0)
        0 - Host platform ($HOST_OS-$HOST_ARCH)
        1 - Android armeabi-v7a
        2 - Android arm64-v8a
        3 - Android x86
        4 - Android x86_64
        5 - QNX armv7le
        6 - QNX aarch64le
        7 - iOS armv7
        8 - iOS arm64
        9 - AX620A arm7

Build Type Options:
    -r, --release          Set build type to Release (default)
    -d, --debug            Set build type to Debug
    --RelWithDebInfo       Set build type to RelWithDebInfo

Configuration Options:
    -n, --name NAME        Set build name (e.g., 'opencv')
    -v, --version VER     Set build version (e.g., '4.11.0')
    -i, --install PATH    Set install path (default: ./lib)
    -e, --extra PATH      Set extra modules path
    -a, --args ARGS       Additional CMake arguments
    --platform-options FILE  Load platform-specific options from file
    --common-options FILE    Load common options from file

Information Options:
    -h, --help             Show this help message
    --list-targets         List all available targets

Examples:
    # Build OpenCV for current platform
    $0

    # Build Android arm64-v8a release version
    $0 -t 2 -r -n opencv -v 4.11.0

    # Build Android armeabi-v7a debug version with custom args
    $0 -t 1 -d -a "-DWITH_CUDA=ON"

EOF
}

show_targets() {
    echo "Available build targets:"
    echo
    echo "  0 - host ($HOST_OS-$HOST_ARCH)"
    echo "  1 - android-armv7a (android-armeabi-v7a)"
    echo "  2 - android-arm64 (android-arm64-v8a)"
    echo "  3 - android-x86 (android-x86)"
    echo "  4 - android-x86_64 (android-x86_64)"
    echo "  5 - qnx-armv7le (qnx-armv7le)"
    echo "  6 - qnx-aarch64 (qnx-aarch64le)"
    echo "  7 - ios-armv7 (ios-armv7)"
    echo "  8 - ios-arm64 (ios-arm64)"
    echo "  9 - ax620a-arm7 (ax620a-arm7)"
    echo
}

################################################################################
# Main Script
################################################################################

main() {
    # Check required commands
    if ! command -v cmake >/dev/null 2>&1; then
        error_exit "cmake is required but not installed"
    fi

    if ! command -v make >/dev/null 2>&1; then
        error_exit "make is required but not installed"
    fi

    setup_host_platform

    # Parse command line arguments
    while [ $# -gt 0 ]; do
        case "$1" in
            -t|--target)
                TARGET_INDEX="$2"
                shift 2
                ;;
            -r|--release)
                BUILD_TYPE="release"
                shift
                ;;
            -d|--debug)
                BUILD_TYPE="debug"
                shift
                ;;
            --RelWithDebInfo)
                BUILD_TYPE="RelWithDebInfo"
                shift
                ;;
            -n|--name)
                BUILD_NAME="$2"
                shift 2
                ;;
            -v|--version)
                BUILD_VERSION="$2"
                shift 2
                ;;
            -i|--install)
                INSTALL_PATH="$2"
                shift 2
                ;;
            -e|--extra)
                EXTRA_MODULES_PATH="$2"
                shift 2
                ;;
            -a|--args)
                BUILD_CMAKE_ARGS="$2"
                shift 2
                ;;
            --platform-options)
                PLATFORM_OPTIONS_FILE="$2"
                shift 2
                ;;
            --common-options)
                COMMON_OPTIONS_FILE="$2"
                shift 2
                ;;
            --list-targets)
                show_targets
                exit 0
                ;;
            -h|--help)
                show_help
                exit 0
                ;;
            *)
                error_exit "Unknown option: $1"
                ;;
        esac
    done

    # Setup build configuration
    get_target_info "$TARGET_INDEX"
    log_info "Target platform: $BUILD_TARGET_OS-$BUILD_TARGET_ARCH"

    # Setup platform-specific configuration
    case "$BUILD_TARGET_OS" in
        "android")
            setup_android_config
            ;;
        "ios")
            setup_ios_config
            ;;
        "qnx")
            setup_qnx_config
            ;;
        "ax620a")
            setup_custom_config
            ;;
        "mac"|"linux")
            # No special configuration needed for host platforms
            ;;
        *)
            error_exit "Unsupported target OS: $BUILD_TARGET_OS"
            ;;
    esac

    # Load platform-specific and common CMake options
    load_platform_options

    # Start build process
    build_target
}

main "$@"
#!/usr/bin/env bash

# ============================================================================
# 🎯 通用 C++ 跨平台构建脚本
# 设计目标: 可复用到任何 C++ 跨平台项目
# 作者: FantasyCXX Team
# 版本: 3.0
# ============================================================================

set -euo pipefail

# 颜色定义 (兼容不同终端)
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo_info() { echo -e "${BLUE}[INFO]${NC} $*"; }
echo_success() { echo -e "${GREEN}[SUCCESS]${NC} $*"; }
echo_warning() { echo -e "${YELLOW}[WARNING]${NC} $*"; }
echo_error() { echo -e "${RED}[ERROR]${NC} $*" >&2; }

# 显示帮助
show_help() {
    cat << EOF
🎯 通用 C++ 跨平台构建脚本 v3.0

用法: $0 [选项] <命令>

命令:
    build      构建项目
    clean      清理构建文件
    help       显示此帮助

选项:
    -t, --target TARGET    指定目标平台 (android|mac|linux|windows|ios)
    -c, --config CONFIG    指定构建配置 (debug|release)
    -j, --jobs NUM         并行构建线程数 (默认: CPU核心数)
    -p, --project DIR      指定项目源码目录 (默认: 当前目录)
    -o, --output DIR       指定输出目录 (默认: lib/{version})
    -h, --help            显示此帮助

示例:
    $0 build -t android -c release
    $0 build -t mac -c debug -j 8
    $0 build -t linux --project ./src --output ./output
    $0 clean

环境变量:
    BUILD_THREADS          并行构建线程数
    VERBOSE                启用详细输出 (true/false)
    ANDROID_NDK_HOME       Android NDK 路径 (Android构建必需)

支持的构建目标:
    android     Android (arm64-v8a)
    mac         macOS (x86_64)
    ios         iOS (arm64)
    linux       Linux (x86_64)
    windows     Windows (x86_64)

📝 注意: 这是一个通用构建脚本，可复用于任何 C++ 项目

EOF
}

# 获取 CPU 核心数
detect_cpu_count() {
    if command -v nproc >/dev/null 2>&1; then
        nproc
    elif command -v sysctl >/dev/null 2>&1; then
        sysctl -n hw.ncpu 2>/dev/null || echo "4"
    else
        echo "4"
    fi
}

# 解析参数
parse_args() {
    COMMAND=""
    TARGET=""
    CONFIG="release"
    JOBS=""
    PROJECT_DIR="."
    OUTPUT_DIR=""

    while [[ $# -gt 0 ]]; do
        case $1 in
            build|clean|help)
                COMMAND="$1"
                shift
                ;;
            -t|--target)
                TARGET="$2"
                shift 2
                ;;
            -c|--config)
                CONFIG="$2"
                shift 2
                ;;
            -j|--jobs)
                JOBS="$2"
                shift 2
                ;;
            -p|--project)
                PROJECT_DIR="$2"
                shift 2
                ;;
            -o|--output)
                OUTPUT_DIR="$2"
                shift 2
                ;;
            -h|--help)
                COMMAND="help"
                shift
                ;;
            *)
                echo_error "未知参数: $1"
                show_help
                exit 1
                ;;
        esac
    done

    # 设置默认值
    if [ -z "$JOBS" ]; then
        JOBS=$(detect_cpu_count)
    fi

    export BUILD_THREADS="$JOBS"
    export BUILD_CONFIG="$CONFIG"
    export PROJECT_SOURCE_DIR="$PROJECT_DIR"
    export OUTPUT_BASE_DIR="${OUTPUT_DIR:-lib}"
}

# 检查环境
check_environment() {
    local target="$1"

    # 检查必需命令
    local required_cmds=("cmake" "make")

    for cmd in "${required_cmds[@]}"; do
        if ! command -v "$cmd" >/dev/null 2>&1; then
            echo_error "必需命令 '$cmd' 未安装"
            exit 1
        fi
    done

    # 检查项目目录
    if [ ! -d "$PROJECT_SOURCE_DIR" ]; then
        echo_error "项目源码目录不存在: $PROJECT_SOURCE_DIR"
        exit 1
    fi

    if [ ! -f "$PROJECT_SOURCE_DIR/CMakeLists.txt" ]; then
        echo_error "项目源码目录中缺少 CMakeLists.txt: $PROJECT_SOURCE_DIR"
        exit 1
    fi

    # 平台特定检查
    case "$target" in
        "android")
            if [ -z "${ANDROID_NDK_HOME:-}" ]; then
                echo_error "Android 构建需要设置 ANDROID_NDK_HOME 环境变量"
                echo_info "示例: export ANDROID_NDK_HOME=/path/to/android-ndk"
                exit 1
            fi
            if [ ! -d "$ANDROID_NDK_HOME" ]; then
                echo_error "Android NDK 路径不存在: $ANDROID_NDK_HOME"
                exit 1
            fi
            ;;
        "ios")
            if ! command -v xcodebuild >/dev/null 2>&1; then
                echo_error "iOS 构建需要安装 Xcode"
                exit 1
            fi
            ;;
        "windows")
            if ! command -v x86_64-w64-mingw32-cmake >/dev/null 2>&1; then
                echo_warning "Windows 交叉编译需要安装 MinGW"
                echo_info "Ubuntu: sudo apt install gcc-mingw-w64-x86-64"
            fi
            ;;
    esac

    echo_success "✅ 环境检查通过 (目标: $target, 线程: $BUILD_THREADS)"
}

# 生成平台标识符
generate_platform_id() {
    local target="$1"
    local config="$2"

    case "$target" in
        "android")
            echo "android-arm64-v8a-${config}"
            ;;
        "mac")
            echo "mac-x86_64-${config}"
            ;;
        "ios")
            echo "ios-arm64-${config}"
            ;;
        "linux")
            echo "linux-x86_64-${config}"
            ;;
        "windows")
            echo "windows-x86_64-${config}"
            ;;
        *)
            echo "${target}-${config}"
            ;;
    esac
}

# 构建项目
build_project() {
    local target="$1"
    local config="$2"

    echo_info "🏗️  构建项目: 目标=$target, 配置=$config, 线程=$BUILD_THREADS"

    local platform_id
    platform_id=$(generate_platform_id "$target" "$config")
    local build_dir="build/${platform_id}"
    local install_dir="$OUTPUT_BASE_DIR/${platform_id}"

    # 创建目录
    mkdir -p "$build_dir"
    mkdir -p "$install_dir"

    # 根据目标平台配置构建
    case "$target" in
        "android")
            build_android "$build_dir" "$install_dir" "$config"
            ;;
        "mac")
            build_macos "$build_dir" "$install_dir" "$config"
            ;;
        "ios")
            build_ios "$build_dir" "$install_dir" "$config"
            ;;
        "linux")
            build_linux "$build_dir" "$install_dir" "$config"
            ;;
        "windows")
            build_windows "$build_dir" "$install_dir" "$config"
            ;;
        *)
            echo_error "不支持的目标平台: $target"
            exit 1
            ;;
    esac

    echo_success "✅ 构建完成: $install_dir"
    echo_info "📦 输出文件位于: $install_dir/"
}

# Android 构建
build_android() {
    local build_dir="$1"
    local install_dir="$2"
    local config="$3"

    cd "$build_dir"

    cmake "$PROJECT_SOURCE_DIR" \
        -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" \
        -DANDROID_ABI=arm64-v8a \
        -DANDROID_PLATFORM=android-24 \
        -DANDROID_STL=c++_shared \
        -DCMAKE_BUILD_TYPE="$config" \
        -DCMAKE_INSTALL_PREFIX="../../../$install_dir" \
        -DBUILD_SHARED_LIBS=ON

    make -j"$BUILD_THREADS"
    make install

    cd ../../..
}

# macOS 构建
build_macos() {
    local build_dir="$1"
    local install_dir="$2"
    local config="$3"

    cd "$build_dir"

    cmake "$PROJECT_SOURCE_DIR" \
        -DCMAKE_BUILD_TYPE="$config" \
        -DCMAKE_INSTALL_PREFIX="../../../$install_dir" \
        -DBUILD_SHARED_LIBS=ON

    make -j"$BUILD_THREADS"
    make install

    cd ../../..
}

# iOS 构建
build_ios() {
    local build_dir="$1"
    local install_dir="$2"
    local config="$3"

    cd "$build_dir"

    # 简化的 iOS 构建配置，实际使用时可能需要更复杂的 toolchain
    cmake "$PROJECT_SOURCE_DIR" \
        -DCMAKE_TOOLCHAIN_FILE="$PROJECT_SOURCE_DIR/cmake/ios.toolchain.cmake" \
        -DIOS_PLATFORM=OS \
        -DENABLE_BITCODE=ON \
        -DCMAKE_BUILD_TYPE="$config" \
        -DCMAKE_INSTALL_PREFIX="../../../$install_dir" \
        -DBUILD_SHARED_LIBS=OFF

    make -j"$BUILD_THREADS"
    make install

    cd ../../..
}

# Linux 构建
build_linux() {
    local build_dir="$1"
    local install_dir="$2"
    local config="$3"

    cd "$build_dir"

    cmake "$PROJECT_SOURCE_DIR" \
        -DCMAKE_BUILD_TYPE="$config" \
        -DCMAKE_INSTALL_PREFIX="../../../$install_dir" \
        -DBUILD_SHARED_LIBS=ON

    make -j"$BUILD_THREADS"
    make install

    cd ../../..
}

# Windows 构建
build_windows() {
    local build_dir="$1"
    local install_dir="$2"
    local config="$3"

    cd "$build_dir"

    if command -v x86_64-w64-mingw32-cmake >/dev/null 2>&1; then
        x86_64-w64-mingw32-cmake "$PROJECT_SOURCE_DIR" \
            -DCMAKE_BUILD_TYPE="$config" \
            -DCMAKE_INSTALL_PREFIX="../../../$install_dir" \
            -DBUILD_SHARED_LIBS=ON
    else
        cmake "$PROJECT_SOURCE_DIR" \
            -DCMAKE_BUILD_TYPE="$config" \
            -DCMAKE_INSTALL_PREFIX="../../../$install_dir" \
            -DBUILD_SHARED_LIBS=ON
    fi

    make -j"$BUILD_THREADS"
    make install

    cd ../../..
}

# 清理构建
clean_build() {
    echo_info "🧹 清理构建文件..."

    rm -rf build/
    rm -rf "${OUTPUT_BASE_DIR:-lib}"/

    # 清理临时文件
    find . -name "*.o" -delete 2>/dev/null || true
    find . -name "*.a" -delete 2>/dev/null || true
    find . -name "*.so" -delete 2>/dev/null || true
    find . -name "*.dylib" -delete 2>/dev/null || true
    find . -name "*.dll" -delete 2>/dev/null || true

    echo_success "✅ 清理完成"
}

# 主函数
main() {
    parse_args "$@"

    case "$COMMAND" in
        "build")
            if [ -z "$TARGET" ]; then
                echo_error "构建需要指定目标平台 (-t|--target)"
                show_help
                exit 1
            fi
            check_environment "$TARGET"
            build_project "$TARGET" "$CONFIG"
            ;;
        "clean")
            clean_build
            ;;
        "help"|"--help"|"-h"|"")
            show_help
            ;;
        *)
            echo_error "未知命令: $COMMAND"
            show_help
            exit 1
            ;;
    esac
}

# 运行主函数
main "$@"
#!/usr/bin/env bash

# ============================================================================
# 🎯 OpenCV 专用构建脚本
# 位置: 3rdparty/opencv/compiler/build-opencv.sh
# 功能: 专门用于构建 OpenCV 库，支持自动下载源码和多平台构建
# 作者: FantasyCXX Team
# 版本: 3.0
# ============================================================================

set -euo pipefail

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo_info() { echo -e "${BLUE}[INFO]${NC} $*"; }
echo_success() { echo -e "${GREEN}[SUCCESS]${NC} $*"; }
echo_warning() { echo -e "${YELLOW}[WARNING]${NC} $*"; }
echo_error() { echo -e "${RED}[ERROR]${NC} $*" >&2; }

# OpenCV 配置
OPENCV_VERSION="4.11.0"
OPENCV_SRC_DIR="src/opencv-${OPENCV_VERSION}"
OPENCV_CONTRIB_SRC_DIR="src/opencv_contrib-${OPENCV_VERSION}"
OPENCV_LIBS_DIR="../lib/v${OPENCV_VERSION}"
OPENCV_DOWNLOAD_URL="https://github.com/opencv/opencv/archive/${OPENCV_VERSION}.zip"
OPENCV_CONTRIB_DOWNLOAD_URL="https://github.com/opencv/opencv_contrib/archive/${OPENCV_VERSION}.zip"

# 显示帮助
show_help() {
    cat << EOF
🎯 OpenCV 专用构建脚本 v3.0

用法: $0 [选项] <命令>

命令:
    download   📥 下载 OpenCV 源码
    android    🤖 构建 Android ARM64
    mac        🍎 构建 macOS
    ios        📱 构建 iOS
    linux      🐧 构建 Linux
    windows    🪟 构建 Windows
    all        🌐 构建所有平台
    clean      🧹 清理构建文件
    help       📖 显示此帮助

选项:
    -c, --config CONFIG    构建配置 (debug|release, 默认: release)
    -j, --jobs NUM         并行构建线程数
    --with-cuda            启用 CUDA 支持 (如果可用)
    --with-contrib         启用 opencv_contrib 模块
    --minimal              最小化构建 (仅核心模块)
    --skip-download        跳过源码下载

示例:
    $0 download                    # 仅下载源码
    $0 android                     # 构建 Android 发布版
    $0 mac -c debug               # 构建 macOS 调试版
    $0 all --minimal              # 构建所有平台的最小版本
    $0 android --with-contrib     # 构建包含 contrib 的 Android 版本
    $0 clean                      # 清理所有构建文件

环境变量:
    ANDROID_NDK_HOME          Android 构建必需
    BUILD_THREADS             并行构建线程数
    OPENCV_CUDA_ARCH_BIN      CUDA 架构版本 (如: 7.5)

💡 提示: 首次使用前请设置 ANDROID_NDK_HOME

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

# 检查命令是否存在
check_command() {
    local cmd="$1"
    if ! command -v "$cmd" >/dev/null 2>&1; then
        echo_error "必需命令 '$cmd' 未安装"
        return 1
    fi
    return 0
}

# 下载文件
download_file() {
    local url="$1"
    local output="$2"
    local description="$3"

    echo_info "📥 下载 ${description}..."
    echo_info "    URL: $url"
    echo_info "    输出: $output"

    if command -v wget >/dev/null 2>&1; then
        wget -O "$output" "$url" || {
            echo_error "下载失败: $url"
            return 1
        }
    elif command -v curl >/dev/null 2>&1; then
        curl -L -o "$output" "$url" || {
            echo_error "下载失败: $url"
            return 1
        }
    else
        echo_error "需要安装 wget 或 curl 来下载文件"
        return 1
    fi

    echo_success "✅ 下载完成: $output"
    return 0
}

# 解压文件
extract_archive() {
    local archive="$1"
    local description="$2"

    echo_info "📦 解压 ${description}..."

    if [[ "$archive" == *.zip ]]; then
        if ! command -v unzip >/dev/null 2>&1; then
            echo_error "需要安装 unzip 来解压 ZIP 文件"
            return 1
        fi
        unzip -q "$archive" || {
            echo_error "解压失败: $archive"
            return 1
        }
    elif [[ "$archive" == *.tar.gz ]]; then
        tar -xzf "$archive" || {
            echo_error "解压失败: $archive"
            return 1
        }
    else
        echo_error "不支持的压缩格式: $archive"
        return 1
    fi

    echo_success "✅ 解压完成"
    return 0
}

# 下载 OpenCV 源码
download_opencv_source() {
    local skip_download="${1:-false}"

    if [ "$skip_download" = "true" ]; then
        echo_info "⏭️  跳过源码下载"
        return 0
    fi

    # 检查是否已存在源码
    if [ -d "$OPENCV_SRC_DIR" ]; then
        echo_info "📁 OpenCV 源码已存在，跳过下载"
    else
        echo_info "📥 开始下载 OpenCV ${OPENCV_VERSION} 源码..."

        # 检查必要命令
        check_command "wget" || check_command "curl" || exit 1
        check_command "unzip" || exit 1

        # 下载主源码
        local opencv_zip="opencv-${OPENCV_VERSION}.zip"
        download_file "$OPENCV_DOWNLOAD_URL" "$opencv_zip" "OpenCV 主源码" || exit 1
        extract_archive "$opencv_zip" "OpenCV 主源码" || exit 1
        rm -f "$opencv_zip"
    fi

    # 检查是否需要下载 contrib
    if [ "${WITH_CONTRIB:-false}" = "true" ]; then
        if [ -d "$OPENCV_CONTRIB_SRC_DIR" ]; then
            echo_info "📁 OpenCV contrib 源码已存在，跳过下载"
        else
            echo_info "📥 开始下载 OpenCV contrib ${OPENCV_VERSION} 源码..."

            local contrib_zip="opencv_contrib-${OPENCV_VERSION}.zip"
            download_file "$OPENCV_CONTRIB_DOWNLOAD_URL" "$contrib_zip" "OpenCV contrib" || exit 1
            extract_archive "$contrib_zip" "OpenCV contrib" || exit 1
            rm -f "$contrib_zip"
        fi
    fi

    echo_success "✅ OpenCV 源码准备完成"
}

# 检查 OpenCV 源码
check_opencv_source() {
    if [ ! -d "$OPENCV_SRC_DIR" ]; then
        echo_error "OpenCV 源码目录不存在: $OPENCV_SRC_DIR"
        echo_info "请先运行: $0 download"
        exit 1
    fi

    if [ ! -f "$OPENCV_SRC_DIR/CMakeLists.txt" ]; then
        echo_error "OpenCV 源码不完整，缺少 CMakeLists.txt"
        exit 1
    fi

    echo_success "✅ OpenCV 源码检查通过"
}

# 检查环境
check_environment() {
    local target="$1"

    # 检查通用工具
    check_command "cmake" || exit 1
    check_command "make" || exit 1

    # 检查 OpenCV 源码
    check_opencv_source

    # 设置并行构建线程数
    if [ -z "${BUILD_THREADS:-}" ]; then
        export BUILD_THREADS=$(detect_cpu_count)
    fi

    # 平台特定检查
    case "$target" in
        "android"|"all")
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
        "ios"|"all")
            if ! command -v xcodebuild >/dev/null 2>&1; then
                echo_error "iOS 构建需要安装 Xcode"
                exit 1
            fi
            ;;
    esac

    echo_success "✅ 环境检查通过 (线程: $BUILD_THREADS)"
}

# 清理构建
clean_build() {
    echo_info "🧹 清理 OpenCV 构建文件..."

    # 清理构建目录
    rm -rf build/
    rm -rf "$OPENCV_LIBS_DIR"/

    # 清理 OpenCV 源码中的构建文件
    if [ -d "$OPENCV_SRC_DIR/build" ]; then
        rm -rf "$OPENCV_SRC_DIR/build"
    fi

    # 清理临时文件
    find . -name "*.tmp" -delete 2>/dev/null || true
    find . -name "*.log" -delete 2>/dev/null || true

    echo_success "✅ 清理完成"
}

# 生成 OpenCV 构建选项
generate_cmake_options() {
    local target="$1"
    local config="$2"
    local minimal="$3"
    local with_contrib="$4"

    local cmake_opts=(
        # 基础配置
        "-DCMAKE_BUILD_TYPE=$config"
        "-DBUILD_SHARED_LIBS=OFF"
        "-DBUILD_opencv_world=ON"
        "-DWITH_OPENMP=ON"
        "-DBUILD_TIFF=ON"
        "-DWITH_FFMPEG=OFF"
        "-DWITH_GSTREAMER=OFF"
        "-DWITH_CUDA=${WITH_CUDA:-OFF}"
        "-DBUILD_TESTS=OFF"
        "-DBUILD_PERF_TESTS=OFF"
        "-DBUILD_EXAMPLES=OFF"
        "-DBUILD_opencv_apps=OFF"
        "-DINSTALL_C_EXAMPLES=OFF"
        "-DINSTALL_PYTHON_EXAMPLES=OFF"
        "-DENABLE_PRECOMPILED_HEADERS=OFF"
        "-DENABLE_CCACHE=ON"
        # 禁用 Android 相关构建
        "-DBUILD_ANDROID_PROJECTS=OFF"
        "-DBUILD_ANDROID_EXAMPLES=OFF"
        "-DBUILD_JAVA=OFF"
        "-DBUILD_FAT_JAVA_LIB=OFF"
    )

    # 最小化构建
    if [ "$minimal" = "true" ]; then
        cmake_opts+=(
            "-DBUILD_opencv_dnn=OFF"
            "-DBUILD_opencv_ml=OFF"
            "-DBUILD_opencv_photo=OFF"
            "-DBUILD_opencv_video=OFF"
            "-DBUILD_opencv_videoio=OFF"
            "-DBUILD_opencv_highgui=OFF"
            "-DBUILD_opencv_imgcodecs=OFF"
            "-DBUILD_opencv_objdetect=OFF"
            "-DBUILD_opencv_features2d=OFF"
            "-DBUILD_opencv_flann=OFF"
            "-DBUILD_opencv_calib3d=OFF"
        )
    fi

    # 平台特定配置
    case "$target" in
        "android")
            cmake_opts+=(
                "-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake"
                "-DANDROID_ABI=arm64-v8a"
                "-DANDROID_PLATFORM=android-24"
                "-DANDROID_STL=c++_shared"
                "-DWITH_JPEG=ON"
                "-DBUILD_JPEG=ON"
                "-DWITH_PNG=ON"
                "-DBUILD_PNG=ON"
            )
            ;;
        "mac"|"linux")
            cmake_opts+=(
                "-DWITH_JPEG=ON"
                "-DWITH_PNG=ON"
                "-DWITH_TIFF=ON"
            )
            ;;
        "ios")
            cmake_opts+=(
                "-DCMAKE_TOOLCHAIN_FILE=$OPENCV_SRC_DIR/platforms/ios/cmake/Toolchains/Toolchain-iPhoneOS_Xcode.cmake"
                "-DCMAKE_C_FLAGS=-fembed-bitcode"
                "-DCMAKE_CXX_FLAGS=-fembed-bitcode"
                "-DBUILD_SHARED_LIBS=OFF"
                "-DIOS_BITCODE=ON"
            )
            ;;
    esac

    # opencv_contrib 支持
    if [ "$with_contrib" = "true" ] && [ -d "$OPENCV_CONTRIB_SRC_DIR" ]; then
        cmake_opts+=(
            "-DOPENCV_EXTRA_MODULES_PATH=../$OPENCV_CONTRIB_SRC_DIR/modules"
        )
    fi

    # CUDA 特定配置
    if [ "${WITH_CUDA:-OFF}" = "ON" ]; then
        cmake_opts+=(
            "-DWITH_CUDA=ON"
        )
        if [ -n "${OPENCV_CUDA_ARCH_BIN:-}" ]; then
            cmake_opts+=(
                "-DOPENCV_CUDA_ARCH_BIN=${OPENCV_CUDA_ARCH_BIN}"
            )
        fi
    fi

    echo "${cmake_opts[@]}"
}

# 构建 OpenCV
build_opencv() {
    local target="$1"
    local config="${2:-release}"
    local minimal="${3:-false}"
    local with_contrib="${4:-false}"

    echo_info "🏗️  构建 OpenCV ${OPENCV_VERSION} ($target-$config)"

    local platform_id
    case "$target" in
        "android")
            platform_id="android-arm64-v8a-${config}"
            ;;
        "mac")
            platform_id="mac-x86_64-${config}"
            ;;
        "ios")
            platform_id="ios-arm64-${config}"
            ;;
        "linux")
            platform_id="linux-x86_64-${config}"
            ;;
        "windows")
            platform_id="windows-x86_64-${config}"
            ;;
        *)
            platform_id="${target}-${config}"
            ;;
    esac

    local build_dir="build/${platform_id}"
    local install_dir="$OPENCV_LIBS_DIR/${platform_id}"

    # 创建目录
    mkdir -p "$build_dir"
    mkdir -p "$install_dir"

    # 生成 CMake 选项
    local cmake_opts
    cmake_opts=$(generate_cmake_options "$target" "$config" "$minimal" "$with_contrib")

    cd "$build_dir"

    echo_info "📝 配置 OpenCV..."
    if [ "${VERBOSE:-false}" = "true" ]; then
        echo_info "CMake 选项: $cmake_opts"
    fi

    cmake "../../$OPENCV_SRC_DIR" \
        -DCMAKE_INSTALL_PREFIX="../../$install_dir" \
        $cmake_opts

    echo_info "🔨 编译 OpenCV..."
    make -j"$BUILD_THREADS"

    echo_info "📦 安装 OpenCV..."
    make install

    cd ../..

    echo_success "✅ OpenCV 构建完成: $install_dir"
    show_usage "$target" "$config"
}

# 显示使用方式
show_usage() {
    local target="$1"
    local config="$2"

    echo
    echo_info "📁 构建结果: $OPENCV_LIBS_DIR/${target}-${config}/"
    echo_info "🔗 CMake 配置示例:"
    echo "   set(OpenCV_DIR \"\${PROJECT_SOURCE_DIR}/3rdparty/opencv/$OPENCV_LIBS_DIR/${target}-${config}/share/opencv4\")"
    echo "   find_package(OpenCV REQUIRED)"
    echo "   target_link_libraries(your_target \${OpenCV_LIBS})"
}

# 主函数
main() {
    local command=""
    local config="release"
    local minimal="false"
    local with_contrib="false"
    local jobs=""
    local skip_download="false"

    # 解析参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            download|android|mac|ios|linux|windows|all|clean|help)
                command="$1"
                shift
                ;;
            -c|--config)
                config="$2"
                shift 2
                ;;
            -j|--jobs)
                jobs="$2"
                export BUILD_THREADS="$2"
                shift 2
                ;;
            --minimal)
                minimal="true"
                shift
                ;;
            --with-contrib)
                with_contrib="true"
                shift
                ;;
            --with-cuda)
                WITH_CUDA="ON"
                shift
                ;;
            --skip-download)
                skip_download="true"
                shift
                ;;
            *)
                echo_error "未知参数: $1"
                show_help
                exit 1
                ;;
        esac
    done

    case "$command" in
        "help"|"--help"|"-h"|"")
            show_help
            ;;
        "download")
            download_opencv_source "false"
            ;;
        "clean")
            clean_build
            ;;
        "android")
            download_opencv_source "$skip_download"
            check_environment "android"
            build_opencv "android" "$config" "$minimal" "$with_contrib"
            ;;
        "mac")
            download_opencv_source "$skip_download"
            check_environment "mac"
            build_opencv "mac" "$config" "$minimal" "$with_contrib"
            ;;
        "ios")
            download_opencv_source "$skip_download"
            check_environment "ios"
            build_opencv "ios" "$config" "$minimal" "$with_contrib"
            ;;
        "linux")
            download_opencv_source "$skip_download"
            check_environment "linux"
            build_opencv "linux" "$config" "$minimal" "$with_contrib"
            ;;
        "windows")
            download_opencv_source "$skip_download"
            check_environment "windows"
            build_opencv "windows" "$config" "$minimal" "$with_contrib"
            ;;
        "all")
            download_opencv_source "$skip_download"
            check_environment "all"
            build_opencv "android" "$config" "$minimal" "$with_contrib"
            build_opencv "mac" "$config" "$minimal" "$with_contrib"
            build_opencv "linux" "$config" "$minimal" "$with_contrib"
            echo_success "🎉 所有平台构建完成!"
            ;;
        *)
            echo_error "未知命令: $command"
            show_help
            exit 1
            ;;
    esac
}

# 运行主函数
main "$@"
#!/bin/bash

# ============================================================================
# 🎯 OpenCV 统一构建脚本
# 功能: 构建 OpenCV 并自动统一安装目录结构
# 作者: FantasyCXX Team
# 版本: 1.0
# ============================================================================

set -e

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

# 显示帮助
show_help() {
    cat << EOF
🎯 OpenCV 统一构建脚本 v1.0

用法: $0 <平台> [选项]

平台:
    android    🤖 构建 Android ARM64 (自动统一结构)
    mac        🍎 构建 macOS (自动统一结构)
    all        🌐 构建所有平台并统一结构

选项:
    -c, --config CONFIG    构建配置 (debug|release, 默认: release)
    -j, --jobs NUM         并行构建线程数
    --with-contrib         启用 opencv_contrib 模块
    --minimal              最小化构建 (仅核心模块)
    --skip-unify          跳过结构统一步骤

示例:
    $0 android                     # 构建 Android 并统一结构
    $0 mac --minimal              # 构建最小化 macOS 版本
    $0 all                        # 构建所有平台并统一结构

💡 说明:
    - 自动调用 build-opencv.sh 进行构建
    - 自动统一安装目录结构
    - 便于嵌入式跨平台构建配置

EOF
}

# 检测是否需要统一 Android 结构
need_android_unify() {
    local android_dir="$1"

    # 检查是否存在 SDK 结构（v4.11+ 的特征）
    if [ -d "$android_dir/sdk" ]; then
        return 0  # 需要统一
    else
        return 1  # 不需要统一
    fi
}

# 统一 Android 结构
unify_android() {
    local android_dir="$1"

    echo_info "🔧 统一 Android 安装结构..."

    if [ ! -d "$android_dir" ]; then
        echo_warning "Android 安装目录不存在，跳过结构统一"
        return 0
    fi

    # 检查是否需要统一
    if ! need_android_unify "$android_dir"; then
        echo_info "✅ Android 结构已经是统一的 (可能是 v2.4 或 v4.9 及更早版本)"
        return 0
    fi

    # 创建统一目录
    mkdir -p "$android_dir/include/opencv4"
    mkdir -p "$android_dir/lib/opencv4/3rdparty"
    mkdir -p "$android_dir/share/opencv4"

    # 移动文件
    if [ -d "$android_dir/sdk/native/jni/include/opencv2" ]; then
        cp -r "$android_dir/sdk/native/jni/include/opencv2" "$android_dir/include/opencv4/"
    fi

    if [ -d "$android_dir/sdk/native/staticlibs/arm64-v8a" ]; then
        cp "$android_dir/sdk/native/staticlibs/arm64-v8a"/*.a "$android_dir/lib/" 2>/dev/null || true
    fi

    if [ -d "$android_dir/sdk/native/3rdparty/libs/arm64-v8a" ]; then
        cp "$android_dir/sdk/native/3rdparty/libs/arm64-v8a"/*.a "$android_dir/lib/opencv4/3rdparty/" 2>/dev/null || true
    fi

    # 创建 CMake 配置
    cat > "$android_dir/share/opencv4/OpenCVConfig.cmake" << 'EOF'
# OpenCV CMake 配置文件
set(OPENCV_VERSION "4.11.0")
set(OPENCV_LIB_DIR "${CMAKE_CURRENT_LIST_DIR}/../../lib")
set(OPENCV_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../include")
set(OPENCV_3RDPARTY_LIB_DIR "${CMAKE_CURRENT_LIST_DIR}/../../lib/opencv4/3rdparty")
set(OPENCV_LIBS opencv_world)
include_directories(${OPENCV_INCLUDE_DIR})
link_directories(${OPENCV_LIB_DIR})
if(EXISTS "${OPENCV_3RDPARTY_LIB_DIR}")
    link_directories(${OPENCV_3RDPARTY_LIB_DIR})
endif()
if(ANDROID)
    set(OPENCV_PLATFORM_LIBS log)
endif()
set(OpenCV_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(OpenCV_FOUND TRUE)
EOF

    echo_success "✅ Android 结构统一完成"
}

# 构建并统一
build_and_unify() {
    local target="$1"
    local config="${2:-release}"
    local minimal="${3:-false}"
    local with_contrib="${4:-false}"
    local skip_unify="${5:-false}"

    echo_info "🏗️  构建 OpenCV for $target ($config)..."

    # 构建参数
    local build_args=()
    if [ "$minimal" = "true" ]; then
        build_args+=(--minimal)
    fi
    if [ "$with_contrib" = "true" ]; then
        build_args+=(--with-contrib)
    fi

    # 执行构建
    if [ -n "${BUILD_THREADS:-}" ]; then
        ./build-opencv.sh "$target" -c "$config" -j "$BUILD_THREADS" "${build_args[@]}"
    else
        ./build-opencv.sh "$target" -c "$config" "${build_args[@]}"
    fi

    # 统一结构
    if [ "$skip_unify" != "true" ]; then
        case "$target" in
            "android")
                local android_dir="../lib/v4.11.0/android-arm64-v8a-$config"
                unify_android "$android_dir"
                ;;
            "mac")
                echo_info "✅ macOS 结构已经是统一的"
                ;;
        esac
    fi

    echo_success "🎉 $target 构建和统一完成"
}

# 主函数
main() {
    local target=""
    local config="release"
    local minimal="false"
    local with_contrib="false"
    local skip_unify="false"
    local jobs=""

    # 解析参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            android|mac|all)
                target="$1"
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
            --skip-unify)
                skip_unify="true"
                shift
                ;;
            help|-h|--help)
                show_help
                exit 0
                ;;
            *)
                echo_error "未知参数: $1"
                show_help
                exit 1
                ;;
        esac
    done

    if [ -z "$target" ]; then
        echo_error "必须指定构建平台"
        show_help
        exit 1
    fi

    # 执行构建
    case "$target" in
        "android")
            build_and_unify "android" "$config" "$minimal" "$with_contrib" "$skip_unify"
            ;;
        "mac")
            build_and_unify "mac" "$config" "$minimal" "$with_contrib" "$skip_unify"
            ;;
        "all")
            echo_info "🌐 构建所有平台..."
            build_and_unify "mac" "$config" "$minimal" "$with_contrib" "$skip_unify"
            build_and_unify "android" "$config" "$minimal" "$with_contrib" "$skip_unify"
            echo_success "🎉 所有平台构建完成！"
            ;;
    esac
}

# 运行主函数
main "$@"
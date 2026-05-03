# OpenCV Build System - Quick Start Guide

This guide provides quick instructions for building OpenCV for different platforms.

## Prerequisites

### Required Tools
- CMake 3.10 or higher
- GNU Make or equivalent
- Git (for source management)

### Platform-Specific Requirements

#### Android
- Android NDK (set `ANDROID_NDK_HOME` environment variable)
- Minimum API level 21 (Android 5.0)

#### iOS
- Xcode with iOS SDK
- iOS toolchain file

#### QNX
- QNX SDK (set `QNX_HOST` and `QNX_TARGET` environment variables)

## Quick Build Commands

### 1. Build for Current Platform (macOS/Linux)
```bash
cd compiler
./build.sh
```

### 2. Build Android ARM64 (arm64-v8a)
```bash
cd compiler
./build.sh -t 2 -r -n opencv -v 4.11.0
```

### 3. Build Android ARMv7 (armeabi-v7a)
```bash
cd compiler
./build.sh -t 1 -d -n opencv -v 4.11.0
```

### 4. Build iOS ARM64
```bash
cd compiler
./build.sh -t 8 -r -n opencv -v 4.11.0
```

### 5. Build with Custom Install Path
```bash
cd compiler
./build.sh -t 2 -r -i /opt/opencv/android
```

## Target Index Reference

| Index | Platform | Architecture | Description |
|-------|----------|--------------|-------------|
| 0 | Host | Auto | Current platform (macOS/Linux) |
| 1 | Android | armeabi-v7a | Android ARMv7 |
| 2 | Android | arm64-v8a | Android ARM64 |
| 3 | Android | x86 | Android x86 |
| 4 | Android | x86_64 | Android x86_64 |
| 5 | QNX | armv7le | QNX ARMv7 |
| 6 | QNX | aarch64le | QNX ARM64 |
| 7 | iOS | armv7 | iOS ARMv7 |
| 8 | iOS | arm64 | iOS ARM64 |
| 9 | AX620A | arm7 | Custom embedded platform |

## Build Types

- `-r, --release`: Release build (optimized, default)
- `-d, --debug`: Debug build (with debug symbols)
- `--RelWithDebInfo`: Release with debug information

## Common Options

```bash
# Show help and all options
./build.sh --help

# List all available targets
./build.sh --list-targets

# Build with additional CMake arguments
./build.sh -t 2 -r -a "-DWITH_CUDA=ON -DBUILD_opencv_dnn=ON"

# Build debug version with verbose output
./build.sh -t 2 -d -n opencv -v 4.11.0
```

## Output Locations

### Build Files (Intermediate)
```
build/<platform>-<arch>-<type>/
# Example: build/android-arm64-v8a-release/
```

### Final Libraries
```
lib/<version>/<platform>-<arch>-<type>/
# Example: lib/v4.11.0/android-arm64-v8a-release/
```

## Environment Variables

### Android
```bash
export ANDROID_NDK_HOME="/path/to/android-ndk"
```

### QNX
```bash
export QNX_HOST="/path/to/qnx/host"
export QNX_TARGET="/path/to/qnx/target"
```

## Troubleshooting

### Common Issues

1. **CMake not found**: Install CMake 3.10+
2. **Android NDK not set**: Set `ANDROID_NDK_HOME` environment variable
3. **Build directory exists**: Remove or backup existing build directory
4. **Permission denied**: Ensure scripts are executable (`chmod +x build.sh`)

### Getting Help

```bash
# Show detailed help
./build.sh --help

# Check available targets
./build.sh --list-targets

# Check logs for errors
tail -f build.log
```

## Integration with Projects

### CMake Integration
Use the `opencv.cmake` file in the root directory to integrate with your CMake project:

```cmake
include(/path/to/opencv/opencv.cmake)
```

### Manual Integration
Link against the built libraries in the `lib/` directory:

```cmake
target_link_libraries(your_target
    ${OPENCV_LIB_DIR}/libopencv_core.so
    ${OPENCV_LIB_DIR}/libopencv_imgproc.so
    # ... other OpenCV libraries
)
```

## Performance Tips

1. **Parallel Build**: The script automatically uses optimal number of threads
2. **Clean Builds**: Remove build directory for clean rebuilds
3. **Incremental Builds**: Re-running the script will do incremental builds
4. **Disk Space**: Build directories can be large (500MB-2GB per platform)

## Next Steps

1. Read the detailed [Android Build Guide](compiler/docs/Android-ARM64-Build-Guide.md)
2. Check [Troubleshooting Guide](compiler/docs/TROUBLESHOOTING.md) for common issues
3. Review [Compilation Guide](compiler/docs/compilation_guide.md) for advanced options
4. Explore [Examples](compiler/docs/examples/) for specific use cases
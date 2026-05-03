# OpenCV Build System - Project Structure

This document describes the optimized project structure for the OpenCV build system.

## Directory Structure

```
/Users/frewen/01.WorkSpace/AuraKaleidos/FantasyCXX/3rdparty/opencv/
├── README.md                           # Main project README
├── PROJECT_STRUCTURE.md                 # This file
├── opencv.cmake                         # CMake configuration for integration
│
├── build/                               # Build output directory (generated)
│   ├── android-arm64-v8a-release/      # Platform-specific build outputs
│   ├── android-armeabi-v7a-debug/
│   ├── mac-x86_64-release/
│   └── ...
│
├── compiler/                           # Build system directory
│   ├── build.sh                         # Main build script (optimized)
│   ├── build.sh.original                # Original backup
│   │
│   ├── cmake/                          # Toolchain configurations
│   │   ├── ios/                         # iOS toolchain
│   │   │   └── ios.toolchain.cmake
│   │   ├── arm-unknown-nto-qnx700eabi.toolchain.cmake
│   │   └── arm-linux-arm7-aixin.toolchain.cmake
│   │
│   ├── cmake_args/                     # Platform-specific CMake configurations
│   │   ├── android_options.txt          # Android-specific options
│   │   ├── host_options.txt             # macOS/Linux options
│   │   ├── embedded_options.txt         # Embedded platforms (QNX, AX620A, iOS)
│   │   └── common_options.txt           # Common options for all platforms
│   │
│   ├── docs/                          # Documentation and examples
│   │   ├── Android-ARM64-Build-Guide.md
│   │   ├── DOCUMENTATION-INDEX.md
│   │   ├── QUICK-START.md
│   │   ├── TROUBLESHOOTING.md
│   │   ├── FILE_LIST.md
│   │   ├── OpenCV_Build_Documentation_zh.md
│   │   ├── README.md
│   │   ├── SUMMARY.md
│   │   ├── compilation_guide.md
│   │   ├── testing_guide.md
│   │   ├── build-android-armv8a-release.sh    # Reference scripts
│   │   └── examples/                         # Example configurations
│   │
│   ├── scripts/                        # Additional build scripts
│   │   ├── build-android-arm64-v8a-release-fixed.sh
│   │   └── build-mac-x86_64-release.sh
│   │
│   ├── opencv-4.11.0/                  # OpenCV source code
│   │   ├── CMakeLists.txt
│   │   ├── include/
│   │   ├── modules/
│   │   ├── platforms/
│   │   └── ...
│   │
│   └── opencv_contrib-4.11.0/          # OpenCV contrib modules
│       ├── modules/
│       └── ...
│
├── lib/                                # Built libraries output
│   ├── v4.11.0/                        # Version-specific libraries
│   │   ├── android-arm64-v8a-release/
│   │   ├── android-armeabi-v7a-release/
│   │   ├── mac-x86_64-release/
│   │   └── ...
│   │
│   └── v2.4.13.4/                     # Legacy version
│
└── src/                                # Source code symlink
    └── opencv-4.11.0 -> ../compiler/opencv-4.11.0
```

## Key Components

### 1. Build System (`compiler/`)
- **`build.sh`**: Main optimized build script with enhanced error handling
- **`cmake/`**: Platform-specific toolchain configurations
- **`cmake_args/`**: Optimized CMake configurations for different platforms
- **`scripts/`**: Reference build scripts for specific use cases

### 2. Configuration Files (`compiler/cmake_args/`)
- **`android_options.txt`**: Optimized for Android mobile platforms
- **`host_options.txt`**: Full feature set for macOS/Linux development
- **`embedded_options.txt`**: Minimal configuration for embedded platforms
- **`common_options.txt`**: Shared options across all platforms

### 3. Documentation (`compiler/docs/`)
- Comprehensive guides for different platforms and use cases
- Troubleshooting and best practices
- Example configurations and scripts

### 4. Build Outputs
- **`build/`**: Intermediate build files (generated)
- **`lib/`**: Final compiled libraries organized by version and platform

## Usage Examples

### Building for Android ARM64
```bash
cd compiler
./build.sh -t 2 -r -n opencv -v 4.11.0
```

### Building for Current Platform
```bash
cd compiler
./build.sh -t 0 -r
```

### Building with Custom Options
```bash
cd compiler
./build.sh -t 2 -r -a "-DWITH_CUDA=ON -DBUILD_opencv_dnn=ON"
```

## Optimization Features

1. **Platform-Specific Configurations**: Each platform has optimized CMake settings
2. **Error Handling**: Comprehensive error checking and meaningful error messages
3. **Logging**: Structured logging with different severity levels
4. **Modular Design**: Easy to extend for new platforms
5. **Documentation**: Comprehensive guides and examples
6. **Backup System**: Original scripts preserved for reference

## Integration with FantasyCXX

The `opencv.cmake` file in the root directory provides integration with the main FantasyCXX project build system.

## Version Management

- Multiple OpenCV versions can coexist in the `lib/` directory
- Version-specific builds are organized by version number
- Easy to switch between different OpenCV versions

## Maintenance

- Regular updates to CMake configurations based on platform requirements
- Documentation updates for new features and platforms
- Optimization of build parameters for performance and size
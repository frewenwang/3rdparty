set(OPENCV_VERSION v4.11.0)                                                          # OpenCV的版本
if (OPENCV_VERSION STREQUAL v4.11.0)
    set(BUILD_OPENCV4 true)
    add_definitions(-DBUILD_OPENCV4)
    set(OPENCV_BUILD_TYPE release)

    # 统一的平台目录结构
    if(TARGET_OS STREQUAL "android")
        # Android 平台使用 arm64-v8a 架构
        set(OPENCV_PLATFORM_DIR ${LIB_DIR}/opencv/lib/${OPENCV_VERSION}/android-arm64-v8a-release)
    elseif(TARGET_OS STREQUAL "mac")
        # macOS 平台使用 x86_64 架构
        set(OPENCV_PLATFORM_DIR ${LIB_DIR}/opencv/lib/${OPENCV_VERSION}/mac-x86_64-release)
    else()
        # 其他平台使用通用命名
        set(OPENCV_PLATFORM_DIR ${LIB_DIR}/opencv/lib/${OPENCV_VERSION}/${TARGET_OS}-${TARGET_ARCH}-${OPENCV_BUILD_TYPE})
    endif()

    set(OPENCV_DIR ${OPENCV_PLATFORM_DIR})
    set(OPENCV_INCLUDE_DIR ${OPENCV_DIR}/include/opencv4)                               # OpenCV的include的目录
    set(OPENCV_LINK_DIR ${OPENCV_DIR}/lib)                                              # OpenCV的链接库
    set(OPENCV_3RDPARTY_LINK_DIR ${OPENCV_DIR}/lib/opencv4/3rdparty)                    # OpenCV的三方库的链接库

    # 主要库文件
    set(OPENCV_LIB opencv_world)

    # 第三方库文件
    set(OPENCV_3RDPARTY_LIBS
        libjpeg-turbo
        libpng
        libtiff
        libwebp
        libopenjp2
        IlmImf
        zlib
        ippicv
        ittnotify
    )

    # 包含目录
    include_directories(${OPENCV_INCLUDE_DIR})

    # 链接目录
    link_directories(${OPENCV_LINK_DIR})
    if(EXISTS ${OPENCV_3RDPARTY_LINK_DIR})
        link_directories(${OPENCV_3RDPARTY_LINK_DIR})
    endif()

    # 平台特定设置
    if(TARGET_OS STREQUAL "android")
        # Android 平台特定库
        set(OPENCV_PLATFORM_LIBS log)
    endif()
elseif (OPENCV_VERSION STREQUAL v2.4.13.4)
    set(BUILD_OPENCV2 true)
    add_definitions(-DBUILD_OPENCV2)
    set(OPENCV_BUILD_TYPE release)
    set(OPENCV_DIR ${LIB_DIR}/opencv/lib/${OPENCV_VERSION}/${TARGET_OS}-${TARGET_ARCH}-${OPENCV_BUILD_TYPE}) #OpenCV的目录
    set(OPENCV_INCLUDE_DIR ${OPENCV_DIR}/include/)                                      # OpenCV的include的目录
    set(OPENCV_LINK_DIR ${OPENCV_DIR}/lib/)                                             # OpenCV的链接库
    set(OPENCV_3RDPARTY_LINK_DIR ${OPENCV_DIR}/3rdparty/)                               # OpenCV的三方库的链接库
    set(OPENCV_LIB opencv_calib3d opencv_highgui opencv_imgproc opencv_core libtiff libpng IlmImf)
    include_directories(${OPENCV_INCLUDE_DIR})
    link_directories(${OPENCV_LINK_DIR})
    link_directories(${OPENCV_3RDPARTY_LINK_DIR})
endif ()
message(STATUS "[Dependency] opencv OPENCV_DIR=" ${OPENCV_DIR})
message(STATUS "[Dependency] opencv OPENCV_INCLUDE_DIR=" ${OPENCV_INCLUDE_DIR})
message(STATUS "[Dependency] opencv OPENCV_LINK_DIR=" ${OPENCV_LINK_DIR})
message(STATUS "[Dependency] opencv OPENCV_3RDPARTY_LINK_DIR=" ${OPENCV_3RDPARTY_LINK_DIR})
message(STATUS "[Dependency] opencv OPENCV_LIB=" ${OPENCV_LIB})
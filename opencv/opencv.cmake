set(OPENCV_VERSION v4.11.0)                                                          # OpenCV的版本
if (OPENCV_VERSION STREQUAL v4.11.0)
    set(BUILD_OPENCV4 true)
    add_definitions(-DBUILD_OPENCV4)
    set(OPENCV_BUILD_TYPE release)
    set(OPENCV_DIR ${LIB_DIR}/opencv/lib/${OPENCV_VERSION}/${TARGET_OS}-${TARGET_ARCH}-${OPENCV_BUILD_TYPE})# OpenCV的目录
    set(OPENCV_INCLUDE_DIR ${OPENCV_DIR}/include/)                                      # OpenCV的include的目录
    set(OPENCV_LINK_DIR ${OPENCV_DIR}/lib/)                                             # OpenCV的链接库
    set(OPENCV_3RDPARTY_LINK_DIR ${OPENCV_DIR}/3rdparty/)                               # OpenCV的三方库的链接库
    set(OPENCV_LIB opencv_calib3d opencv_highgui opencv_imgproc opencv_core libtiff libpng IlmImf opencv_imgcodecs
        opencv_flann opencv_features2d opencv_video opencv_videoio)
    include_directories(${OPENCV_INCLUDE_DIR})
    link_directories(${OPENCV_LINK_DIR})
    link_directories(${OPENCV_3RDPARTY_LINK_DIR})
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
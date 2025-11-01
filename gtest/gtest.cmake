
set(GTEST_VERSION "v1.11.0")
set(GTEST_PATH "${CMAKE_CURRENT_SOURCE_DIR}/third_party/gtest")

set(GTEST_LIB_PATH "${GTEST_PATH}/lib/${GTEST_VERSION}")
set(GTEST_SRC_PATH "${GTEST_PATH}/src/")


set(GTEST_INCLUDE_PATH ${GTEST_LIB_PATH}/include)
set(GTEST_LINK_PATH "${GTEST_LIB_PATH}/${TARGET_OS}-${TARGET_ARCH}-${CMAKE_BUILD_TYPE}/lib/")

# 判断是不是Debug模式
if (DEBUG)
    set(GTEST_LIB gtestd gtest_maind)
else ()
    set(GTEST_LIB gtest gtest_main)
endif ()



include_directories(${GTEST_INCLUDE_PATH})
link_directories(${GTEST_LINK_PATH})
message(STATUS "[Dependency] gtest GTEST_INCLUDE_PATH = ${GTEST_INCLUDE_PATH}")
message(STATUS "[Dependency] gtest GTEST_LINK_PATH = ${GTEST_LINK_PATH}")
message(STATUS "[Dependency] gtest LIB = ${GTEST_LIB}")

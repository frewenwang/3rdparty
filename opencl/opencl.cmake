
set(OPENCL_VERSION "opencl200")
set(OPENCL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/third_party/opencl/${SOC_VENDOR}/${OPENCL_VERSION}")


if (ANDROID)
    set(OPENCL_INCLUDE_PATH "${OPENCL_PATH}/include")
    if (${TARGET_ARCH} MATCHES "arm64-v8a")
        set(OPENCL_LIB_PATH "${OPENCL_PATH}/lib64")
    elseif (${TARGET_ARCH} MATCHES "armeabi-v7a")
        set(OPENCL_LIB_PATH "${OPENCL_PATH}/lib")
    endif ()
    set(OPENCL_LIB OpenCL)

    include_directories("${OPENCL_INCLUDE_PATH}")
    link_directories(${OPENCL_LIB_PATH})
    link_libraries(${OPENCL_LIB})
elseif (MAC)
    find_package(OpenCL REQUIRED)
    link_directories(${OpenCL_INCLUDE_DIRS})
    link_libraries(${OpenCL_LIBRARIES})
endif ()

message(STATUS "[Dependency] opencl PATH = ${OPENCL_PATH}")
message(STATUS "[Dependency] opencl INCLUDE_DIR = ${OPENCL_INCLUDE_PATH}")
message(STATUS "[Dependency] opencl LIB_PTH = ${OPENCL_LIB_PATH}")
message(STATUS "[Dependency] opencl LIB = ${OPENCL_LIB}")

install(FILES "${OPENCL_LIB_PATH}/libOpenCL.so" DESTINATION install/3rdparty/opencl)
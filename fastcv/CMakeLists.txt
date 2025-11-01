cmake_minimum_required(VERSION 3.10)

link_directories(
        /home/baiduiov/work/code/8295/Qnx/apps/qnx_ap/install/aarch64le/lib
)
add_definitions(-D_QNX_SOURCE)
set(DEMO_NAME fastcv_demo)

# --------------------------------------------------------------------------------------------
add_executable(
        ${DEMO_NAME}
        src/main.cpp
)
#        src/crc.S
#        src/AsmCrc3.cpp
#        src/fastcvSimpleTest.cpp

target_compile_options(${DEMO_NAME} PRIVATE -std=c++11 -g --static)
#target_compile_options(${DEMO_NAME} PRIVATE -O3)
#-mcpu=generic+crc

target_include_directories(
        ${DEMO_NAME}
        PRIVATE
        include
        /home/baiduiov/work/code/8295/Qnx/apps/qnx_ap/AMSS/inc
)

target_link_libraries(
        ${DEMO_NAME}
        PRIVATE
        fastcvopt OSAbstraction cdsprpc fastrpc_pmem smmu_client libstd
)

install(TARGETS ${DEMO_NAME}
        ARCHIVE DESTINATION ${CMAKE_BINARY_DIR}
        LIBRARY DESTINATION ${CMAKE_BINARY_DIR}
        RUNTIME DESTINATION ${CMAKE_BINARY_DIR}
        )

## -----------------------------------------------------------------------------
#add_executable(
#        AsmCrc
#        src/AsmCrc3.cpp
#)
#
#target_compile_options(AsmCrc PRIVATE -std=c++11 -mcpu=generic+crc -O3)
#-march=armv8-a+crc -mtune=generic


# -----------------------------------------------------------------------------
#add_executable(
#        AsmCrc3
#        src/test.asm src/vcIO.inc
#)
#
#target_compile_options(AsmCrc3 PRIVATE -std=c++11 -mcpu=generic+crc -O3)
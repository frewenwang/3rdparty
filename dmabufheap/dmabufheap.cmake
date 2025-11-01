set(DMA_BUF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/dmabufheap")
set(DMA_BUF_INCLUDE_PATH "${DMA_BUF_PATH}/include")

if(${CONFIG} MATCHES "arm32")
    set(DMA_BUF_LIB_DIR "${DMA_BUF_PATH}/lib")
elseif(${CONFIG} MATCHES "arm64")
    set(DMA_BUF_LIB_DIR "${DMA_BUF_PATH}/lib64")
endif()

message(STATUS "DMA_BUF_PATH = ${DMA_BUF_PATH}")
message(STATUS "DMA_BUF_INCLUDE_PATH = ${DMA_BUF_INCLUDE_PATH}")
message(STATUS "DMA_BUF_LIB_DIR = ${DMA_BUF_LIB_DIR}")


include_directories("${DMA_BUF_INCLUDE_PATH}")
link_directories(${DMA_BUF_LIB_DIR})
link_libraries("dmabufheap")

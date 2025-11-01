set(CURRENT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/")
message(STATUS "[Dependency] CURRENT_PATH = ${CURRENT_PATH}")
set(LIB_PURE_DIR 3rdparty)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/${LIB_PURE_DIR})

# include(${CURRENT_PATH}/opencl/opencl.cmake)

# include(${LIB_DIR}/opencl/opencl.cmake)

include(${LIB_DIR}/opencv/opencv.cmake)


 include(${LIB_DIR}/gtest/gtest.cmake)

if (BUILD_CERES)
    set(CERES_VERSION v2.2.0)  # 设置CERES的版本号
    set(CERES_BASE_DIR ${LIB_DIR}/ceres-solver/${CERES_VERSION})   # 设置CERES的基础目录

    set(CERES_INCLUDE_DIR ${CERES_BASE_DIR}/include)
    set(CERES_LIB_DIR ${CERES_BASE_DIR}/lib/${TARGET_OS}-${TARGET_ARCH}-${CMAKE_BUILD_TYPE})   # 根据系统版本设置依赖库的目录
    if (NOT EXISTS ${CERES_LIB_DIR})
        set(CERES_LIB_DIR ${CERES_BASE_DIR}/lib/${TARGET_OS}-${TARGET_ARCH})
    endif ()
    if (USE_CERES_FROM_SOURCE)
        set(CERES_LINK_DIR ${PROJECT_BINARY_DIR}/${LIB_PURE_DIR}/json/src/)
        add_subdirectory(${CERES_DIR}/src)
    else ()
        set(CERES_LINK_DIR ${CERES_LIB_DIR})
    endif ()
    set(CERES_LIB ceres)
    include_directories(${CERES_INCLUDE_DIR})
    link_directories(${CERES_LINK_DIR})
    message(STATUS "[Dependency] ceres INCLUDE_DIR = ${CERES_INCLUDE_DIR}")
    message(STATUS "[Dependency] ceres LINK_DIR = ${CERES_LINK_DIR}")
    message(STATUS "[Dependency] ceres LIB = ${CERES_LIB}")
endif ()

if (BUILD_EIGEN)
    set(USE_EIGEN_SYSTEM_LIB false)
    # 官网文档
    # http://ceres-solver.org/index.html
    set(EIGEN_NAME eigen)  # 设置FlatBuffers的名称
    set(EIGEN_VERSION v3.4.0)  # 设置FlatBuffers的版本号
    if (USE_EIGEN_SYSTEM_LIB)
        # 使用安装在系统环境里面的eigen库
        set(EIGEN_INCLUDE_DIR "/usr/local/include/eigen3")
        # set(EIGEN_LIB_DIR "/usr/local/lib/engines-3")                 # 这里写你的Eigen3的lib路径
        # set(EIGEN_LINK_DIR ${EIGEN_LIB_DIR})
    else ()
        # 使用项目工程本地集成的eigen库
        set(EIGEN_BASE_DIR ${LIB_DIR}/eigen/${EIGEN_VERSION})   # 设置eigen的基础目录
        set(EIGEN_INCLUDE_DIR ${EIGEN_BASE_DIR}/include)
        # set(EIGEN_LIB_DIR "")                 # 这里写你的Eigen3的lib路径
        # set(EIGEN_LINK_DIR ${EIGEN_LIB_DIR})
    endif ()
    include_directories(${EIGEN_INCLUDE_DIR})
    # Eigen是头文件库: Eigen 不需要编译成库文件，只需要包含其头文件即可。
    # link_directories("${EIGEN_LINK_DIR}")
    message(STATUS "[Dependency] eigen INCLUDE_DIR = ${EIGEN_INCLUDE_DIR}")
    message(STATUS "[Dependency] eigen LIB_DIR = ${EIGEN_LIB_DIR}")
    message(STATUS "[Dependency] eigen LINK_DIR = ${EIGEN_LINK_DIR}")
endif ()

#if (BUILD_BOOST_LIB)
#    set(BOOST_NAME boost)  # 设置Gtest的版本号
#    set(BOOST_VERSION v1.86.0)  # 设置Gtest的版本号
#    set(BOOST_BASE_DIR ${LIB_DIR}/${BOOST_NAME}/${BOOST_VERSION})   # 设置Gtest的基础目录
#
#    set(BOOST_INCLUDE_DIR ${BOOST_BASE_DIR}/include)
#    set(BOOST_LIB_DIR ${BOOST_BASE_DIR}/lib/${TARGET_OS}-${TARGET_ARCH}-${CMAKE_BUILD_TYPE})   # 根据系统版本设置依赖库的目录
#    if (NOT EXISTS ${BOOST_LIB_DIR})
#        set(BOOST_LIB_DIR ${BOOST_BASE_DIR}/lib/${TARGET_OS}-${TARGET_ARCH})
#    endif ()
#    if (USE_BOOST_FROM_SOURCE)
#        set(BOOST_LINK_DIR ${PROJECT_BINARY_DIR}/${LIB_PURE_DIR}/json/src/)
#        add_subdirectory(${BOOST_DIR}/src)
#    else ()
#        set(BOOST_LINK_DIR ${BOOST_LIB_DIR})
#    endif ()
#    set(BOOST_LIB boost_math_c99 boost_math_c99f boost_math_c99l boost_math_tr1
#        boost_math_tr1f boost_math_tr1l)
#    include_directories(${BOOST_INCLUDE_DIR})
#    link_directories(${BOOST_LINK_DIR})
#    message(STATUS "[Dependency] boost INCLUDE_DIR = ${BOOST_INCLUDE_DIR}")
#    message(STATUS "[Dependency] boost LINK_DIR = ${BOOST_LINK_DIR}")
#    message(STATUS "[Dependency] boost LIB = ${BOOST_LIB}")
#endif ()
#
#if (BUILD_OSQP)
#    set(OSQP_NAME osqp)  # 设置OSQP的版本号
#    set(OSQP_VERSION v0.6.3)  # 设置OSQP的版本号
#    set(OSQP_BASE_DIR ${LIB_DIR}/${OSQP_NAME}/${OSQP_VERSION})   # 设置OSQP的基础目录
#
#    set(OSQP_INCLUDE_DIR ${OSQP_BASE_DIR}/include)
#    set(OSQP_LIB_DIR ${OSQP_BASE_DIR}/lib/${TARGET_OS}-${TARGET_ARCH}-${CMAKE_BUILD_TYPE})   # 根据系统版本设置依赖库的目录
#    if (NOT EXISTS ${OSQP_LIB_DIR})
#        set(OSQP_LIB_DIR ${OSQP_BASE_DIR}/lib/${TARGET_OS}-${TARGET_ARCH})
#    endif ()
#    if (USE_OSQP_FROM_SOURCE)
#        set(OSQP_LINK_DIR ${PROJECT_BINARY_DIR}/${LIB_PURE_DIR}/json/src/)
#        add_subdirectory(${OSQP_DIR}/src)
#    else ()
#        set(OSQP_LINK_DIR ${OSQP_LIB_DIR})
#    endif ()
#    set(OSQP_LIB osqp)
#    include_directories(${OSQP_INCLUDE_DIR})
#    link_directories(${OSQP_LINK_DIR})
#    message(STATUS "[Dependency] OSQP INCLUDE_DIR = ${OSQP_INCLUDE_DIR}")
#    message(STATUS "[Dependency] OSQP LINK_DIR = ${OSQP_LINK_DIR}")
#    message(STATUS "[Dependency] OSQP LIB = ${OSQP_LIB}")
#endif ()
#
#if (BUILD_OSQP_EIGEN)
#    set(OSQP_Eigen_NAME osqp_eigen)  # 设置OSQP_Eigen的版本号
#    set(OSQP_Eigen_VERSION v0.8.1)  # 设置OSQP_Eigen的版本号
#    set(OSQP_Eigen_BASE_DIR ${LIB_DIR}/${OSQP_Eigen_NAME}/${OSQP_Eigen_VERSION})   # 设置OSQP_Eigen的基础目录
#
#    set(OSQP_Eigen_INCLUDE_DIR ${OSQP_Eigen_BASE_DIR}/include)
#    set(OSQP_Eigen_LIB_DIR ${OSQP_Eigen_BASE_DIR}/lib/${TARGET_OS}-${TARGET_ARCH}-${CMAKE_BUILD_TYPE})   # 根据系统版本设置依赖库的目录
#    if (NOT EXISTS ${OSQP_Eigen_LIB_DIR})
#        set(OSQP_Eigen_LIB_DIR ${OSQP_Eigen_BASE_DIR}/lib/${TARGET_OS}-${V_TARGET_ARCH})
#    endif ()
#    if (USE_OSQP_Eigen_FROM_SOURCE)
#        set(OSQP_Eigen_LINK_DIR ${PROJECT_BINARY_DIR}/${LIB_PURE_DIR}/json/src/)
#        add_subdirectory(${OSQP_Eigen_DIR}/src)
#    else ()
#        set(OSQP_Eigen_LINK_DIR ${OSQP_Eigen_LIB_DIR})
#    endif ()
#    set(OSQP_Eigen_LIB OSQP_Eigen)
#    include_directories(${OSQP_Eigen_INCLUDE_DIR})
#    link_directories(${OSQP_Eigen_LINK_DIR})
#    message(STATUS "[Dependency] OSQP_Eigen INCLUDE_DIR = ${OSQP_Eigen_INCLUDE_DIR}")
#    message(STATUS "[Dependency] OSQP_Eigen LINK_DIR = ${OSQP_Eigen_LINK_DIR}")
#    message(STATUS "[Dependency] OSQP_Eigen LIB = ${OSQP_Eigen_LIB}")
#endif ()
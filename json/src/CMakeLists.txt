SET(json_lib_name jsoncpp)

#IF(${lib_type} MATCHES ${lib_type_shared})
##    INSTALL(TARGETS ${json_lib_name} LIBRARY DESTINATION ${install_libs_dir})
#    FILE(COPY ${CMAKE_CURRENT_BINARY_DIR}/lib${json_lib_name}.so DESTINATION ${install_libs_dir})
#ELSEIF(${lib_type} MATCHES ${lib_type_static})
##    INSTALL(TARGETS ${json_lib_name} ARCHIVE DESTINATION ${install_libs_dir})
#    FILE(COPY ${CMAKE_CURRENT_BINARY_DIR}/lib${json_lib_name}.a  DESTINATION ${install_libs_dir})
#ENDIF()
#
INSTALL(DIRECTORY ../include/ DESTINATION ${VALIB_INSTALL_INCLUDE_DIR})
#FILE(COPY ../include/ DESTINATION ${install_include_dir})


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")

IF(${lib_type} MATCHES ${lib_type_shared})
    set(BUILD_SHARED_LIBS ON)
ELSEIF(${lib_type} MATCHES ${lib_type_static})
    set(BUILD_STATIC_LIBS ON)
ENDIF()

IF( CMAKE_COMPILER_IS_GNUCXX )
    #Get compiler version.
    EXECUTE_PROCESS( COMMAND ${CMAKE_CXX_COMPILER} -dumpversion
            OUTPUT_VARIABLE GNUCXX_VERSION )

    #-Werror=* was introduced -after- GCC 4.1.2
    IF( GNUCXX_VERSION VERSION_GREATER 4.1.2 )
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=strict-aliasing")
    ENDIF()
ENDIF( CMAKE_COMPILER_IS_GNUCXX )

INCLUDE(CheckIncludeFileCXX)
INCLUDE(CheckTypeSize)
INCLUDE(CheckStructHasMember)
INCLUDE(CheckCXXSymbolExists)

check_include_file_cxx(clocale HAVE_CLOCALE)
check_cxx_symbol_exists(localeconv clocale HAVE_LOCALECONV)

IF(CMAKE_VERSION VERSION_LESS 3.0.0)
    # The "LANGUAGE CXX" parameter is not supported in CMake versions below 3,
    # so the C compiler and header has to be used.
    check_include_file(locale.h HAVE_LOCALE_H)
    SET(CMAKE_EXTRA_INCLUDE_FILES locale.h)
    check_type_size("struct lconv" LCONV_SIZE)
    UNSET(CMAKE_EXTRA_INCLUDE_FILES)
    check_struct_has_member("struct lconv" decimal_point locale.h HAVE_DECIMAL_POINT)
ELSE()
    SET(CMAKE_EXTRA_INCLUDE_FILES clocale)
    check_type_size(lconv LCONV_SIZE LANGUAGE CXX)
    UNSET(CMAKE_EXTRA_INCLUDE_FILES)
    check_struct_has_member(lconv decimal_point clocale HAVE_DECIMAL_POINT LANGUAGE CXX)
ENDIF()

IF(NOT (HAVE_CLOCALE AND HAVE_LCONV_SIZE AND HAVE_DECIMAL_POINT AND HAVE_LOCALECONV))
    MESSAGE(WARNING "Locale functionality is not supported")
    ADD_DEFINITIONS(-DJSONCPP_NO_LOCALE_SUPPORT)
ENDIF()

SET( JSONCPP_INCLUDE_DIR ../include/json )
INCLUDE_DIRECTORIES(../include/)
SET( PUBLIC_HEADERS
        ${JSONCPP_INCLUDE_DIR}/config.h
        ${JSONCPP_INCLUDE_DIR}/forwards.h
        ${JSONCPP_INCLUDE_DIR}/json_features.h
        ${JSONCPP_INCLUDE_DIR}/value.h
        ${JSONCPP_INCLUDE_DIR}/reader.h
        ${JSONCPP_INCLUDE_DIR}/writer.h
        ${JSONCPP_INCLUDE_DIR}/assertions.h
        ${JSONCPP_INCLUDE_DIR}/version.h
        ${JSONCPP_INCLUDE_DIR}/autolink.h
        ${JSONCPP_INCLUDE_DIR}/json.h
        ${JSONCPP_INCLUDE_DIR}/allocator.h
        )

SOURCE_GROUP( "Public API" FILES ${PUBLIC_HEADERS} )

SET(jsoncpp_sources
        json_tool.h
        json_reader.cpp
        json_valueiterator.inl
        json_value.cpp
        json_writer.cpp
        version.h.in)

# Install instructions for this target
IF(JSONCPP_WITH_CMAKE_PACKAGE)
    SET(INSTALL_EXPORT EXPORT jsoncpp)
ELSE(JSONCPP_WITH_CMAKE_PACKAGE)
    SET(INSTALL_EXPORT)
ENDIF()

IF(BUILD_SHARED_LIBS)
    ADD_DEFINITIONS( -DJSON_DLL_BUILD )
    ADD_LIBRARY(${json_lib_name} SHARED ${PUBLIC_HEADERS} ${jsoncpp_sources})
    SET_TARGET_PROPERTIES( ${json_lib_name} PROPERTIES VERSION ${JSONCPP_VERSION} SOVERSION ${JSONCPP_SOVERSION})
    SET_TARGET_PROPERTIES( ${json_lib_name} PROPERTIES OUTPUT_NAME jsoncpp
            DEBUG_OUTPUT_NAME jsoncpp${DEBUG_LIBNAME_SUFFIX} )
    SET_TARGET_PROPERTIES( ${json_lib_name} PROPERTIES POSITION_INDEPENDENT_CODE ON)

    # Set library's runtime search path on OSX
    IF(APPLE)
        SET_TARGET_PROPERTIES( ${json_lib_name} PROPERTIES INSTALL_RPATH "@loader_path/." )
    ENDIF()

    INSTALL( TARGETS ${json_lib_name} ${INSTALL_EXPORT}
            RUNTIME DESTINATION ${INSTALL_LIB_DIR}
            LIBRARY DESTINATION ${INSTALL_LIB_DIR}
            ARCHIVE DESTINATION ${INSTALL_LIB_DIR})

    #    FILE(COPY ${CMAKE_CURRENT_BINARY_DIR}/lib${json_lib_name}.so DESTINATION ${install_libs_dir})

    IF(NOT CMAKE_VERSION VERSION_LESS 2.8.11)
        TARGET_INCLUDE_DIRECTORIES( ${json_lib_name} PUBLIC
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/${JSONCPP_INCLUDE_DIR}>)
    ENDIF()

ENDIF()

IF(BUILD_STATIC_LIBS)
    ADD_LIBRARY(${json_lib_name} STATIC ${PUBLIC_HEADERS} ${jsoncpp_sources})
    SET_TARGET_PROPERTIES( ${json_lib_name} PROPERTIES VERSION ${JSONCPP_VERSION} SOVERSION ${JSONCPP_SOVERSION})
    # avoid name clashes on windows as the shared import lib is also named jsoncpp.lib
    if (NOT DEFINED STATIC_SUFFIX AND BUILD_SHARED_LIBS)
        set (STATIC_SUFFIX "_static")
    endif ()
    SET_TARGET_PROPERTIES (${json_lib_name} PROPERTIES OUTPUT_NAME jsoncpp${STATIC_SUFFIX}
            DEBUG_OUTPUT_NAME jsoncpp${STATIC_SUFFIX}${DEBUG_LIBNAME_SUFFIX})

    #    INSTALL( TARGETS ${json_lib_name} ${INSTALL_EXPORT}
    #            RUNTIME DESTINATION ${install_libs_dir}
    #            LIBRARY DESTINATION ${install_libs_dir}
    #            ARCHIVE DESTINATION ${install_libs_dir})
    #    FILE(COPY ${CMAKE_CURRENT_BINARY_DIR}/lib${json_lib_name}.a DESTINATION ${install_libs_dir})

    IF(NOT CMAKE_VERSION VERSION_LESS 2.8.11)
        TARGET_INCLUDE_DIRECTORIES( ${json_lib_name} PUBLIC
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/${JSONCPP_INCLUDE_DIR}>
                )
    ENDIF()

ENDIF()

project(zanshin)

find_package(KDE4 REQUIRED)
find_package(KdepimLibs REQUIRED)
find_package(Boost REQUIRED)
macro_log_feature(KDEPIMLIBS_FOUND "kdepimlibs" "KDE PIM libraries" "The KDE PIM libs are required to build Zanshin" "http://www.kde.org")

include(KDE4Defaults)

macro(assert_min_ver version)
    set(error_msg "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_VERSION} not supported")
    if("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "${version}")
        message(FATAL_ERROR "${msg}")
    endif()
endmacro()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # using Clang >= 3.3
    assert_min_ver("3.3")
    add_definitions(-DQ_DECL_OVERRIDE=override)
    add_definitions(-DQ_DECL_FINAL=final)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # using GCC >= 4.8
    assert_min_ver("4.8")
    add_definitions(-DQ_DECL_OVERRIDE=override)
    add_definitions(-DQ_DECL_FINAL=final)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    # currently Intel compiler does not support all required C++11 features in zanshin
    # http://software.intel.com/en-us/articles/c0x-features-supported-by-intel-c-compiler
    assert_min_ver("15.0")
    add_definitions(-DQ_DECL_OVERRIDE=)
    add_definitions(-DQ_DECL_FINAL=)
else()
    add_definitions(-DQ_DECL_OVERRIDE=)
    add_definitions(-DQ_DECL_FINAL=)
endif()

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS} -fPIC")

option(ZANSHIN_BUILD_COVERAGE "Build Zanshin with gcov support" OFF)
if(ZANSHIN_BUILD_COVERAGE AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
include_directories (
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${KDE4_INCLUDES}
    ${KDEPIMLIBS_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIR}
)

add_subdirectory(3rdparty)
add_subdirectory(attic)
add_subdirectory(src)
add_subdirectory(tests)


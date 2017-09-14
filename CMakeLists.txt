project(zanshin)
cmake_minimum_required(VERSION 3.2)

find_package(ECM REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(GenerateExportHeader)
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(FeatureSummary)
include(ECMInstallIcons)
include(ECMMarkAsTest)
include(ECMPoQmTools)

find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Core Gui Widgets Qml Test)
find_package(Boost REQUIRED)
macro(assert_min_ver version)
    set(error_msg "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_VERSION} not supported")
    if("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "${version}")
        message(FATAL_ERROR "${msg}")
    endif()
endmacro()

if(APPLE)
    if((NOT "${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "6.0.0.0") AND
	    "${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "6.0.0.6000058")
        # Apple Clang 6.0.0.6000057 is known to fail on some of our code using std::mem_fn
        # but have no issues with boost::mem_fn
	   message("problematic Apple Clang version ${CMAKE_CXX_COMPILER_VERSION}, using boost::mem_fn")
        add_definitions(-DZANSHIN_USE_BOOST_MEM_FN)
    endif()
endif()

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    set(CMAKE_CXX_STANDARD 14) # Enable C++14, with cmake >= 3.1
    set(CMAKE_CXX_EXTENSIONS OFF) # Don't enable gcc-specific extensions
endif()

kde_enable_exceptions()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

option(ZANSHIN_BUILD_COVERAGE "Build Zanshin with gcov support" OFF)
if(ZANSHIN_BUILD_COVERAGE AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

option(ZANSHIN_BUILD_ASAN "Build Zanshin with asan support" OFF)
if(ZANSHIN_BUILD_ASAN)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls")
   link_libraries("asan")
endif()

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS -DQT_NO_URL_CAST_FROM_STRING)
include_directories (
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${Boost_INCLUDE_DIR}
    3rdparty/kdepim/
    3rdparty/kdepim/libkdepim/
)

find_package(KF5 REQUIRED COMPONENTS
    AkonadiCalendar
    AkonadiNotes
    AkonadiSearch
    IdentityManagement
    KontactInterface
    Ldap
    Runner
    Wallet
    WindowSystem
    I18n
    KDELibs4Support
)
find_package(KF5Akonadi "5.1" CONFIG REQUIRED)

add_subdirectory(3rdparty)
add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()


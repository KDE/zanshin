cmake_minimum_required(VERSION 3.16)
# KDE Application Version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "24")
set(RELEASE_SERVICE_VERSION_MINOR "11")
set(RELEASE_SERVICE_VERSION_MICRO "70")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(zanshin VERSION ${RELEASE_SERVICE_VERSION})

set(KF_MIN_VERSION "6.5.0")
find_package(ECM ${KF_MIN_VERSION} REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})
set(KDE_COMPILERSETTINGS_LEVEL "5.84")

include(GenerateExportHeader)
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(FeatureSummary)
include(ECMInstallIcons)
include(ECMMarkAsTest)
include(ECMPoQmTools)
include(ECMSetupVersion)
include(ECMDeprecationSettings)
include(ECMFeatureSummary)

set(REQUIRED_QT_VERSION "6.5.0")
find_package(Qt6 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Core Gui Widgets Test)
find_package(Boost REQUIRED)
find_package(Threads REQUIRED)

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
    set(CMAKE_CXX_STANDARD 17)
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

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_STRICT_ITERATORS
)

include_directories (
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${Boost_INCLUDE_DIR}
)

find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    Runner
    WindowSystem
    I18n
    CalendarCore
    Parts
    Crash
)

find_package(KPim6Akonadi "6.2.40" REQUIRED)
find_package(KPim6AkonadiCalendar "6.2.40" REQUIRED)
find_package(KPim6KontactInterface "6.2.40" REQUIRED)

add_definitions(-DQT_NO_CONTEXTLESS_CONNECT)
ecm_set_disabled_deprecation_versions(QT 6.3.0
     KF 6.5.0
    )

add_subdirectory(src)
ki18n_install(po)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()


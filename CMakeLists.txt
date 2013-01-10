project (zanshin)

find_package (KDE4 REQUIRED)
find_package (KdepimLibs REQUIRED)
find_package (Boost REQUIRED)
find_package(Soprano REQUIRED)
macro_log_feature(KDEPIMLIBS_FOUND "kdepimlibs" "KDE PIM libraries" "The KDE PIM libs are required to build Zanshin" "http://www.kde.org")
find_package (NepomukCore REQUIRED)

include(KDE4Defaults)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS} -fPIC")

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${KDEPIMLIBS_INCLUDE_DIRS} ${Boost_INCLUDE_DIR} ${NEPOMUK_CORE_INCLUDE_DIR})

add_subdirectory(src)
add_subdirectory(tests)


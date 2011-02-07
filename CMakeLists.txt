project (zanshin)

find_package (KDE4 REQUIRED)
find_package (KdepimLibs REQUIRED)
find_package (Boost REQUIRED)
macro_log_feature(KDEPIMLIBS_FOUND "kdepimlibs" "KDE PIM libraries" "The KDE PIM libs are required to build Zanshin" "http://www.kde.org")

include(KDE4Defaults)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${Boost_INCLUDE_DIR})

add_subdirectory(src)
add_subdirectory(tests)


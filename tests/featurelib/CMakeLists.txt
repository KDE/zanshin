set(featurelib_SRCS
   zanshincontext.cpp
)

add_definitions(-DZANSHIN_USER_XMLDATA="${CMAKE_SOURCE_DIR}/tests/features/testenv/data/testdata.xml")

include_directories(${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/src)
add_library(featurelib STATIC ${featurelib_SRCS})
target_link_libraries(featurelib
    Qt::Test
    KF${QT_MAJOR_VERSION}::ConfigCore
    integration
)


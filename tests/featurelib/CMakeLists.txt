set(featurelib_SRCS
   zanshincontext.cpp
   "${CMAKE_SOURCE_DIR}/src/zanshin/app/dependencies.cpp"
)

add_definitions(-DZANSHIN_USER_XMLDATA="${CMAKE_SOURCE_DIR}/tests/features/testenv/data/testdata.xml")

include_directories(${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/src)
add_library(featurelib STATIC ${featurelib_SRCS})
target_link_libraries(featurelib
    Qt5::Test
    KF5::ConfigCore
    presentation
)


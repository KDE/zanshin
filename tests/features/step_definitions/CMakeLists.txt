include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/cucumber-cpp/include)

# Because cucumber-cpp exhibits plenty of warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")

set(steps_SRCS main.cpp)

add_executable(zanshin-cuke-steps ${steps_SRCS})
target_link_libraries(zanshin-cuke-steps
    ${Boost_REGEX_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${QT_QTTEST_LIBRARY}
    cucumber-cpp

    testlib
    akonadi
    domain
    presentation
    utils
)

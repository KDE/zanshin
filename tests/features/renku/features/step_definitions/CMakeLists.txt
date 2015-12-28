include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/cucumber-cpp/include)

# Because cucumber-cpp exhibits plenty of warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")

set(steps_SRCS
    main.cpp
    ../../../cuke-steps.cpp
    ${CMAKE_SOURCE_DIR}/src/renku/app/dependencies.cpp
)

add_executable(renku-cuke-steps ${steps_SRCS})
target_link_libraries(renku-cuke-steps
    cucumber-cpp
    ${Boost_REGEX_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    Qt5::Test
    Qt5::Script
    KF5::AkonadiWidgets
    KF5::AkonadiXml
    testlib
    akonadi
    domain
    presentation
    scripting
    utils
)

add_subdirectory(features)

add_executable(tests-features-zanshin ../features-run.cpp)
add_test(NAME tests-features-zanshin COMMAND tests-features-zanshin)
add_definitions(-DUSER_XMLDATA="${CMAKE_CURRENT_SOURCE_DIR}/../testenv/data/testdata.xml"
                -DCUKE_STEPS="${CMAKE_CURRENT_BINARY_DIR}/features/step_definitions/zanshin-cuke-steps"
                -DFEATURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(tests-features-zanshin
    Qt5::Core
    Qt5::Network
)

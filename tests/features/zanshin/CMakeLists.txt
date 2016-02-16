add_subdirectory(features)

add_executable(zanshin-features-run ../features-run.cpp)
add_test(zanshin-features-run zanshin-features-run)
add_definitions(-DUSER_XMLDATA="${CMAKE_CURRENT_SOURCE_DIR}/../testenv/data/testdata.xml"
                -DCUKE_STEPS="${CMAKE_CURRENT_BINARY_DIR}/features/step_definitions/zanshin-cuke-steps"
                -DFEATURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(zanshin-features-run
    Qt5::Core
    Qt5::Network
)

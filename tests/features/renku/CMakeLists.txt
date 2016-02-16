add_subdirectory(features)

add_executable(renku-features-run ../features-run.cpp)
add_test(renku-features-run renku-features-run)
add_definitions(-DUSER_XMLDATA="${CMAKE_CURRENT_SOURCE_DIR}/../testenv/data/testdata.xml"
                -DCUKE_STEPS="${CMAKE_CURRENT_BINARY_DIR}/features/step_definitions/renku-cuke-steps"
                -DFEATURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(renku-features-run
    Qt5::Core
    Qt5::Network
)

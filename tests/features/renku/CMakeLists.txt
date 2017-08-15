add_subdirectory(features)

add_executable(tests-features-renku ../features-run.cpp)
add_test(tests-features-renku tests-features-renku)
add_definitions(-DUSER_XMLDATA="${CMAKE_CURRENT_SOURCE_DIR}/../testenv/data/testdata.xml"
                -DCUKE_STEPS="${CMAKE_CURRENT_BINARY_DIR}/features/step_definitions/renku-cuke-steps"
                -DFEATURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(tests-features-renku
    Qt5::Core
    Qt5::Network
)
# Don't run this at the same time as tests-features-zanshin
set_tests_properties(tests-features-renku PROPERTIES RUN_SERIAL TRUE)

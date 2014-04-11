find_package(Boost "1.51" COMPONENTS system regex unit_test_framework)
if(Boost_UNIT_TEST_FRAMEWORK_FOUND)

    add_subdirectory(step_definitions)

    set(CUCUMBER_CWD ${CMAKE_CURRENT_SOURCE_DIR}/..)

    set(zanshin-cuke-steps ${CMAKE_CURRENT_BINARY_DIR}/step_definitions/zanshin-cuke-steps)
    configure_file(cucumber-run.sh.in cucumber-run.sh)

    find_program(_akonaditest akonaditest)
    set(cucumber-run-cmd ${_akonaditest} -c ${CMAKE_CURRENT_SOURCE_DIR}/testenv/config.xml sh ${CMAKE_CURRENT_BINARY_DIR}/cucumber-run.sh)

    add_custom_target(cucumber-run
                      COMMAND ${cucumber-run-cmd}
                      WORKING_DIRECTORY ${CUCUMBER_CWD}
    )

    add_custom_target(cucumber-run-done
                      COMMAND ${cucumber-run-cmd} --tags ~@wip
                      WORKING_DIRECTORY ${CUCUMBER_CWD}
    )

    add_test(zanshin-features-done make cucumber-run-done)

endif()

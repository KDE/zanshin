find_package(Boost "1.40" COMPONENTS system regex unit_test_framework)
if(Boost_UNIT_TEST_FRAMEWORK_FOUND)

    add_subdirectory(step_definitions)

    set(CUCUMBER_CWD ${CMAKE_CURRENT_SOURCE_DIR}/..)

    set(zanshin-cuke-steps "${CMAKE_CURRENT_BINARY_DIR}/step_definitions/zanshin-cuke-steps 2>&1")


    # cucumber with akonadi fake rules

    configure_file(cucumber-run.sh.in cucumber-run.sh)

    set(cucumber-run-cmd sh ${CMAKE_CURRENT_BINARY_DIR}/cucumber-run.sh ${CMAKE_CURRENT_SOURCE_DIR}/testenv/kdehome/testdata.xml)

    add_custom_target(cucumber-run
                      COMMAND ${cucumber-run-cmd}
                      WORKING_DIRECTORY ${CUCUMBER_CWD}
    )

    add_custom_target(cucumber-run-done
                      COMMAND ${cucumber-run-cmd} --tags ~@wip
                      WORKING_DIRECTORY ${CUCUMBER_CWD}
    )

    # Default test target
    add_test(zanshin-features-done make cucumber-run-done)

    # cucumber within akonaditest rules

    configure_file(cucumber-akonadi-run.sh.in cucumber-akonadi-run.sh)
    find_program(_akonaditest akonaditest)
    set(cucumber-akonadi-run-cmd ${_akonaditest} -c ${CMAKE_CURRENT_SOURCE_DIR}/testenv/config.xml sh ${CMAKE_CURRENT_BINARY_DIR}/cucumber-akonadi-run.sh 2> ${CMAKE_CURRENT_BINARY_DIR}/akonaditest.log)

    add_custom_target(cucumber-akonadi-run
                      COMMAND ${cucumber-akonadi-run-cmd}
                      WORKING_DIRECTORY ${CUCUMBER_CWD}
    )

    add_custom_target(cucumber-akonadi-run-done
                      COMMAND ${cucumber-akonadi-run-cmd} --tags ~@wip
                      WORKING_DIRECTORY ${CUCUMBER_CWD}
    )

endif()

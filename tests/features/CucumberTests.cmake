macro(cucumber_tests)
    foreach(_testsuite ${ARGN})
        set(CUCUMBER_CWD ${CMAKE_CURRENT_SOURCE_DIR}/${_testsuite})
        set(cuke-steps "${CMAKE_CURRENT_BINARY_DIR}/${_testsuite}/features/step_definitions/${_testsuite}-cuke-steps 2>&1")


        # cucumber with akonadi fake rules
        configure_file(cucumber-run.sh.in ${_testsuite}/cucumber-run.sh)
        set(cucumber-run-cmd sh ${CMAKE_CURRENT_BINARY_DIR}/${_testsuite}/cucumber-run.sh ${CMAKE_CURRENT_SOURCE_DIR}/testenv/data/testdata.xml)

        add_custom_target(cucumber-run-${_testsuite}
                          COMMAND ${cucumber-run-cmd}
                          WORKING_DIRECTORY ${CUCUMBER_CWD}
                          USES_TERMINAL
        )

        add_custom_target(cucumber-run-${_testsuite}-done
                          COMMAND ${cucumber-run-cmd} --tags ~@wip
                          WORKING_DIRECTORY ${CUCUMBER_CWD}
                          USES_TERMINAL
        )


        # Default test target
        add_test(${_testsuite}-features-done ${CMAKE_MAKE_PROGRAM} cucumber-run-${_testsuite}-done)



        # cucumber within akonaditest rules
        configure_file(cucumber-akonadi-run.sh.in ${_testsuite}/cucumber-akonadi-run.sh)
        find_program(_akonaditest akonaditest)
        set(cucumber-akonadi-run-cmd ${_akonaditest} -c ${CMAKE_CURRENT_SOURCE_DIR}/testenv/config.xml sh ${CMAKE_CURRENT_BINARY_DIR}/${_testsuite}/cucumber-akonadi-run.sh 2> ${CMAKE_CURRENT_BINARY_DIR}/${_testsuite}/akonaditest.log)

        add_custom_target(cucumber-akonadi-run-${_testsuite}
                          COMMAND ${cucumber-akonadi-run-cmd}
                          WORKING_DIRECTORY ${CUCUMBER_CWD}
                          USES_TERMINAL
        )

        add_custom_target(cucumber-akonadi-run-${_testsuite}-done
                          COMMAND ${cucumber-akonadi-run-cmd} --tags ~@wip
                          WORKING_DIRECTORY ${CUCUMBER_CWD}
                          USES_TERMINAL
        )

        # Default test target
        add_test(${_testsuite}-features-done ${CMAKE_MAKE_PROGRAM} -C ${CMAKE_BINARY_DIR} cucumber-run-${_testsuite}-done)
    endforeach()
endmacro()

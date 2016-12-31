set(_akonaditest_source_dir ${CMAKE_CURRENT_LIST_DIR})

MACRO(ZANSHIN_AKONADI_AUTO_TESTS)
  set(prefix "${CMAKE_CURRENT_SOURCE_DIR}")
  string(REPLACE "${CMAKE_SOURCE_DIR}" "" prefix "${prefix}")
  string(REPLACE "/" "-" prefix "${prefix}")
  string(REPLACE "\\" "-" prefix "${prefix}")
  string(LENGTH "${prefix}" prefix_length)
  string(SUBSTRING "${prefix}" 1 ${prefix_length}-1 prefix)

  FOREACH(_testname ${ARGN})
    set(_prefixed_testname "${prefix}-${_testname}")
    add_executable(${_prefixed_testname} ${_testname}.cpp)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
    target_link_libraries(${_prefixed_testname} Qt5::Test akonadi domain utils)

    set(_location ${CMAKE_CURRENT_BINARY_DIR}/${_prefixed_testname})
    if (WIN32)
      set(_executable ${_location}.exe)
    else (WIN32)
      set(_executable ${_location})
    endif (WIN32)

    find_program(_testrunner akonaditest)

    set(AKONADITEST_CWD ${CMAKE_CURRENT_SOURCE_DIR})
    set(zanshin-testcommand "${_executable} 2>&1")
    configure_file(${_akonaditest_source_dir}/akonaditest-run.sh.in ${_prefixed_testname}-run.sh)

    set(akonaditest-run-cmd ${_testrunner} -c ${CMAKE_CURRENT_SOURCE_DIR}/testenv/config.xml
                            sh ${CMAKE_CURRENT_BINARY_DIR}/${_prefixed_testname}-run.sh 2> ${CMAKE_CURRENT_BINARY_DIR}/akonaditest-${_prefixed_testname}.log)

    add_custom_target(${_prefixed_testname}-run
                      COMMAND ${akonaditest-run-cmd}
                      WORKING_DIRECTORY ${AKONADITEST_CWD}
                      USES_TERMINAL
    )

    add_test(${_prefixed_testname} ${CMAKE_MAKE_PROGRAM} -C ${CMAKE_BINARY_DIR} ${_prefixed_testname}-run)
  ENDFOREACH(_testname)
ENDMACRO(ZANSHIN_AKONADI_AUTO_TESTS)

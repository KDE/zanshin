include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/testlib")

MACRO(ZANSHIN_MANUAL_TESTS)
  set(prefix "${CMAKE_CURRENT_SOURCE_DIR}")
  string(REPLACE "${CMAKE_SOURCE_DIR}" "" prefix "${prefix}")
  string(REPLACE "/" "-" prefix "${prefix}")
  string(REPLACE "\\" "-" prefix "${prefix}")
  string(LENGTH "${prefix}" prefix_length)
  string(SUBSTRING "${prefix}" 1 ${prefix_length}-1 prefix)

  FOREACH(_testname ${ARGN})
    set(_prefixed_testname "${prefix}-${_testname}")
    add_executable(${_prefixed_testname} ${_testname}.cpp)
    kde_enable_exceptions()
    target_link_libraries(${_prefixed_testname}
      Qt::Test

      integration
      widgets
    )
  ENDFOREACH(_testname)
ENDMACRO(ZANSHIN_MANUAL_TESTS)

MACRO(ZANSHIN_AUTO_TESTS)
  set(prefix "${CMAKE_CURRENT_SOURCE_DIR}")
  string(REPLACE "${CMAKE_SOURCE_DIR}" "" prefix "${prefix}")
  string(REPLACE "/" "-" prefix "${prefix}")
  string(REPLACE "\\" "-" prefix "${prefix}")
  string(LENGTH "${prefix}" prefix_length)
  string(SUBSTRING "${prefix}" 1 ${prefix_length}-1 prefix)

  FOREACH(_testname ${ARGN})
    set(_prefixed_testname "${prefix}-${_testname}")
    add_executable(${_prefixed_testname} ${_testname}.cpp)
    add_test(NAME ${_prefixed_testname} COMMAND ${_prefixed_testname})
    kde_enable_exceptions()
    target_link_libraries(${_prefixed_testname}
      Qt::Test

      testlib
      integration
      widgets
      KPim6::AkonadiXml
    )
  ENDFOREACH(_testname)
ENDMACRO(ZANSHIN_AUTO_TESTS)

MACRO(ZANSHIN_FEATURE_TESTS)
  set(prefix "${CMAKE_CURRENT_SOURCE_DIR}")
  string(REPLACE "${CMAKE_SOURCE_DIR}" "" prefix "${prefix}")
  string(REPLACE "/" "-" prefix "${prefix}")
  string(REPLACE "\\" "-" prefix "${prefix}")
  string(LENGTH "${prefix}" prefix_length)
  string(SUBSTRING "${prefix}" 1 ${prefix_length}-1 prefix)

  FOREACH(_testname ${ARGN})
    set(_prefixed_testname "${prefix}-${_testname}")
    add_executable(${_prefixed_testname} ${_testname}.cpp)
    add_test(NAME ${_prefixed_testname} COMMAND ${_prefixed_testname})
    kde_enable_exceptions()
    target_link_libraries(${_prefixed_testname}
      Qt::Test

      featurelib
      testlib
      integration
      widgets
      KPim6::AkonadiXml
    )
  ENDFOREACH(_testname)
ENDMACRO(ZANSHIN_FEATURE_TESTS)

add_subdirectory(featurelib)
add_subdirectory(features)
add_subdirectory(manual)
add_subdirectory(benchmarks)
add_subdirectory(testlib)
add_subdirectory(units)

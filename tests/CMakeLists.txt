include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/testlib")

MACRO(ZANSHIN_MANUAL_TESTS)
  FOREACH(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp ${CMAKE_SOURCE_DIR}/src/zanshin/app/dependencies.cpp)
    kde_enable_exceptions()
    target_link_libraries(${_testname}
      Qt5::Test
      
      akonadi
      domain
      presentation
      scripting
      utils
      widgets
    )
  ENDFOREACH(_testname)
ENDMACRO(ZANSHIN_MANUAL_TESTS)

MACRO(ZANSHIN_AUTO_TESTS)
  FOREACH(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    add_test(${_testname} ${_testname})
    kde_enable_exceptions()
    target_link_libraries(${_testname}
      Qt5::Test
      
      testlib
      akonadi
      domain
      presentation
      scripting
      utils
      widgets
      KF5::AkonadiXml
    )
  ENDFOREACH(_testname)
ENDMACRO(ZANSHIN_AUTO_TESTS)

add_subdirectory(features)
add_subdirectory(manual)
add_subdirectory(benchmarks)
add_subdirectory(testlib)
add_subdirectory(units)

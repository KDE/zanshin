include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/testlib")

MACRO(ZANSHIN_MANUAL_TESTS)
  FOREACH(_testname ${ARGN})
    kde4_add_executable(${_testname} TEST ${_testname}.cpp ${CMAKE_SOURCE_DIR}/src/app/dependencies.cpp)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
    target_link_libraries(${_testname}
      ${QT_QTTEST_LIBRARY}
      ${KDE4_KDEUI_LIBS}
      akonadi
      domain
      presentation
      utils
      widgets
      ${KDEPIM_STATIC_LIBS}
    )
  ENDFOREACH(_testname)
ENDMACRO(ZANSHIN_MANUAL_TESTS)

MACRO(ZANSHIN_AUTO_TESTS)
  FOREACH(_testname ${ARGN})
    kde4_add_unit_test(${_testname} TESTNAME zanshin-${_testname} ${_testname}.cpp)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
    target_link_libraries(${_testname}
      ${QT_QTTEST_LIBRARY}
      ${KDE4_KDEUI_LIBS}
      testlib
      akonadi
      domain
      presentation
      utils
      widgets
      ${KDEPIM_STATIC_LIBS}
    )
  ENDFOREACH(_testname)
ENDMACRO(ZANSHIN_AUTO_TESTS)

add_subdirectory(features)
add_subdirectory(manual)
add_subdirectory(benchmarks)
add_subdirectory(testlib)
add_subdirectory(units)

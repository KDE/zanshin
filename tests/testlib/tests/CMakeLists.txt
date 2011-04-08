include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../..)

MACRO(ZANSHIN_TESTLIB_UNIT_TESTS)
  FOREACH(_testname ${ARGN})
    kde4_add_unit_test(${_testname} TESTNAME zanshin-testlib-${_testname} ${_testname}.cpp)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
    target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_AKONADI_LIBS}
                                       ${KDEPIMLIBS_KCALCORE_LIBS} ${QT_QTTEST_LIBRARY}
                                       zanshin_testlib
    )
  ENDFOREACH(_testname)
ENDMACRO(ZANSHIN_TESTLIB_UNIT_TESTS)

########### automated tests ###############

ZANSHIN_TESTLIB_UNIT_TESTS(
    modelstructuretest
)


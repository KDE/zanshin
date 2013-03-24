add_subdirectory(testlib)

include_directories( ${CMAKE_SOURCE_DIR}/src)

MACRO(ZANSHIN_UNIT_TESTS)
  FOREACH(_testname ${ARGN})
    kde4_add_unit_test(${_testname} TESTNAME zanshin-${_testname} ${_testname}.cpp)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
    target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_AKONADI_LIBS}
                                       ${KDEPIMLIBS_KCALCORE_LIBS} ${QT_QTTEST_LIBRARY}
                                       zanshin_static
                                       zanshin_testlib
    )
  ENDFOREACH(_testname)
ENDMACRO(ZANSHIN_UNIT_TESTS)

########### automated tests ###############

ZANSHIN_UNIT_TESTS(
  modelstacktest
  selectionproxymodeltest
  sidebarmodelspec
#  todocategoriesmodelspec
  todometadatamodelspec
  todotreemodelspec
  parentstructuremodelspec
  reparentingmodelspec
  pimitemrelationstest
  pimitemtest
)


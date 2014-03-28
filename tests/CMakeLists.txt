add_subdirectory(testlib)

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

MACRO(ZANSHIN_MANUAL_TESTS)
  FOREACH(_testname ${ARGN})
    kde4_add_executable(${_testname} TEST ${_testname}.cpp)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
    target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_AKONADI_LIBS}
                                       ${KDEPIMLIBS_KCALCORE_LIBS} ${QT_QTTEST_LIBRARY}
                                       zanshin_static
                                       zanshin_testlib
                                       akonadi
                                       domain
                                       presentation
                                       utils
    )
  ENDFOREACH(_testname)
ENDMACRO(ZANSHIN_MANUAL_TESTS)

MACRO(ZANSHIN_AUTO_TESTS)
  FOREACH(_testname ${ARGN})
    kde4_add_unit_test(${_testname} TESTNAME zanshin-${_testname} ${_testname}.cpp)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
    target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_AKONADI_LIBS}
                                       ${KDEPIMLIBS_KCALCORE_LIBS} ${QT_QTTEST_LIBRARY}
                                       zanshin_static
                                       zanshin_testlib
                                       akonadi
                                       domain
                                       presentation
                                       utils
    )
  ENDFOREACH(_testname)
ENDMACRO(ZANSHIN_AUTO_TESTS)

add_subdirectory(manual)
add_subdirectory(units)
########### automated tests ###############

ZANSHIN_AUTO_TESTS(
  modelstacktest
  selectionproxymodeltest
  sidebarmodelspec
#  todocontextsmodelspec
#  todometadatamodelspec
#  todotreemodelspec
  parentstructuremodelspec
  reparentingmodelspec
  pimitemrelationstest
  pimitemtest
)


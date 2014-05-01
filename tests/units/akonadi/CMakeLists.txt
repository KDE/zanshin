MACRO(ZANSHIN_AKONADI_AUTO_TESTS)
  FOREACH(_testname ${ARGN})
    kde4_add_executable(${_testname} TEST ${_testname}.cpp)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
    target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_AKONADI_LIBS}
                                       ${KDEPIMLIBS_KCALCORE_LIBS} ${KDEPIMLIBS_KMIME_LIBS} ${QT_QTTEST_LIBRARY}
                                       akonadi
                                       domain
                                       utils
    )

    get_target_property(_location ${_testname} LOCATION)
    if (WIN32)
      set(_executable ${_location}.bat)
    else (WIN32)
      set(_executable ${_location}.shell)
    endif (WIN32)

    find_program(_testrunner akonaditest)

    add_test(zanshin-${_testname}
        ${_testrunner} -c
        ${CMAKE_CURRENT_SOURCE_DIR}/testenv/config.xml
        ${_executable}
    )
  ENDFOREACH(_testname)
ENDMACRO(ZANSHIN_AKONADI_AUTO_TESTS)

zanshin_auto_tests(
  akonadidatasourcequeriestest
  akonadiserializertest
  akonadistoragesettingstest
  akonaditaskqueriestest
  akonadinotequeriestest
  akonaditaskrepositorytest
)

zanshin_akonadi_auto_tests(
  akonadistoragetest
)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/..)

macro(zanshin_add_unit_test _testname _extra_sources)
  kde4_add_executable(${_testname} TEST ${_testname}.cpp ${_extra_sources} ${ARGN})
  target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${KDE4_AKONADI_LIBS} ${KDE4_KCAL_LIBS} ${QT_QTTEST_LIBRARY})

  # based on kde4_add_unit_test
  if (WIN32)
    get_target_property( _loc ${_testname} LOCATION )
    set(_executable ${_loc}.bat)
  else (WIN32)
    set(_executable ${EXECUTABLE_OUTPUT_PATH}/${_testname})
  endif (WIN32)
  if (UNIX)
    set(_executable ${_executable}.shell)
  endif (UNIX)

  find_program(_testrunner akonaditest)

  add_test( zanshin-${_testname} 
    ${_testrunner} -c
    ${CMAKE_CURRENT_SOURCE_DIR}/unittestenv/config.xml
    ${_executable} 
  )
endmacro(zanshin_add_unit_test)

zanshin_add_unit_test(librarymodeltest modeltestbase.cpp ../librarymodel.cpp ../todotreemodel.cpp ../todoflatmodel.cpp)
zanshin_add_unit_test(todoflatmodeltest modeltestbase.cpp ../todoflatmodel.cpp)
zanshin_add_unit_test(todotreemodeltest modeltestbase.cpp ../todotreemodel.cpp ../todoflatmodel.cpp)
zanshin_add_unit_test(todocategoriesmodeltest modeltestbase.cpp ../todocategoriesmodel.cpp ../todocategoriesattribute.cpp ../todoflatmodel.cpp)


set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/..)

macro(zanshin_add_unit_test _testname _extra_sources)
    kde4_add_unit_test(${_testname} ${_testname}.cpp ${_extra_sources} ${ARGN})
    target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${KDE4_AKONADI_LIBS} ${KDE4_KCAL_LIBS} ${QT_QTTEST_LIBRARY})
    add_definitions(-DTEST_DATA="\\"${CMAKE_CURRENT_SOURCE_DIR}/testdata.ics\\"")
endmacro(zanshin_add_unit_test)

zanshin_add_unit_test(todoflatmodeltest modeltestbase.cpp ../todoflatmodel.cpp)
zanshin_add_unit_test(todotreemodeltest modeltestbase.cpp ../todotreemodel.cpp ../todoflatmodel.cpp)
zanshin_add_unit_test(todocategoriesmodeltest modeltestbase.cpp ../todocategoriesmodel.cpp ../todocategoriesattribute.cpp ../todoflatmodel.cpp)


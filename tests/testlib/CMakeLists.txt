# Even the test lib needs to be tested. ;-)
add_subdirectory(tests)

########### testlib ###############

set(zanshin_testlib_SRCS
   c.cpp
   cat.cpp
   indent.cpp
   mockdatastore.cpp
   mockmodel.cpp
   modelbuilderbehavior.cpp
   modelnode.cpp
   modelpath.cpp
   modelstructure.cpp
   modeltest.cpp
   modelutils.cpp
   qcompare.cpp
   t.cpp
   v.cpp
   g.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/src)
kde4_add_library(zanshin_testlib STATIC ${zanshin_testlib_SRCS})
target_link_libraries(zanshin_testlib ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_AKONADI_LIBS} ${KDEPIMLIBS_KCALCORE_LIBS} ${QT_QTTEST_LIBRARY})


set(testlib_SRCS
   akonadidebug.cpp
   akonadifakedata.cpp
   akonadifakedataxmlloader.cpp
   akonadifakejobs.cpp
   akonadifakemonitor.cpp
   akonadifakestorage.cpp
   akonadifakestoragebehavior.cpp
   akonadistoragetestbase.cpp
   fakejob.cpp
   gencollection.cpp
   gentag.cpp
   gentodo.cpp
   modeltest.cpp
   monitorspy.cpp
   testhelpers.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/src)
add_library(testlib STATIC ${testlib_SRCS})
target_link_libraries(testlib
    KF5::AkonadiCore
    KF5::AkonadiXml
    KF5::CalendarCore
    Qt5::Test
    KF5::KDELibs4Support
)


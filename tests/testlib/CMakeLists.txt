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
   gentodo.cpp
   monitorspy.cpp
   testhelpers.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/src)
add_library(testlib STATIC ${testlib_SRCS})
set_property(TARGET testlib PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(testlib
    KPim6::AkonadiCore
    KPim6::AkonadiXml
    KF6::CalendarCore
    Qt::DBus
    Qt::Test
)


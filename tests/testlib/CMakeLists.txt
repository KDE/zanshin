set(testlib_SRCS
   akonadimocks.cpp
   modeltest.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/src)
kde4_add_library(testlib STATIC ${testlib_SRCS})
target_link_libraries(testlib ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_AKONADI_LIBS} ${KDEPIMLIBS_KCALCORE_LIBS} ${QT_QTTEST_LIBRARY})


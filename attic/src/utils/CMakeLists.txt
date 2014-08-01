include_directories(${CMAKE_SOURCE_DIR}/src/pimlibs ${CMAKE_BINARY_DIR}/src/pimlibs)
set(oldutils_SRCS
    datestringbuilder.cpp
    ${CMAKE_SOURCE_DIR}/src/pimlibs/kdateedit.cpp
    ${CMAKE_SOURCE_DIR}/src/pimlibs/kdatepickerpopup.cpp
)

kde4_add_library(oldutils STATIC ${oldutils_SRCS})
target_link_libraries(oldutils
    ${KDE4_KDECORE_LIBS}
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${QT_QTXML_LIBRARY}
    akonadi-notes
#   kmime
)

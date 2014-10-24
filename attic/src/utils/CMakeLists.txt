include_directories(${CMAKE_SOURCE_DIR}/3rdparty/kdepim/libkdepim ${CMAKE_BINARY_DIR}/3rdparty/kdepim/libkdepim)
set(oldutils_SRCS
    datestringbuilder.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/kdepim/libkdepim/kdateedit.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/kdepim/libkdepim/kdatepickerpopup.cpp
)

kde4_add_library(oldutils STATIC ${oldutils_SRCS})
target_link_libraries(oldutils
    ${KDE4_KDECORE_LIBS}
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${QT_QTXML_LIBRARY}
    akonadi-notes
#   kmime
)

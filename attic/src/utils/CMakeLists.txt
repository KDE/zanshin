set(oldutils_SRCS
    datestringbuilder.cpp
    kdateedit.cpp
    kdatepickerpopup.cpp
)

kde4_add_library(oldutils STATIC ${oldutils_SRCS})
target_link_libraries(oldutils
    ${KDE4_KDECORE_LIBS}
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${QT_QTXML_LIBRARY}
    akonadi-notes
#   kmime
)

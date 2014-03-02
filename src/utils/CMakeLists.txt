
set(utils_SRCS
    datestringbuilder.cpp
    dependencymanager.cpp
    kdateedit.cpp
    kdatepickerpopup.cpp
 )

kde4_add_library(utils STATIC ${utils_SRCS})
target_link_libraries(utils
  ${KDE4_KDECORE_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${QT_QTXML_LIBRARY}
  akonadi-notes
#   kmime
)

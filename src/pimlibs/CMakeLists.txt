set(pimlibs_SRCS
    kdateedit.cpp
    kdatepickerpopup.cpp
)

kde4_add_library(pimlibs STATIC ${pimlibs_SRCS})
target_link_libraries(pimlibs ${KDE4_KDECORE_LIBS})

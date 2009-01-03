find_package(KdepimLibs REQUIRED)

add_subdirectory(tests)

########### next target ###############

set(zanshin_SRCS
   actionlistdelegate.cpp
   actionlistview.cpp
   contextsmodel.cpp
   globalmodel.cpp
   main.cpp
   mainwindow.cpp
   projectsmodel.cpp
   todocategoriesattribute.cpp
   todocategoriesmodel.cpp
   todoflatmodel.cpp
   todotreemodel.cpp
)

kde4_add_executable(zanshin ${zanshin_SRCS})

target_link_libraries(zanshin ${KDE4_KDEUI_LIBS} ${KDE4_AKONADI_LIBS} ${KDE4_KCAL_LIBS})

install(TARGETS zanshin DESTINATION ${BIN_INSTALL_DIR})

########### install files ###############




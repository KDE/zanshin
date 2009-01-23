find_package(KdepimLibs REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

add_subdirectory(tests)

########### next target ###############

set(zanshin_SRCS
   actionlistdelegate.cpp
   actionlisteditor.cpp
   actionlistmodel.cpp
   actionlistview.cpp
   configdialog.cpp
   contextsmodel.cpp
   globalmodel.cpp
   librarymodel.cpp
   main.cpp
   mainwindow.cpp
   projectsmodel.cpp
   todocategoriesattribute.cpp
   todocategoriesmodel.cpp
   todoflatmodel.cpp
   todotreemodel.cpp
   sidebar.cpp
)

kde4_add_kcfg_files(zanshin_SRCS globalsettings.kcfgc)

kde4_add_executable(zanshin ${zanshin_SRCS})

target_link_libraries(zanshin ${KDE4_KDEUI_LIBS} ${KDE4_AKONADI_LIBS} ${KDE4_KCAL_LIBS})

install(TARGETS zanshin DESTINATION ${BIN_INSTALL_DIR})
install(FILES zanshinui.rc DESTINATION ${DATA_INSTALL_DIR}/zanshin)

########### install files ###############




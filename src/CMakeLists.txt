########### zanshin ###############

set(zanshin_SRCS
   main.cpp
   actionduedatedelegate.cpp
   actionlistdelegate.cpp
   actionlisteditor.cpp
   combomodel.cpp
   debugwindow.cpp
   kdateedit.cpp
   kdatepickerpopup.cpp
   mainwindow.cpp
   modelstack.cpp
   selectionproxymodel.cpp
   sidebar.cpp
   sidebarmodel.cpp
   todocategoriesmodel.cpp
   todoproxymodelbase.cpp
   todotreemodel.cpp
   todomodel.cpp
   todonode.cpp
   todonodemanager.cpp
)

kde4_add_executable(zanshin ${zanshin_SRCS})

target_link_libraries(zanshin ${KDE4_KDEUI_LIBS} ${KDE4_AKONADI_LIBS} ${KDE4_KCAL_LIBS})

########### install files ###############

install(TARGETS zanshin DESTINATION ${BIN_INSTALL_DIR})
install(FILES zanshinui.rc DESTINATION ${DATA_INSTALL_DIR}/zanshin)
install(FILES zanshin.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})


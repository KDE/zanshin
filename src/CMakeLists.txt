########### zanshin ###############

set(zanshin_SRCS
   main.cpp
   actionduedatedelegate.cpp
   actionlistcheckablemodel.cpp
   actionlistcombobox.cpp
   actionlistcompletermodel.cpp
   actionlistcompleterview.cpp
   actionlistdelegate.cpp
   actionlisteditor.cpp
   actionlisteditorpage.cpp
   categorymanager.cpp
   combomodel.cpp
   debugwindow.cpp
   kdateedit.cpp
   kdatepickerpopup.cpp
   mainwindow.cpp
   modelstack.cpp
   quickselectdialog.cpp
   selectionproxymodel.cpp
   sidebar.cpp
   sidebarmodel.cpp
   sidebarpage.cpp
   todocategoriesmodel.cpp
   todohelpers.cpp
   todoproxymodelbase.cpp
   todotreemodel.cpp
   todotreeview.cpp
   todomodel.cpp
   todonode.cpp
   todonodemanager.cpp
)

kde4_add_executable(zanshin ${zanshin_SRCS})

target_link_libraries(zanshin ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_AKONADI_LIBS} ${KDEPIMLIBS_KCALCORE_LIBS})

########### install files ###############

install(TARGETS zanshin DESTINATION ${BIN_INSTALL_DIR})
install(FILES zanshinui.rc DESTINATION ${DATA_INSTALL_DIR}/zanshin)
install(FILES zanshin.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})


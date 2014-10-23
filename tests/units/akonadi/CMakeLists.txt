include(MacroAkonadiAutoTests)

zanshin_auto_tests(
  akonadiapplicationselectedattributetest
  akonadiartifactqueriestest
  akonadicontextqueriestest
  akonadicontextrepositorytest
  akonadidatasourcequeriestest
  akonadidatasourcerepositorytest
  akonadinotequeriestest
  akonadinoterepositorytest
  akonadiprojectqueriestest
  akonadiprojectrepositorytest
  akonadiserializertest
  akonadistoragesettingstest
  akonaditagqueriestest
  akonaditaskqueriestest
  akonaditaskrepositorytest
)

zanshin_akonadi_auto_tests(
  akonadistoragetest
)

target_link_libraries(akonadistoragetest testlib)

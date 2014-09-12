include(MacroAkonadiAutoTests)

zanshin_auto_tests(
  akonadiartifactqueriestest
  akonadicontextqueriestest
  akonadidatasourcequeriestest
  akonadinotequeriestest
  akonadinoterepositorytest
  akonadiprojectqueriestest
  akonadiprojectrepositorytest
  akonadiserializertest
  akonadistoragesettingstest
  akonaditaskqueriestest
  akonaditaskrepositorytest
)

zanshin_akonadi_auto_tests(
  akonadistoragetest
)

target_link_libraries(akonadistoragetest testlib)

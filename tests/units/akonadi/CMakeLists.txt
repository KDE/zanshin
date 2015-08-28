include(MacroAkonadiAutoTests)

zanshin_auto_tests(
  akonadiapplicationselectedattributetest
  akonadiartifactqueriestest
  akonadicontextqueriestest
  akonadicontextrepositorytest
  akonadidatasourcequeriestest
  akonadidatasourcerepositorytest
  akonadilivequeryintegratortest
  akonadinotequeriestest
  akonadinoterepositorytest
  akonadiprojectqueriestest
  akonadiprojectrepositorytest
  akonadiserializertest
  akonadistoragesettingstest
  akonaditagqueriestest
  akonaditagrepositorytest
  akonaditaskqueriestest
  akonaditaskrepositorytest
  akonaditimestampattributetest
)

zanshin_akonadi_auto_tests(
  akonadistoragetest
)

target_link_libraries(akonadistoragetest testlib)

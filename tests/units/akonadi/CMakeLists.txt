include(MacroAkonadiAutoTests)

zanshin_auto_tests(
  akonadiapplicationselectedattributetest
  akonadicontextqueriestest
  akonadicontextrepositorytest
  akonadidatasourcequeriestest
  akonadidatasourcerepositorytest
  akonadilivequeryhelperstest
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

target_link_libraries(akonadistoragetest akonadi testlib)

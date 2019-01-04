include(MacroAkonadiAutoTests)

zanshin_auto_tests(
  akonadiapplicationselectedattributetest
  akonadicachetest
  akonadicachingstoragetest
  akonadicontextqueriestest
  akonadicontextrepositorytest
  akonadidatasourcequeriestest
  akonadidatasourcerepositorytest
  akonadilivequeryhelperstest
  akonadilivequeryintegratortest
  akonadiprojectqueriestest
  akonadiprojectrepositorytest
  akonadiserializertest
  akonadistoragesettingstest
  akonaditaskqueriestest
  akonaditaskrepositorytest
  akonaditimestampattributetest
)

zanshin_akonadi_auto_tests(
  akonadistoragetest
  akonadicachingstorageintegrationtest
)

target_link_libraries(tests-units-akonadi-akonadistoragetest akonadi testlib)
target_link_libraries(tests-units-akonadi-akonadicachingstorageintegrationtest akonadi testlib)

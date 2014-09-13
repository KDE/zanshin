include(MacroAkonadiAutoTests)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../src/migrator)

zanshin_auto_tests(
)

zanshin_akonadi_auto_tests(
  zanshin021migrationtest
)

target_link_libraries(zanshin021migrationtest
   migrator
   testlib
)

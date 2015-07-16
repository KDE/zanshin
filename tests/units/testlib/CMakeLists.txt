add_definitions(-DSOURCE_DIR=\\"${CMAKE_CURRENT_SOURCE_DIR}\\")

zanshin_auto_tests(
  akonadifakedatatest
  akonadifakedataxmlloadertest
  akonadifakestoragetest
  gencollectiontest
  gennotetest
  gentagtest
  gentodotest
  monitorspytest
)

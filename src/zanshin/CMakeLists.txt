add_subdirectory(app)
add_subdirectory(kontact)
if(NOT WIN32)
   add_subdirectory(runner)
endif()
add_subdirectory(migrator)


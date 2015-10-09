include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# frontends
add_subdirectory(zanshin)

# modules
add_subdirectory(akonadi)
add_subdirectory(domain)
add_subdirectory(presentation)
add_subdirectory(scripting)
add_subdirectory(utils)
add_subdirectory(widgets)


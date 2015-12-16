include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# icons
add_subdirectory(icons)

# frontends
add_subdirectory(zanshin)
add_subdirectory(renku)

# modules
add_subdirectory(akonadi)
add_subdirectory(domain)
add_subdirectory(presentation)
add_subdirectory(scripting)
add_subdirectory(utils)
add_subdirectory(widgets)


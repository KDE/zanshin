include_directories(${CMAKE_CURRENT_SOURCE_DIR})

ecm_setup_version(${RELEASE_SERVICE_VERSION} VARIABLE_PREFIX ZANSHIN VERSION_HEADER zanshin_version.h)

# icons
add_subdirectory(icons)

# frontends
add_subdirectory(zanshin)

# modules
add_subdirectory(akonadi)
add_subdirectory(domain)
add_subdirectory(presentation)
add_subdirectory(integration)
add_subdirectory(utils)
add_subdirectory(widgets)


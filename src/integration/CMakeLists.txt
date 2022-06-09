
add_library(integration STATIC)
target_sources(integration PRIVATE
    dependencies.cpp
    dependencies.h
)
set_property(TARGET integration PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(integration
    akonadi
    domain
    presentation
    utils
)

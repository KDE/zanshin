set(integration_SRCS
    dependencies.cpp
)

add_library(integration STATIC ${integration_SRCS})
set_property(TARGET integration PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(integration
    akonadi
    domain
    presentation
    utils
)

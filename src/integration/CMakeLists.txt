set(integration_SRCS
    dependencies.cpp
)

add_library(integration STATIC ${integration_SRCS})
target_link_libraries(integration
    akonadi
    domain
    presentation
    utils
)

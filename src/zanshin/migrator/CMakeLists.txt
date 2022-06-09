
add_library(migrator STATIC)
target_sources(migrator PRIVATE
    zanshin021migrator.cpp
    zanshincontextitemsmigrator.cpp
    zanshin021migrator.h
    zanshincontextitemsmigrator.h
)
set_property(TARGET migrator PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(migrator
    akonadi
    domain
    utils
)

add_executable(zanshin-migrator main.cpp)
target_link_libraries(zanshin-migrator migrator)
install(TARGETS zanshin-migrator DESTINATION ${KDE_INSTALL_BINDIR})

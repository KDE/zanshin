set(migrator_SRCS
    zanshin021migrator.cpp
)

add_library(migrator STATIC ${migrator_SRCS})
target_link_libraries(migrator
    akonadi
    domain
    utils
)

add_executable(zanshin-migrator main.cpp)
target_link_libraries(zanshin-migrator migrator)
install(TARGETS zanshin-migrator DESTINATION ${KDE_INSTALL_BINDIR})

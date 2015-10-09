set(migrator_SRCS
    zanshin021migrator.cpp
)

kde4_add_library(migrator STATIC ${migrator_SRCS})
target_link_libraries(migrator
    ${QT4_QTCORE_LIBRARY}
    akonadi
    domain
    utils
)

kde4_add_executable(zanshin-migrator main.cpp)
target_link_libraries(zanshin-migrator migrator)
install(TARGETS zanshin-migrator DESTINATION ${BIN_INSTALL_DIR})

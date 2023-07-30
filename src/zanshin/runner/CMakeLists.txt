
kcoreaddons_add_plugin(org.kde.zanshin INSTALL_NAMESPACE "kf6/krunner")
target_sources(org.kde.zanshin PRIVATE
    zanshinrunner.cpp
    zanshinrunner.h
)
target_link_libraries(org.kde.zanshin
    akonadi
    domain
    utils
    KF6::ConfigCore
    KF6::Runner
)

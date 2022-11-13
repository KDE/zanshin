
kcoreaddons_add_plugin(org.kde.zanshin INSTALL_NAMESPACE "kf${QT_MAJOR_VERSION}/krunner")
target_sources(org.kde.zanshin PRIVATE
    zanshinrunner.cpp
    zanshinrunner.h
)
target_link_libraries(org.kde.zanshin
    akonadi
    domain
    utils
    KF5::ConfigCore
    KF5::Runner
)

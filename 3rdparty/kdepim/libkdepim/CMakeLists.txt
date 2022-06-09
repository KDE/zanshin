add_definitions(-DQT_NO_CAST_FROM_ASCII)
add_definitions(-DQT_NO_CAST_TO_ASCII)
add_definitions(-DTRANSLATION_DOMAIN=\"libkdepim\")

add_library(zanshinkdepimstatic STATIC ${kdepim_LIB_SRCS})
target_sources(zanshinkdepimstatic PRIVATE kdateedit.cpp kdateedit.h)
set_property(TARGET zanshinkdepimstatic PROPERTY POSITION_INDEPENDENT_CODE ON)

generate_export_header(zanshinkdepimstatic BASE_NAME kdepim)

target_link_libraries(zanshinkdepimstatic
  KF5::WidgetsAddons
  KF5::I18n
)

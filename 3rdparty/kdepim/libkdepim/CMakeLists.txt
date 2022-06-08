add_definitions(-DQT_NO_CAST_FROM_ASCII)
add_definitions(-DQT_NO_CAST_TO_ASCII)
add_definitions(-DTRANSLATION_DOMAIN=\"libkdepim\")

set(kdepim_LIB_SRCS
   kdateedit.cpp
)

add_library(zanshinkdepimstatic STATIC ${kdepim_LIB_SRCS})
set_property(TARGET zanshinkdepimstatic PROPERTY POSITION_INDEPENDENT_CODE ON)

generate_export_header(zanshinkdepimstatic BASE_NAME kdepim)

target_link_libraries(zanshinkdepimstatic
  KF5::WidgetsAddons
  KF5::I18n
)

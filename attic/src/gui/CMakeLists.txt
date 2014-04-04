add_subdirectory(itemlist)
add_subdirectory(itemeditor)
add_subdirectory(shared)
add_subdirectory(sidebar)

set(gui_SRCS
    ${itemlist_SRCS}
    ${itemeditor_SRCS}
    ${shared_SRCS}
    ${sidebar_SRCS}
    PARENT_SCOPE
)

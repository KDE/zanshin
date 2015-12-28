macro(add_libkdepim_blacklistbaloocompletion_unittest _name)
    ecm_add_test(${ARGN}
        TEST_NAME ${_name}
        NAME_PREFIX "libkdepim-addessline-blaclistbaloocompletion-"
        LINK_LIBRARIES Qt5::Test KF5::Libkdepim KF5::WidgetsAddons KF5::Completion KF5::I18n KF5::ConfigCore KF5::ItemViews KF5::Codecs
    )
endmacro()

add_libkdepim_blacklistbaloocompletion_unittest(emailcompletiondialogtest
    blacklistbalooemailcompletiondialogtest.cpp
    ../blacklistbalooemailcompletionwidget.cpp
    ../blacklistbalooemailwarning.cpp
)
add_libkdepim_blacklistbaloocompletion_unittest(emailcompletionwidgettest
    blacklistbalooemailcompletionwidgettest.cpp
    ../blacklistbalooemailcompletionwidget.cpp
    ../blacklistbalooemailwarning.cpp
)
add_libkdepim_blacklistbaloocompletion_unittest(emaillisttest
    blacklistbalooemaillisttest.cpp
)
add_libkdepim_blacklistbaloocompletion_unittest(emailsearchjobtest
    blacklistbalooemailsearchjobtest.cpp
)
add_libkdepim_blacklistbaloocompletion_unittest(emailutiltest
    blacklistbalooemailutiltest.cpp
)
add_libkdepim_blacklistbaloocompletion_unittest(emailwarningtest
    blacklistbalooemailwarningtest.cpp
    ../blacklistbalooemailwarning.cpp
)

find_package(Boost "1.40" COMPONENTS system regex unit_test_framework)
if(Boost_UNIT_TEST_FRAMEWORK_FOUND)

    add_subdirectory(zanshin)

    include(CucumberTests.cmake)
    cucumber_tests(zanshin)

endif()

include_directories(../)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

########### next target ###############

set(testaddresseelineedit_SRCS testaddresseelineedit.cpp)

add_executable(testaddresseelineedit ${testaddresseelineedit_SRCS})
ecm_mark_as_test(testaddresseelineedit)

target_link_libraries(testaddresseelineedit zanshinkdepimstatic )

########### next target ###############

set(testldapclient_SRCS testldapclient.cpp)

add_executable(testldapclient ${testldapclient_SRCS})
ecm_mark_as_test(testldapclient)

target_link_libraries(testldapclient KF5::Ldap  zanshinkdepimstatic)


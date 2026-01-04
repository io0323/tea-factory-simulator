add_executable(unit_tests test_dummy.cpp)

target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_test(NAME unit_tests COMMAND unit_tests)



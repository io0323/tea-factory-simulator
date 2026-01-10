add_executable(unit_tests test_dummy.cpp)

target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_test(NAME unit_tests COMMAND unit_tests)

add_executable(cli_args_tests
  test_args.cpp
  ${CMAKE_SOURCE_DIR}/src/cli/Args.cpp
)

target_include_directories(cli_args_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_test(NAME cli_args_tests COMMAND cli_args_tests)



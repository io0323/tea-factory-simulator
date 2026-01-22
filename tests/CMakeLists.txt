add_executable(unit_tests
  test_teabatch.cpp
  ${CMAKE_SOURCE_DIR}/src/TeaBatch.cpp
)

target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(unit_tests PRIVATE tea_core)

add_test(NAME unit_tests COMMAND unit_tests)

add_executable(cli_args_tests
  test_args.cpp
  ${CMAKE_SOURCE_DIR}/src/cli/Args.cpp
)

target_include_directories(cli_args_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_test(NAME cli_args_tests COMMAND cli_args_tests)

add_test(NAME teabatch_tests COMMAND unit_tests)

add_executable(simulator_tests
  test_simulator.cpp
)

target_include_directories(simulator_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(simulator_tests PRIVATE tea_core)

add_test(NAME simulator_tests COMMAND simulator_tests)

add_executable(csvwriter_tests
  test_csvwriter.cpp
)

target_include_directories(csvwriter_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(csvwriter_tests PRIVATE tea_core)

add_test(NAME csvwriter_tests COMMAND csvwriter_tests)

add_executable(process_tests
  test_processes.cpp
)

target_include_directories(process_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(process_tests PRIVATE tea_core)

add_test(NAME process_tests COMMAND process_tests)

add_executable(model_tests
  test_model.cpp
)

target_include_directories(model_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(model_tests PRIVATE tea_core)

add_test(NAME model_tests COMMAND model_tests)

add_executable(tealeaf_tests
  test_tealeaf.cpp
)

target_include_directories(tealeaf_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(tealeaf_tests PRIVATE tea_core)

add_test(NAME tealeaf_tests COMMAND tealeaf_tests)

add_executable(csvwriter_io_tests
  test_csvwriter_io.cpp
)

target_include_directories(csvwriter_io_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(csvwriter_io_tests PRIVATE tea_core)

add_test(NAME csvwriter_io_tests COMMAND csvwriter_io_tests)


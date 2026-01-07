add_executable(unit_tests
  test_teabatch.cpp
  ${CMAKE_SOURCE_DIR}/src/TeaBatch.cpp
)

target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_test(NAME unit_tests COMMAND unit_tests)

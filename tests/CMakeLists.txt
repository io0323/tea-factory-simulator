add_executable(unit_tests
  test_teabatch.cpp
  ${CMAKE_SOURCE_DIR}/src/TeaBatch.cpp
  ${CMAKE_SOURCE_DIR}/src/domain/Model.cpp
  ${CMAKE_SOURCE_DIR}/src/process/SteamingProcess.cpp
  ${CMAKE_SOURCE_DIR}/src/process/RollingProcess.cpp
  ${CMAKE_SOURCE_DIR}/src/process/DryingProcess.cpp
)

target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_test(NAME unit_tests COMMAND unit_tests)



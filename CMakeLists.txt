cmake_minimum_required(VERSION 3.16)

project(TeaFactorySimulator VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(TEAFACTORY_BUILD_CLI "Build CLI simulator (no GUI deps)" ON)
option(TEAFACTORY_BUILD_GUI "Build GUI dashboard (ImGui/GLFW/OpenGL3)" ON)

if(TEAFACTORY_BUILD_CLI)
  add_executable(tea_factory_simulator_cli
    src/cli/main.cpp
    src/cli/Args.cpp
    src/io/CsvWriter.cpp
    src/process/SteamingProcess.cpp
    src/process/RollingProcess.cpp
    src/process/DryingProcess.cpp
    src/simulation/Simulator.cpp
  )

  target_include_directories(tea_factory_simulator_cli PRIVATE src)
endif()

if(TEAFACTORY_BUILD_GUI)
  include(FetchContent)

  # GUI 依存は FetchContent で取得します。
  # - GLFW: ウィンドウ/入力
  # - Dear ImGui: 軽量GUI
  # - gl3w: OpenGL 関数ローダ（ImGuiリポジトリに同梱）
  set(FETCHCONTENT_QUIET OFF)

  FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.9
  )
  set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

  FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.2
  )

  FetchContent_MakeAvailable(glfw imgui)

  add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
  )

  target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${glfw_SOURCE_DIR}/include
  )

  # imgui_impl_opengl3.cpp は ImGui 同梱の内部ローダ
  # (backends/imgui_impl_opengl3_loader.h) を自動使用します。

  add_executable(TeaFactorySimulator
    src/main.cpp
    src/io/CsvWriter.cpp
    src/TeaBatch.cpp
    src/Simulator.cpp
  )

  target_include_directories(TeaFactorySimulator PRIVATE src)
  target_link_libraries(TeaFactorySimulator PRIVATE imgui_lib glfw)

  if(APPLE)
    target_compile_definitions(TeaFactorySimulator PRIVATE GL_SILENCE_DEPRECATION)
    target_link_libraries(TeaFactorySimulator PRIVATE "-framework OpenGL")
  else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(TeaFactorySimulator PRIVATE OpenGL::GL)
  endif()
endif()



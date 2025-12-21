#include <chrono>
#include <cstdio>
#include <optional>

#include "Simulator.h"
#include "TeaBatch.h"

#include "io/CsvWriter.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

namespace {

/* 値を [0, 1] にクランプします。 */
float clamp01(float v) {
  if (v < 0.0F) {
    return 0.0F;
  }
  if (v > 1.0F) {
    return 1.0F;
  }
  return v;
}

/* プログレスバー + 数値を同一行で描画します。 */
void draw_bar(const char* label,
              float fraction,
              const char* overlay,
              float width) {
  /*
    UI 要件:
      - ImGui::ProgressBar を使用
      - 1 画面ダッシュボードで状態を可視化
  */
  ImGui::TextUnformatted(label);
  ImGui::SameLine(140.0F);
  ImGui::ProgressBar(clamp01(fraction), ImVec2(width, 0.0F), overlay);
}

} /* namespace */

/* GUI アプリのエントリポイントです。 */
int main() {
  /*
    ビルド:
      cmake -S . -B build
      cmake --build build -j

    実行:
      ./build/TeaFactorySimulator
  */
  if (glfwInit() == GLFW_FALSE) {
    std::fprintf(stderr, "Failed to init GLFW\n");
    return 1;
  }

  const char* glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow* window =
      glfwCreateWindow(720, 360, "TeaFactory Simulator", nullptr, nullptr);
  if (window == nullptr) {
    std::fprintf(stderr, "Failed to create GLFW window\n");
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  tea_gui::Simulator simulator;
  std::optional<tea_io::CsvWriter> csv;
  int last_csv_elapsed = -1;

  using clock = std::chrono::steady_clock;
  auto last = clock::now();

  while (glfwWindowShouldClose(window) == GLFW_FALSE) {
    glfwPollEvents();

    const auto now = clock::now();
    const std::chrono::duration<double> dt = now - last;
    last = now;

    simulator.update(dt.count());

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(700, 340), ImGuiCond_Always);
    ImGui::Begin("TeaFactory Simulator", nullptr,
                 ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoCollapse);

    const tea_gui::TeaBatch& batch = simulator.batch();

    /*
      GUI版CSV出力:
      - Start時に tea_factory_gui.csv を新規作成（上書き）
      - 実行中、elapsedSeconds が進んだタイミング（=1秒ごと）で1行追記
    */
    const int elapsed = batch.elapsed_seconds();
    if (csv.has_value() && elapsed != last_csv_elapsed) {
      last_csv_elapsed = elapsed;
      csv->write_row(tea_gui::to_string(batch.process()),
                     elapsed,
                     batch.moisture(),
                     batch.temperature_c(),
                     batch.aroma(),
                     batch.color());
    }

    ImGui::Text("Current Process: %s", tea_gui::to_string(batch.process()));
    ImGui::Text("Elapsed Time: %d sec", batch.elapsed_seconds());
    ImGui::Separator();

    const float bar_width = 360.0F;

    const float moisture = static_cast<float>(batch.moisture());
    const float moisture_pct = moisture * 100.0F;
    char moisture_text[32];
    std::snprintf(moisture_text, sizeof(moisture_text), "%.0f%%",
                  moisture_pct);
    draw_bar("Moisture", moisture, moisture_text, bar_width);

    const float temp_c = static_cast<float>(batch.temperature_c());
    const float temp_fraction = clamp01(temp_c / 100.0F);
    char temp_text[32];
    std::snprintf(temp_text, sizeof(temp_text), "%.0fC", temp_c);
    draw_bar("Temperature", temp_fraction, temp_text, bar_width);

    const float aroma = static_cast<float>(batch.aroma());
    const float aroma_fraction = clamp01(aroma / 100.0F);
    char aroma_text[32];
    std::snprintf(aroma_text, sizeof(aroma_text), "%.0f", aroma);
    draw_bar("Aroma", aroma_fraction, aroma_text, bar_width);

    const float color = static_cast<float>(batch.color());
    const float color_fraction = clamp01(color / 100.0F);
    char color_text[32];
    std::snprintf(color_text, sizeof(color_text), "%.0f", color);
    draw_bar("Color", color_fraction, color_text, bar_width);

    ImGui::Separator();

    const double score = batch.quality_score();
    ImGui::Text("Quality Score: %.0f", score);
    ImGui::Text("Status: %s", batch.quality_status().c_str());

    ImGui::Separator();

    if (ImGui::Button("Start")) {
      simulator.start();
      if (!csv.has_value()) {
        csv.emplace("tea_factory_gui.csv");
        csv->write_header();
        last_csv_elapsed = -1;
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Pause")) {
      simulator.pause();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
      simulator.reset();
      csv.reset();
      last_csv_elapsed = -1;
    }

    ImGui::End();

    ImGui::Render();

    int display_w = 0;
    int display_h = 0;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.10F, 0.10F, 0.12F, 1.00F);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}



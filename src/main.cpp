/*
 * @file main.cpp
 * @brief GUI版のメインエントリポイント
 *
 * このファイルは、GLFWとDear ImGuiを使用してGUIアプリケーションを初期化し、
 * シミュレーションの進行状況をリアルタイムで表示および操作します。
 * お茶の製造プロセスを視覚化し、ユーザーがシミュレーションパラメータを
 * 調整できるようにします。
 */

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

/*
 * @brief 値を [0, 1] にクランプします。
 *
 * @param v クランプする値
 * @return クランプされた値
 */
float clamp01(float v) {
  if (v < 0.0F) {
    return 0.0F;
  }
  if (v > 1.0F) {
    return 1.0F;
  }
  return v;
}

/*
 * @brief プログレスバー + 数値を同一行で描画します。
 *
 * UI 要件:
 *   - ImGui::ProgressBar を使用
 *   - 1 画面ダッシュボードで状態を可視化
 *
 * @param label ラベル文字列
 * @param fraction プログレスバーの割合 (0.0F-1.0F)
 * @param overlay プログレスバー上に表示するオーバーレイ文字列
 * @param width プログレスバーの幅
 */
void draw_bar(const char* label,
              float fraction,
              const char* overlay,
              float width) {
  ImGui::TextUnformatted(label);
  ImGui::SameLine(140.0F);
  ImGui::ProgressBar(clamp01(fraction), ImVec2(width, 0.0F), overlay);
}

} /* namespace */

/*
 * @brief GUIアプリケーションのメインエントリポイント
 *
 * GLFW、OpenGL、Dear ImGuiを初期化し、シミュレーションループを実行します。
 * シミュレーションの状態を更新し、GUIで表示します。
 *
 * @return 0 成功、1 失敗
 */
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
  int selected_batch = 0;
  int desired_batches = 1;

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

    desired_batches = simulator.batch_count();
    if (selected_batch >= simulator.batch_count()) {
      selected_batch = simulator.batch_count() - 1;
    }
    const tea_gui::TeaBatch& batch = simulator.batch_at(selected_batch);

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
    ImGui::Text("Batch: %d / %d", selected_batch + 1, simulator.batch_count());
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

    /*
      モデル選択:
      - 停止中のみ変更可能
      - 変更時は初期状態へリセットして適用します
    */
    int model_idx = 0;
    {
      const tea::ModelType m = simulator.model(); // 変更
      if (m == tea::ModelType::GENTLE) { // 変更
        model_idx = 1;
      } else if (m == tea::ModelType::AGGRESSIVE) { // 変更
        model_idx = 2;
      }
    }
    const char* model_items[] = {"default", "gentle", "aggressive"};
    ImGui::TextUnformatted("Model");
    ImGui::SameLine(140.0F);
    if (ImGui::Combo("##model", &model_idx, model_items, 3)) {
      if (!simulator.is_running()) {
        const tea::ModelType next = // 変更
            (model_idx == 1) ? tea::ModelType::GENTLE :
            (model_idx == 2) ? tea::ModelType::AGGRESSIVE :
                               tea::ModelType::DEFAULT; // 変更
        simulator.set_model(next);
      }
    }

    ImGui::TextUnformatted("Batches");
    ImGui::SameLine(140.0F);
    ImGui::SetNextItemWidth(120.0F);
    int tmp_batches = desired_batches;
    if (ImGui::InputInt("##batches", &tmp_batches)) {
      if (tmp_batches < 1) {
        tmp_batches = 1;
      }
      if (tmp_batches > 16) {
        tmp_batches = 16;
      }
      desired_batches = tmp_batches;
    }
    ImGui::SameLine();
    if (ImGui::Button("Apply") && !simulator.is_running()) {
      simulator.set_batch_count(desired_batches);
      selected_batch = 0;
      csv.reset();
      last_csv_elapsed = -1;
    }

    ImGui::TextUnformatted("Select");
    ImGui::SameLine(140.0F);
    int tmp_sel = selected_batch;
    if (ImGui::SliderInt("##batchsel", &tmp_sel, 0,
                         std::max(0, simulator.batch_count() - 1))) {
      selected_batch = tmp_sel;
    }

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

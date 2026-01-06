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
#include <algorithm>
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
  固定長のリングバッファ（履歴）です。
  UIでトレンド（PlotLines）を描くために直近の値を保持します。
*/
class HistoryBuffer final {
 public:
  /* 指定容量で初期化します。 */
  explicit HistoryBuffer(std::size_t capacity)
      : data_(capacity, 0.0F), capacity_(capacity) {}

  /* 全要素を0で初期化し、履歴を破棄します。 */
  void clear() {
    std::fill(data_.begin(), data_.end(), 0.0F);
    head_ = 0;
    size_ = 0;
  }

  /* 末尾へ追加します（満杯なら古い値から上書き）。 */
  void push(float v) {
    if (capacity_ == 0) {
      return;
    }
    data_[head_] = v;
    head_ = (head_ + 1) % capacity_;
    if (size_ < capacity_) {
      ++size_;
    }
  }

  /* 現在保持している要素数を返します。 */
  std::size_t size() const {
    return size_;
  }

  /* PlotLines 用の連続配列（内部バッファ）を返します。 */
  const float* data() const {
    return data_.data();
  }

  /* PlotLines 用のオフセット（先頭位置）を返します。 */
  int offset() const {
    return static_cast<int>(head_);
  }

 private:
  std::vector<float> data_;
  std::size_t capacity_ = 0;
  std::size_t head_ = 0;
  std::size_t size_ = 0;
};

/* 値を [min, max] にクランプします。 */
float clamp(float v, float min_v, float max_v) {
  if (v < min_v) {
    return min_v;
  }
  if (v > max_v) {
    return max_v;
  }
  return v;
}

/* 工程の総所要時間（秒）を返します（表示用固定値）。 */
int total_process_seconds() {
  return 120;
}

/* 経過秒から工程全体の進捗率 [0, 1] を返します。 */
float total_progress_fraction(int elapsed_seconds) {
  const int total = total_process_seconds();
  if (total <= 0) {
    return 0.0F;
  }
  return clamp01(static_cast<float>(elapsed_seconds) /
                 static_cast<float>(total));
}

/* 品質ステータスに応じた色を返します。 */
ImVec4 quality_color(const std::string& status) {
  if (status == "GOOD") {
    return ImVec4(0.10F, 0.80F, 0.45F, 1.00F);
  }
  if (status == "OK") {
    return ImVec4(0.95F, 0.75F, 0.15F, 1.00F);
  }
  return ImVec4(0.95F, 0.25F, 0.25F, 1.00F);
}

/* 工程状態に応じた色を返します。 */
ImVec4 process_color(tea_gui::ProcessState state) {
  if (state == tea_gui::ProcessState::STEAMING) {
    return ImVec4(0.35F, 0.70F, 0.95F, 1.00F);
  }
  if (state == tea_gui::ProcessState::ROLLING) {
    return ImVec4(0.75F, 0.55F, 0.95F, 1.00F);
  }
  if (state == tea_gui::ProcessState::DRYING) {
    return ImVec4(0.95F, 0.55F, 0.20F, 1.00F);
  }
  return ImVec4(0.60F, 0.60F, 0.65F, 1.00F);
}

/* ダッシュボード向けにテーマ/余白を調整します。 */
void apply_dashboard_style() {
  ImGuiStyle& style = ImGui::GetStyle();

  style.WindowRounding = 8.0F;
  style.FrameRounding = 6.0F;
  style.GrabRounding = 6.0F;
  style.ScrollbarRounding = 8.0F;
  style.TabRounding = 6.0F;

  style.WindowPadding = ImVec2(14.0F, 12.0F);
  style.FramePadding = ImVec2(10.0F, 6.0F);
  style.ItemSpacing = ImVec2(10.0F, 8.0F);
  style.ItemInnerSpacing = ImVec2(8.0F, 6.0F);
}

/* ラベルと値を1行で描画します。 */
void draw_kv_line(const char* label, const char* value, float label_width) {
  ImGui::TextUnformatted(label);
  ImGui::SameLine(label_width);
  ImGui::TextUnformatted(value);
}

/* 色付きのバッジ（チップ）を描画します。 */
void draw_badge(const char* text, const ImVec4& color) {
  ImGui::PushStyleColor(ImGuiCol_Text, color);
  ImGui::TextUnformatted(text);
  ImGui::PopStyleColor();
}

/*
  キーボードショートカットを処理します。
  入力中（テキスト入力フォーカス等）の誤動作を避けるため、
  WantTextInput の間は処理しません。
*/
void handle_shortcuts(const ImGuiIO& io, tea_gui::Simulator& simulator) {
  if (io.WantTextInput) {
    return;
  }

  if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
    if (simulator.is_running()) {
      simulator.pause();
    } else {
      simulator.start();
    }
  }

  if (ImGui::IsKeyPressed(ImGuiKey_R)) {
    simulator.reset();
  }
}

/* プログレスバー + 数値を同一行で描画します。 */
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
  io.IniFilename = "tea_factory_imgui.ini";
  io.FontGlobalScale = 1.05F;
  ImGui::StyleColorsDark();
  apply_dashboard_style();

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  tea_gui::Simulator simulator;
  std::optional<tea_io::CsvWriter> csv;
  int last_csv_elapsed = -1;
  int selected_batch = 0;
  int desired_batches = 1;
  int last_history_elapsed = -1;
  int last_history_selected_batch = -1;

  HistoryBuffer moisture_hist(180);
  HistoryBuffer temp_hist(180);
  HistoryBuffer aroma_hist(180);
  HistoryBuffer color_hist(180);
  HistoryBuffer score_hist(180);

  bool csv_enabled = true;
  char csv_path[256] = "tea_factory_gui.csv";

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

    handle_shortcuts(io, simulator);

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(920, 520), ImGuiCond_FirstUseEver);
    ImGui::Begin("TeaFactory Simulator", nullptr, ImGuiWindowFlags_NoCollapse);

    desired_batches = simulator.batch_count();
    if (selected_batch >= simulator.batch_count()) {
      selected_batch = simulator.batch_count() - 1;
    }
    const tea_gui::TeaBatch& batch = simulator.batch_at(selected_batch);
    const int elapsed = batch.elapsed_seconds();

    /*
      GUI版CSV出力:
      - Start時に tea_factory_gui.csv を新規作成（上書き）
      - 実行中、elapsedSeconds が進んだタイミング（=1秒ごと）で1行追記
    */
    if (csv.has_value() && elapsed != last_csv_elapsed) {
      last_csv_elapsed = elapsed;
      csv->write_row(tea_gui::to_string(batch.process()),
                     elapsed,
                     batch.moisture(),
                     batch.temperature_c(),
                     batch.aroma(),
                     batch.color());
    }

    /*
      履歴更新（トレンド表示用）:
      - 1秒ごとにサンプル
      - 選択バッチが変わったら履歴をリセット
    */
    if (selected_batch != last_history_selected_batch) {
      last_history_selected_batch = selected_batch;
      last_history_elapsed = -1;
      moisture_hist.clear();
      temp_hist.clear();
      aroma_hist.clear();
      color_hist.clear();
      score_hist.clear();
    }
    if (elapsed != last_history_elapsed) {
      last_history_elapsed = elapsed;
      moisture_hist.push(static_cast<float>(batch.moisture()));
      temp_hist.push(static_cast<float>(batch.temperature_c()));
      aroma_hist.push(static_cast<float>(batch.aroma()));
      color_hist.push(static_cast<float>(batch.color()));
      score_hist.push(static_cast<float>(batch.quality_score()));
    }

    const float total_prog = total_progress_fraction(elapsed);
    char total_overlay[64];
    std::snprintf(total_overlay, sizeof(total_overlay), "%ds / %ds",
                  elapsed, total_process_seconds());

    ImGui::TextUnformatted("Overview");
    ImGui::Separator();
    ImGui::ProgressBar(total_prog, ImVec2(-1.0F, 0.0F), total_overlay);
    ImGui::Spacing();

    char process_text[64];
    std::snprintf(process_text, sizeof(process_text), "%s",
                  tea_gui::to_string(batch.process()));
    char elapsed_text[64];
    std::snprintf(elapsed_text, sizeof(elapsed_text), "%d sec", elapsed);
    char batch_text[64];
    std::snprintf(batch_text, sizeof(batch_text), "%d / %d",
                  selected_batch + 1, simulator.batch_count());

    const float label_width = 160.0F;
    draw_kv_line("Current Process", process_text, label_width);
    ImGui::SameLine();
    draw_badge(simulator.is_running() ? "RUNNING" : "PAUSED",
               simulator.is_running()
                   ? ImVec4(0.20F, 0.85F, 0.55F, 1.00F)
                   : ImVec4(0.95F, 0.75F, 0.15F, 1.00F));
    draw_kv_line("Elapsed Time", elapsed_text, label_width);
    draw_kv_line("Batch", batch_text, label_width);
    ImGui::Spacing();

    if (ImGui::BeginTable("layout", 2,
                          ImGuiTableFlags_SizingStretchProp |
                              ImGuiTableFlags_BordersInnerV)) {
      ImGui::TableSetupColumn("Metrics", ImGuiTableColumnFlags_WidthStretch,
                              0.62F);
      ImGui::TableSetupColumn("Controls", ImGuiTableColumnFlags_WidthStretch,
                              0.38F);
      ImGui::TableNextRow();

      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted("Metrics");
      ImGui::Separator();

      const float bar_width = clamp(ImGui::GetContentRegionAvail().x - 20.0F,
                                    260.0F, 520.0F);

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

      ImGui::Spacing();
      ImGui::Separator();
      ImGui::Spacing();

      ImGui::TextUnformatted("Trends (last ~180s)");
      ImGui::Separator();

      const ImVec2 plot_size(-1.0F, 60.0F);
      ImGui::PlotLines("Moisture",
                       moisture_hist.data(),
                       static_cast<int>(moisture_hist.size()),
                       moisture_hist.offset(),
                       nullptr,
                       0.0F,
                       1.0F,
                       plot_size);

      ImGui::PlotLines("Temp (C)",
                       temp_hist.data(),
                       static_cast<int>(temp_hist.size()),
                       temp_hist.offset(),
                       nullptr,
                       0.0F,
                       100.0F,
                       plot_size);

      ImGui::PlotLines("Aroma",
                       aroma_hist.data(),
                       static_cast<int>(aroma_hist.size()),
                       aroma_hist.offset(),
                       nullptr,
                       0.0F,
                       100.0F,
                       plot_size);

      ImGui::PlotLines("Color",
                       color_hist.data(),
                       static_cast<int>(color_hist.size()),
                       color_hist.offset(),
                       nullptr,
                       0.0F,
                       100.0F,
                       plot_size);

      ImGui::PlotLines("Quality",
                       score_hist.data(),
                       static_cast<int>(score_hist.size()),
                       score_hist.offset(),
                       nullptr,
                       0.0F,
                       100.0F,
                       plot_size);

      ImGui::TextUnformatted("Quality");
      ImGui::Separator();

      const float score = static_cast<float>(batch.quality_score());
      char score_text[32];
      std::snprintf(score_text, sizeof(score_text), "%.0f", score);
      ImGui::ProgressBar(clamp01(score / 100.0F), ImVec2(-1.0F, 0.0F),
                         score_text);

      const std::string status = batch.quality_status();
      ImGui::TextUnformatted("Status");
      ImGui::SameLine();
      draw_badge(status.c_str(), quality_color(status));

      ImGui::Spacing();
      ImGui::TextUnformatted("Current Stage");
      ImGui::SameLine();
      draw_badge(tea_gui::to_string(batch.process()),
                 process_color(batch.process()));

      ImGui::TableSetColumnIndex(1);
      ImGui::TextUnformatted("Controls");
      ImGui::Separator();

      ImGui::TextUnformatted("Settings");
      ImGui::Separator();

      ImGui::Checkbox("CSV output", &csv_enabled);
      ImGui::BeginDisabled(simulator.is_running());
      ImGui::SetNextItemWidth(-1.0F);
      ImGui::InputText("CSV path", csv_path, sizeof(csv_path));
      ImGui::EndDisabled();
      if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) &&
          simulator.is_running()) {
        ImGui::SetTooltip("Pauseしてから変更できます。");
      }

      ImGui::Spacing();
      ImGui::TextUnformatted("Shortcuts");
      ImGui::Separator();
      ImGui::BulletText("Space: Start/Pause");
      ImGui::BulletText("R: Reset");
      ImGui::Spacing();

      /*
        モデル選択:
          - 停止中のみ変更可能
          - 変更時は初期状態へリセットして適用します
      */
      int model_idx = 0;
      {
        const tea_gui::ModelType m = simulator.model();
        if (m == tea_gui::ModelType::GENTLE) {
          model_idx = 1;
        } else if (m == tea_gui::ModelType::AGGRESSIVE) {
          model_idx = 2;
        }
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
      ImGui::EndDisabled();
      if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) &&
          simulator.is_running()) {
        ImGui::SetTooltip("Pauseしてから変更できます。");
      }

      ImGui::BeginDisabled(simulator.is_running());
      ImGui::TextUnformatted("Batches");
      ImGui::SameLine(label_width);
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
      if (ImGui::Button("Apply")) {
        simulator.set_batch_count(desired_batches);
        selected_batch = 0;
        csv.reset();
        last_csv_elapsed = -1;
      }
      ImGui::EndDisabled();
      if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) &&
          simulator.is_running()) {
        ImGui::SetTooltip("Pauseしてから変更できます。");
      }

      ImGui::TextUnformatted("Select");
      ImGui::SameLine(label_width);
      int tmp_sel = selected_batch;
      if (ImGui::SliderInt("##batchsel", &tmp_sel, 0,
                           std::max(0, simulator.batch_count() - 1))) {
        selected_batch = tmp_sel;
      }

      ImGui::Spacing();
      ImGui::Separator();
      ImGui::Spacing();

      if (ImGui::Button("Start", ImVec2(-1.0F, 0.0F))) {
        simulator.start();
        if (csv_enabled && !csv.has_value()) {
          csv.emplace(csv_path);
          csv->write_header();
        }
        last_csv_elapsed = -1;
      }

      if (ImGui::Button("Pause", ImVec2(-1.0F, 0.0F))) {
        simulator.pause();
      }

      if (ImGui::Button("Reset", ImVec2(-1.0F, 0.0F))) {
        simulator.reset();
        csv.reset();
        last_csv_elapsed = -1;
      }

      ImGui::EndTable();
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

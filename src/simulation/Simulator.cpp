#include "simulation/Simulator.h"

#include <algorithm>
#include <iomanip>
#include <ostream>
#include <string>

#include "domain/ProcessState.h"
#include "io/CsvWriter.h"
#include "process/DryingProcess.h"
#include "process/RollingProcess.h"
#include "process/SteamingProcess.h"

namespace tea {

/* 既定設定で構築します。 */
Simulator::Simulator() : config_() {
  build_default_stages();
}

/* 設定を指定して構築します。 */
Simulator::Simulator(SimulationConfig config) : config_(config) {
  build_default_stages();
}

/* 初期状態を設定します。 */
void Simulator::set_initial_leaf(const TeaLeaf& leaf) {
  leaf_ = leaf;
  normalize(leaf_);
}

/* 既定のステージ構成（蒸し→揉捻→乾燥）を作ります。 */
void Simulator::build_default_stages() {
  stages_.clear();
  const ModelParams model = make_model(config_.model);

  Stage steaming;
  steaming.process = std::make_unique<SteamingProcess>(model.steaming);
  steaming.duration_seconds = config_.steaming_seconds;
  stages_.push_back(std::move(steaming));

  Stage rolling;
  rolling.process = std::make_unique<RollingProcess>(model.rolling);
  rolling.duration_seconds = config_.rolling_seconds;
  stages_.push_back(std::move(rolling));

  Stage drying;
  drying.process = std::make_unique<DryingProcess>(model.drying);
  drying.duration_seconds = config_.drying_seconds;
  stages_.push_back(std::move(drying));
}

/* 全工程を実行し、各ステップの状態を出力します。 */
void Simulator::run(std::ostream& os) {
  run(os, nullptr);
}

/* CSV出力を伴って全工程を実行します。 */
void Simulator::run(std::ostream& os, ::tea_io::CsvWriter* csv) {
  elapsed_seconds_ = 0;

  for (const Stage& stage : stages_) {
    /*
      dt が工程時間で割り切れない場合でも、工程時間ぴったりで進むように
      残り時間に応じて最後のステップ幅を調整します。
    */
    const int dt = config_.dt_seconds;
    int remaining = stage.duration_seconds;

    while (remaining > 0) {
      const int step = std::min(dt, remaining);
      stage.process->apply_step(leaf_, step);
      elapsed_seconds_ += step;
      remaining -= step;

      log_step(os, stage.process->state(), elapsed_seconds_);
      if (csv != nullptr) {
        csv->write_row(to_string(stage.process->state()),
                       elapsed_seconds_,
                       leaf_.moisture,
                       leaf_.temperature_c,
                       leaf_.aroma,
                       leaf_.color);
      }
    }
  }
}

/* 1 ステップのログ行を出力します。 */
void Simulator::log_step(std::ostream& os,
                         ProcessState state,
                         int elapsed_seconds) {
  /*
    ログは「工程名 + 経過時間 + 状態量」を固定フォーマットで出します。
    例:
      [STEAMING] t=30s moisture=0.78 temp=95.0 aroma=40.0 color=10.0
  */
  const char* label = to_string(state);
  os << '[' << label << ']';
  {
    /*
      表示揃えのため、"[STEAMING]" を幅 11 とみなし、短い工程名は
      右側に空白を付けます。
    */
    const int used = static_cast<int>(2 + std::char_traits<char>::length(label));
    const int pad = std::max(0, 11 - used);
    os << std::string(static_cast<std::size_t>(pad), ' ');
  }
  os << "t=" << elapsed_seconds << "s ";

  os << std::fixed << std::setprecision(2);
  os << "moisture=" << leaf_.moisture << ' ';

  os << std::fixed << std::setprecision(1);
  os << "temp=" << leaf_.temperature_c << ' ';
  os << "aroma=" << leaf_.aroma << ' ';
  os << "color=" << leaf_.color << '\n';
}

} /* namespace tea */



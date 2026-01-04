/*
 * @file main.cpp
 * @brief CLI版のメインエントリポイント
 *
 * このファイルは、コマンドライン引数に基づいてシミュレーションを設定し、
 * お茶の製造プロセスをシミュレートし、その結果をコンソール出力および
 * CSVファイルに書き込みます。
 */

#include <iostream>
#include <optional>
#include <sstream>
#include <vector>

#include "cli/Args.h"
#include "io/CsvWriter.h"
#include "domain/Model.h"
#include "simulation/Simulator.h"

/*
 * @brief CLIアプリケーションのメインエントリポイント
 *
 * コマンドライン引数をパースし、シミュレーション設定を行います。
 * 複数のバッチを並行してシミュレートし、その進行状況をコンソールに表示し、
 * 必要に応じてCSVファイルに書き込みます。
 *
 * @param argc コマンドライン引数の数
 * @param argv コマンドライン引数の配列
 * @return 0 成功、2 引数エラー
 */
int main(int argc, char** argv) {
  const tea_cli::Args args = tea_cli::parse_args(argc, argv);
  if (args.error.has_value()) {
    std::cerr << "Error: " << *args.error << "\n\n";
    std::cerr << tea_cli::help_text();
    return 2;
  }
  if (args.show_help) {
    std::cout << tea_cli::help_text();
    return 0;
  }

  tea::SimulationConfig config;
  config.dt_seconds = args.dt_seconds;
  config.steaming_seconds = args.steaming_seconds;
  config.rolling_seconds = args.rolling_seconds;
  config.drying_seconds = args.drying_seconds;
  if (args.model == "gentle") {
    config.model = tea::ModelType::GENTLE;
  } else if (args.model == "aggressive") {
    config.model = tea::ModelType::AGGRESSIVE;
  } else {
    config.model = tea::ModelType::DEFAULT;
  }

  tea::Simulator simulator(config);

  /*
    複数バッチ:
    - 同一設定で複数の Simulator を進めます（擬似的な複数ライン）
    - ログは batch=<id> を付与して出します
    - CSVはバッチごとに別ファイルへ出力します（フォーマット互換性のため）
  */
  const int batches = args.batches;
  std::vector<tea::Simulator> sims;
  sims.reserve(static_cast<std::size_t>(batches));
  for (int i = 0; i < batches; ++i) {
    tea::Simulator s(config);
    tea::TeaLeaf leaf;
    /*
      バッチ差分は決定論的に小さく付与します（乱数は使わない）。
      例: moisture をバッチ番号に応じて僅かに変える。
    */
    leaf.moisture = tea::clamp(leaf.moisture - 0.01 * i, 0.0, 1.0);
    leaf.aroma = tea::clamp(leaf.aroma + 0.5 * i, 0.0, 100.0);
    leaf.color = tea::clamp(leaf.color + 0.3 * i, 0.0, 100.0);
    s.set_initial_leaf(leaf);
    sims.push_back(std::move(s));
  }

  std::vector<std::optional<te-io::CsvWriter>> csvs; // 修正
  csvs.resize(static_cast<std::size_t>(batches));
  if (args.csv_enabled) {
    for (int i = 0; i < batches; ++i) {
      std::ostringstream path;
      if (batches == 1) {
        path << args.csv_path;
      } else {
        path << "tea_factory_cli_batch_" << i << ".csv";
      }
      csvs[static_cast<std::size_t>(i)].emplace(path.str());
      csvs[static_cast<std::size_t>(i)]->write_header();
    }
  }

  bool any_running = true;
  while (any_running) {
    any_running = false;
    for (int i = 0; i < batches; ++i) {
      ::te-io::CsvWriter* csv_ptr = nullptr; // 修正
      if (args.csv_enabled) {
        csv_ptr = &(*csvs[static_cast<std::size_t>(i)]);
      }
      if (sims[static_cast<std::size_t>(i)].step(config.dt_seconds, csv_ptr)) {
        any_running = true;
        const tea::Simulator& s = sims[static_cast<std::size_t>(i)];
        const tea::TeaLeaf& st = s.leaf();
        std::cout << "[batch=" << i << "] ";
        std::cout << '[' << tea::to_string(s.current_process()) << "] ";
        std::cout << "t=" << s.elapsed_seconds() << "s ";
        std::cout.setf(std::ios::fixed);
        std::cout.precision(2);
        std::cout << "moisture=" << st.moisture << ' ';
        std::cout.precision(1);
        std::cout << "temp=" << st.temperature_c << ' ';
        std::cout << "aroma=" << st.aroma << ' ';
        std::cout << "color=" << st.color << '\n';
      }
    }
  }

  return 0;
}

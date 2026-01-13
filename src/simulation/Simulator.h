#pragma once

#include <iosfwd>
#include <memory>
#include <vector>

#include "domain/Model.h"
#include "domain/TeaLeaf.h"
#include "process/IProcess.h"

namespace tea_io {
class CsvWriter;
} /* namespace tea_io */

namespace tea {

/* シミュレーションの実行設定です。 */
struct SimulationConfig final {
  int dt_seconds = 1;         /* 時間刻み [s] */
  int steaming_seconds = 30;  /* 蒸し工程の時間 [s] */
  int rolling_seconds = 30;   /* 揉捻工程の時間 [s] */
  int drying_seconds = 60;    /* 乾燥工程の時間 [s] */
  ModelType model = ModelType::DEFAULT; /* モデル（係数セット） */
};

/* 製造工程シミュレーションを統括し、工程遷移とログ出力を行います。 */
class Simulator final {
 public:
  /* 既定設定でシミュレータを構築します。 */
  Simulator();

  /* 設定を指定してシミュレータを構築します。 */
  explicit Simulator(SimulationConfig config);

  /* 初期状態の茶葉を設定します。 */
  void set_initial_leaf(const TeaLeaf& leaf);

  /* 全工程（蒸し→揉捻→乾燥）を実行し、各ステップをログ出力します。 */
  void run(std::ostream& os);

  /* CSV出力を伴って全工程を実行します（csv が null の場合は無効）。 */
  void run(std::ostream& os, ::tea_io::CsvWriter* csv);

  /* 1 ステップ進めます。完了済みなら false を返します。 */
  bool step(int dt_seconds, ::tea_io::CsvWriter* csv);

  /* 現在工程を返します（完了時は FINISHED を返します）。 */
  ProcessState current_process() const;

  /* 現在の茶葉状態を返します。 */
  const TeaLeaf& leaf() const;

  /* 経過時間（秒）を返します。 */
  int elapsed_seconds() const;

 private:
  /* 工程と継続時間を束ねたステージです。 */
  struct Stage final {
    std::unique_ptr<IProcess> process;
    int duration_seconds = 0;
  };

  /* 既定のステージ構成を構築します。 */
  void build_default_stages();

  /* 1 行分のログを出力します。 */
  void log_step(std::ostream& os, ProcessState state, int elapsed_seconds);

  SimulationConfig config_;
  TeaLeaf leaf_;
  int elapsed_seconds_ = 0;
  std::vector<Stage> stages_;

  std::size_t stage_index_ = 0;
  int stage_remaining_seconds_ = 0;
};

} /* namespace tea */



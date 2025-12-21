#pragma once

#include <string>

namespace tea_gui {

/* GUI版のモデル種別です（係数セット切替用）。 */
enum class ModelType {
  DEFAULT,
  GENTLE,
  AGGRESSIVE
};

/* 表示用のモデル名を返します。 */
const char* to_string(ModelType type);

/* 製造工程の状態（GUI版）を表す列挙です。 */
enum class ProcessState {
  STEAMING,
  ROLLING,
  DRYING,
  FINISHED
};

/* 工程名を表示用文字列に変換します。 */
const char* to_string(ProcessState state);

/*
  茶葉1バッチの状態と、工程遷移・物性更新ロジックを保持します。
  UI からは「現在値を読む」「deltaTimeで更新する」だけにし、描画と分離します。
*/
class TeaBatch final {
 public:
  /* 既定の初期状態で構築します。 */
  TeaBatch();

  /* モデル（係数セット）を設定します。 */
  void set_model(ModelType type);

  /* 初期状態へ戻します。 */
  void reset();

  /* deltaTime（秒）だけ状態を進めます。 */
  void update(double delta_seconds);

  /* 現在工程を返します。 */
  ProcessState process() const;

  /* 経過時間（秒）を返します。 */
  int elapsed_seconds() const;

  /* 各状態量を返します。 */
  double moisture() const;
  double temperature_c() const;
  double aroma() const;
  double color() const;

  /* FINISHED のときに品質スコアを返します（それ以外は現在値で算出）。 */
  double quality_score() const;

  /* 品質ステータス（GOOD/OK/BAD）を返します。 */
  std::string quality_status() const;

 private:
  /* 値域へクランプします。 */
  void normalize();

  /* 現在工程の残り時間（秒）を返します。 */
  double stage_remaining_seconds() const;

  /* 工程の閾値で次工程へ進めます。 */
  void advance_stage_if_needed();

  ModelType model_ = ModelType::DEFAULT;

  ProcessState process_ = ProcessState::STEAMING;
  double stage_elapsed_seconds_ = 0.0;
  double total_elapsed_seconds_ = 0.0;

  double moisture_ = 0.75;
  double temperature_c_ = 25.0;
  double aroma_ = 10.0;
  double color_ = 10.0;

  double quality_score_final_ = 0.0;
};

} /* namespace tea_gui */



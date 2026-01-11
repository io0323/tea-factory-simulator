/*
 * @file TeaBatch.h
 * @brief お茶のバッチ処理と状態管理の定義
 *
 * このファイルは、お茶の製造プロセスにおける単一のバッチの状態を管理する
 * TeaBatchクラスのインターフェースを定義しています。各工程での状態変化、
 * 品質スコア、および工程遷移ロジックが含まれます。
 */

#pragma once

#include <string>
#include <memory> // For std::unique_ptr

// GUI版のモデル種別と工程状態の定義をdomainからインクルード
#include "domain/Model.h"
#include "domain/ProcessState.h"
#include "process/IProcess.h" // For IProcess
#include "domain/TeaLeaf.h" // For tea::TeaLeaf

namespace tea_gui {

// tea::ModelTypeとtea::ProcessStateはdomain/Model.hとdomain/ProcessState.hで定義されているため、ここでは不要。
// ただし、to_string関数はtea_gui::名前空間に属するため、ここではそのまま残します。
const char* to_string(tea::ModelType type);
const char* to_string(tea::ProcessState state);


/*
  茶葉1バッチの状態と、工程遷移・物性更新ロジックを保持します。
  UI からは「現在値を読む」「deltaTimeで更新する」だけにし、描画と分離します。
*/
class TeaBatch final {
 public:
  /* 既定の初期状態で構築します。 */
  TeaBatch();

  /* モデル（係数セット）を設定します。 */
  void set_model(tea::ModelType type);

  /* 初期状態へ戻します。 */
  void reset();

  /* deltaTime（秒）だけ状態を進めます。 */
  void update(double delta_seconds);

  /* 現在工程を返します。 */
  tea::ProcessState process() const;

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

  tea::ModelType model_ = tea::ModelType::DEFAULT;
  tea::ModelParams model_params_;
  
  std::unique_ptr<tea::IProcess> current_process_handler_;
  
  double stage_elapsed_seconds_ = 0.0;
  double total_elapsed_seconds_ = 0.0;
  /*
    GUI側はフレームdt（小数）で update() を呼ぶため、
    1秒未満を蓄積して「整数秒になった分だけ」工程ロジックへ反映します。
  */
  double pending_seconds_ = 0.0;

  tea::TeaLeaf leaf_; // 追加

  double quality_score_final_ = 0.0;
  /* FINISHED 到達時の品質スコアを確定したかどうかを保持します。 */
  bool has_quality_score_final_ = false;
};

} /* namespace tea_gui */

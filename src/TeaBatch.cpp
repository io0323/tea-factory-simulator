/*
 * @file TeaBatch.cpp
 * @brief お茶のバッチ処理と状態管理
 *
 * このファイルは、お茶の製造プロセスにおける単一のバッチの状態を管理する
 * TeaBatchクラスの実装を含んでいます。各工程（蒸し、揉捻、乾燥）での
 * お茶の状態変化（水分、温度、香気、色）をシミュレートし、
 * 品質スコアとステータスを計算します。
 */

#include "TeaBatch.h"

#include <algorithm> // For std::clamp, std::max
#include <cmath>     // For std::exp, std::floor
#include <string>    // For std::string (used in quality_status)

#include "process/SteamingProcess.h" // For SteamingProcess
#include "process/RollingProcess.h"  // For RollingProcess
#include "process/DryingProcess.h"   // For DryingProcess

namespace tea_gui {

/*
 * @brief モデルタイプを表示用の文字列に変換します。
 *
 * @param type 変換するModelType
 * @return モデル名を表す文字列
 */
const char* to_string(tea::ModelType type) {
  switch (type) {
    case tea::ModelType::DEFAULT:
      return "default";
    case tea::ModelType::GENTLE:
      return "gentle";
    case tea::ModelType::AGGRESSIVE:
      return "aggressive";
  }
  return "unknown";
}

/*
 * @brief 工程名を表示用文字列に変換します。
 *
 * @param state 変換するProcessState
 * @return 工程名を表す文字列
 */
const char* to_string(tea::ProcessState state) {
  switch (state) {
    case tea::ProcessState::STEAMING:
      return "STEAMING";
    case tea::ProcessState::ROLLING:
      return "ROLLING";
    case tea::ProcessState::DRYING:
      return "DRYING";
    case tea::ProcessState::FINISHED:
      return "FINISHED";
  }
  return "UNKNOWN";
}

/*
 * @brief TeaBatchの既定コンストラクタです。
 *
 * 初期状態にリセットして構築します。
 */
TeaBatch::TeaBatch() {
  set_model(model_); // set_modelでcurrent_process_handler_が初期化される
  reset();
}

/*
 * @brief シミュレーションモデル（係数セット）を設定します。
 *
 * @param type 設定するモデルのタイプ
 */
void TeaBatch::set_model(tea::ModelType type) {
  model_ = type;
  model_params_ = tea::make_model(model_);

  // プロセスハンドラを新しいモデルパラメータで初期化
  if (current_process_handler_ == nullptr || current_process_handler_->state() == tea::ProcessState::STEAMING) {
    current_process_handler_ = std::make_unique<tea::SteamingProcess>(model_params_.steaming);
  } else if (current_process_handler_->state() == tea::ProcessState::ROLLING) {
    current_process_handler_ = std::make_unique<tea::RollingProcess>(model_params_.rolling);
  } else if (current_process_handler_->state() == tea::ProcessState::DRYING) {
    current_process_handler_ = std::make_unique<tea::DryingProcess>(model_params_.drying);
  } else {
    // FINISHED状態の場合など、現在のプロセスを維持
  }
}

/*
 * @brief バッチを初期状態へリセットします。
 *
 * 工程、経過時間、お茶の状態量（水分、温度、香気、色）を初期値に戻し、
 * 品質スコアをリセットします。
 */
void TeaBatch::reset() {
  stage_elapsed_seconds_ = 0.0;
  total_elapsed_seconds_ = 0.0;
  pending_seconds_ = 0.0;

  leaf_ = tea::TeaLeaf(); // TeaLeafを初期化

  quality_score_final_ = 0.0;
  has_quality_score_final_ = false;
  normalize();
  
  // プロセスをSTEAMINGにリセット
  current_process_handler_ = std::make_unique<tea::SteamingProcess>(model_params_.steaming);
}

/*
 * @brief deltaTime（秒）だけバッチの状態を進めます。
 *
 * 現在の工程に基づいてお茶の状態量（水分、温度、香気、色）を更新し、
 * 必要に応じて次の工程へ移行します。
 *
 * @param delta_seconds 更新する時間間隔（秒）
 */
void TeaBatch::update(double delta_seconds) {
  if (current_process_handler_ == nullptr || current_process_handler_->state() == tea::ProcessState::FINISHED) {
    return;
  }

  pending_seconds_ += std::max(0.0, delta_seconds);
  int remaining_seconds =
      static_cast<int>(std::floor(pending_seconds_));
  pending_seconds_ -= static_cast<double>(remaining_seconds);
  if (remaining_seconds <= 0) {
    return;
  }

  /*
    小数dtは蓄積し、整数秒になった分だけ状態更新します。
    さらに工程境界を跨ぐ場合は「工程残り時間で分割」して適用します。
  */
  while (remaining_seconds > 0 &&
         current_process_handler_ != nullptr &&
         current_process_handler_->state() != tea::ProcessState::FINISHED) {
    const int stage_remaining =
        static_cast<int>(stage_remaining_seconds());
    const int step_seconds = std::min(remaining_seconds, stage_remaining);
    if (step_seconds <= 0) {
      break;
    }

    stage_elapsed_seconds_ += static_cast<double>(step_seconds);
    total_elapsed_seconds_ += static_cast<double>(step_seconds);

    current_process_handler_->apply_step(leaf_, step_seconds);
    normalize();
    advance_stage_if_needed();

    remaining_seconds -= step_seconds;
  }

  if (!has_quality_score_final_ &&
      (current_process_handler_ == nullptr ||
       current_process_handler_->state() == tea::ProcessState::FINISHED)) {
    quality_score_final_ = quality_score();
    has_quality_score_final_ = true;
  }
}

/*
 * @brief 現在の工程を返します。
 *
 * @return 現在のProcessState
 */
tea::ProcessState TeaBatch::process() const {
  if (current_process_handler_ == nullptr) {
    return tea::ProcessState::FINISHED; // エラーハンドリングまたはデフォルト値
  }
  return current_process_handler_->state();
}

/*
 * @brief シミュレーションの経過時間（秒）を返します。
 *
 * @return 経過時間（秒）
 */
int TeaBatch::elapsed_seconds() const {
  return static_cast<int>(std::floor(total_elapsed_seconds_));
}

/*
 * @brief 現在の水分量を返します。
 *
 * @return 水分量 (0.0 - 1.0)
 */
double TeaBatch::moisture() const {
  return leaf_.moisture;
}

/*
 * @brief 現在の温度（摂氏）を返します。
 *
 * @return 温度 (摂氏)
 */
double TeaBatch::temperature_c() const {
  return leaf_.temperature_c;
}

/*
 * @brief 現在の香気を返します。
 *
 * @return 香気 (0.0 - 100.0)
 */
double TeaBatch::aroma() const {
  return leaf_.aroma;
}

/*
 * @brief 現在の色を返します。
 *
 * @return 色 (0.0 - 100.0)
 */
double TeaBatch::color() const {
  return leaf_.color;
}

/*
 * @brief 品質スコア（0-100）を要件式で算出します。
 *
 * バッチが終了している場合は最終品質スコアを返し、
 * それ以外の場合は現在の状態からスコアを算出します。
 *
 * 要件式:
 *   qualityScore = aroma * 0.4
 *                + color * 0.4
 *                + (1.0 - moisture) * 100 * 0.2
 *
 * @return 算出された品質スコア
 */
double TeaBatch::quality_score() const {
  /*
    要件式:
      qualityScore = aroma * 0.4
                   + color * 0.4
                   + (1.0 - moisture) * 100 * 0.2
  */
  const double score =
      leaf_.aroma * 0.4 + leaf_.color * 0.4 + (1.0 - leaf_.moisture) * 100.0 * 0.2;
  return std::clamp(score, 0.0, 100.0);
}

/*
 * @brief 品質ステータス（GOOD/OK/BAD）を返します。
 *
 * 品質スコアに基づいて、GOOD (80以上), OK (60以上), BAD (それ未満) の
 * いずれかのステータスを返します。
 *
 * @return 品質ステータスを表す文字列
 */
std::string TeaBatch::quality_status() const {
  const bool is_finished =
      (current_process_handler_ == nullptr) ||
      (current_process_handler_->state() == tea::ProcessState::FINISHED);
  const double score =
      (is_finished && has_quality_score_final_) ? quality_score_final_
                                                : quality_score();
  if (score >= 80.0) {
    return "GOOD";
  }
  if (score >= 60.0) {
    return "OK";
  }
  return "BAD";
}

/*
 * @brief お茶の状態量を有効な値域へクランプします。
 *
 * 水分、香気、色の値がそれぞれの最小値・最大値の範囲に収まるように調整します。
 */
void TeaBatch::normalize() {
  leaf_.moisture = std::clamp(leaf_.moisture, 0.0, 1.0);
  leaf_.aroma = std::clamp(leaf_.aroma, 0.0, 100.0);
  leaf_.color = std::clamp(leaf_.color, 0.0, 100.0);
}

/*
 * @brief 現在工程の残り時間（秒）を返します。
 *
 * 各工程の固定時間に基づいて、現在の工程の残り時間を計算します。
 *
 * @return 現在工程の残り時間（秒）
 */
double TeaBatch::stage_remaining_seconds() const {
  /*
    工程時間は固定（例示）とし、シンプルにします。
    STEAMING: 30s, ROLLING: 30s, DRYING: 60s
  */
  const double steam_s = 30.0;
  const double roll_s = 30.0;
  const double dry_s = 60.0;

  if (current_process_handler_ == nullptr || current_process_handler_->state() == tea::ProcessState::STEAMING) {
    return std::max(0.0, steam_s - stage_elapsed_seconds_);
  }
  if (current_process_handler_->state() == tea::ProcessState::ROLLING) {
    return std::max(0.0, roll_s - stage_elapsed_seconds_);
  }
  if (current_process_handler_->state() == tea::ProcessState::DRYING) {
    return std::max(0.0, dry_s - stage_elapsed_seconds_);
  }
  return 0.0;
}

/*
 * @brief 工程の閾値に達した場合、次の工程へ進めます。
 *
 * 現在の工程の残り時間が0以下になった場合、次の工程へ移行します。
 * 最終工程の場合はFINISHED状態へ移行します。
 */
void TeaBatch::advance_stage_if_needed() {
  if (current_process_handler_ == nullptr || current_process_handler_->state() == tea::ProcessState::FINISHED) {
    return;
  }

  if (stage_remaining_seconds() > 0.0) {
    return;
  }

  stage_elapsed_seconds_ = 0.0;

  if (current_process_handler_->state() == tea::ProcessState::STEAMING) {
    current_process_handler_ = std::make_unique<tea::RollingProcess>(model_params_.rolling);
  } else if (current_process_handler_->state() == tea::ProcessState::ROLLING) {
    current_process_handler_ = std::make_unique<tea::DryingProcess>(model_params_.drying);
  } else if (current_process_handler_->state() == tea::ProcessState::DRYING) {
    current_process_handler_.reset(); // FINISHED状態
  }
}

} /* namespace tea_gui */

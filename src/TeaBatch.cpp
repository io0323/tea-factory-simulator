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
#include <cmath>     // For std::floor
#include <string>    // For std::string (used in quality_status)

#include "process/SteamingProcess.h" // For SteamingProcess
#include "process/RollingProcess.h"  // For RollingProcess
#include "process/DryingProcess.h"   // For DryingProcess

namespace tea_gui {

namespace {

/*
 * @brief GUI版で用いる既定の工程時間（秒）です。
 */
constexpr int kSteamingSeconds = 30;
constexpr int kRollingSeconds = 30;
constexpr int kDryingSeconds = 60;

/*
 * @brief 小数dtの蓄積で 1.0 に届かないケースを吸収する許容誤差です。
 *
 * 例: 0.1 を10回足した結果が 0.999999999... になるケースでも、
 * 1秒ステップが発火するようにします。
 */
constexpr double kTimeAccumulatorEpsilon = 1e-9;

} /* namespace */

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
 * @brief 工程の既定所要時間（秒）を返します。
 *
 * @param state 工程状態
 * @return 既定所要時間（秒）
 */
int TeaBatch::default_stage_seconds(tea::ProcessState state) {
  if (state == tea::ProcessState::STEAMING) {
    return kSteamingSeconds;
  }
  if (state == tea::ProcessState::ROLLING) {
    return kRollingSeconds;
  }
  if (state == tea::ProcessState::DRYING) {
    return kDryingSeconds;
  }
  return 0;
}

/*
 * @brief シミュレーションモデル（係数セット）を設定します。
 *
 * @param type 設定するモデルのタイプ
 */
void TeaBatch::set_model(tea::ModelType type) {
  model_ = type;
  model_params_ = tea::make_model(model_);

  /*
    停止中にモデルが変わるケースを想定し、工程状態は維持したまま
    パラメータだけ差し替えます。
    FINISHED ならハンドラが無いので何もしません。
  */
  if (current_process_handler_ == nullptr) {
    return;
  }

  const tea::ProcessState state = current_process_handler_->state();
  if (state == tea::ProcessState::STEAMING) {
    current_process_handler_ =
        std::make_unique<tea::SteamingProcess>(model_params_.steaming);
  } else if (state == tea::ProcessState::ROLLING) {
    current_process_handler_ =
        std::make_unique<tea::RollingProcess>(model_params_.rolling);
  } else if (state == tea::ProcessState::DRYING) {
    current_process_handler_ =
        std::make_unique<tea::DryingProcess>(model_params_.drying);
  }
}

/*
 * @brief バッチを初期状態へリセットします。
 *
 * 工程、経過時間、お茶の状態量（水分、温度、香気、色）を初期値に戻し、
 * 品質スコアをリセットします。
 */
void TeaBatch::reset() {
  time_accumulator_seconds_ = 0.0;
  elapsed_seconds_ = 0;
  stage_remaining_seconds_ = kSteamingSeconds;

  leaf_ = tea::TeaLeaf(); // TeaLeafを初期化

  has_quality_score_final_ = false;
  quality_score_final_ = 0.0;
  tea::normalize(leaf_);
  
  // プロセスをSTEAMINGにリセット
  current_process_handler_ =
      std::make_unique<tea::SteamingProcess>(model_params_.steaming);
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
  if (current_process_handler_ == nullptr) {
    return;
  }

  /*
    GUI版はフレーム単位で dt が渡されるため、小数秒を蓄積して 1 秒単位で
    工程へ適用します。工程境界を跨ぐ場合は残り時間で分割します。
  */
  time_accumulator_seconds_ += std::max(0.0, delta_seconds);

  while (time_accumulator_seconds_ + kTimeAccumulatorEpsilon >= 1.0) {
    if (current_process_handler_ == nullptr) {
      break;
    }

    if (stage_remaining_seconds_ <= 0) {
      stage_remaining_seconds_ =
          default_stage_seconds(current_process_handler_->state());
    }

    const int available =
        static_cast<int>(std::floor(time_accumulator_seconds_ +
                                    kTimeAccumulatorEpsilon));
    if (available <= 0) {
      break;
    }
    const int step = std::min(available, stage_remaining_seconds_);

    current_process_handler_->apply_step(leaf_, step);
    tea::normalize(leaf_);

    elapsed_seconds_ += step;
    stage_remaining_seconds_ -= step;
    time_accumulator_seconds_ =
        std::max(0.0, time_accumulator_seconds_ - static_cast<double>(step));

    if (stage_remaining_seconds_ > 0) {
      continue;
    }

    const tea::ProcessState state = current_process_handler_->state();
    if (state == tea::ProcessState::STEAMING) {
      current_process_handler_ =
          std::make_unique<tea::RollingProcess>(model_params_.rolling);
      stage_remaining_seconds_ = kRollingSeconds;
      continue;
    }
    if (state == tea::ProcessState::ROLLING) {
      current_process_handler_ =
          std::make_unique<tea::DryingProcess>(model_params_.drying);
      stage_remaining_seconds_ = kDryingSeconds;
      continue;
    }
    if (state == tea::ProcessState::DRYING) {
      current_process_handler_.reset();
      stage_remaining_seconds_ = 0;
      time_accumulator_seconds_ = 0.0;

      if (!has_quality_score_final_) {
        quality_score_final_ = quality_score();
        has_quality_score_final_ = true;
      }
      break;
    }
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
  return elapsed_seconds_;
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
  if (process() == tea::ProcessState::FINISHED && has_quality_score_final_) {
    return quality_score_final_;
  }
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
      (process() == tea::ProcessState::FINISHED && has_quality_score_final_)
          ? quality_score_final_
          : quality_score();
  if (score >= 80.0) {
    return "GOOD";
  }
  if (score >= 60.0) {
    return "OK";
  }
  return "BAD";
}

} /* namespace tea_gui */

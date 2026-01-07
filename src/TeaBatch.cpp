#include "TeaBatch.h"

#include <algorithm>
#include <cmath>

namespace tea_gui {

namespace {

/*
 * @brief モデル種別に応じた係数倍率を返します。
 *
 * GUI版の工程ロジックは読みやすさ重視で係数を直書きしますが、モデル差分は
 * 「一部係数だけがDEFAULTのまま」という不整合が出やすいので倍率で統一します。
 *
 * @param type モデル種別
 * @return 係数倍率（DEFAULT=1）
 */
double model_scale(ModelType type) {
  if (type == ModelType::GENTLE) {
    return 0.75;
  }
  if (type == ModelType::AGGRESSIVE) {
    return 1.25;
  }
  return 1.0;
}

} /* namespace */

/* 表示用のモデル名を返します。 */
const char* to_string(ModelType type) {
  switch (type) {
    case ModelType::DEFAULT:
      return "default";
    case ModelType::GENTLE:
      return "gentle";
    case ModelType::AGGRESSIVE:
      return "aggressive";
  }
  return "unknown";
}

/* 工程名を表示用文字列に変換します。 */
const char* to_string(ProcessState state) {
  switch (state) {
    case ProcessState::STEAMING:
      return "STEAMING";
    case ProcessState::ROLLING:
      return "ROLLING";
    case ProcessState::DRYING:
      return "DRYING";
    case ProcessState::FINISHED:
      return "FINISHED";
  }
  return "UNKNOWN";
}

/* 既定の初期状態で構築します。 */
TeaBatch::TeaBatch() {
  reset();
}

/* モデル（係数セット）を設定します。 */
void TeaBatch::set_model(ModelType type) {
  model_ = type;
}

/* 初期状態へ戻します。 */
void TeaBatch::reset() {
  process_ = ProcessState::STEAMING;
  stage_elapsed_seconds_ = 0.0;
  total_elapsed_seconds_ = 0.0;

  moisture_ = 0.75;
  temperature_c_ = 25.0;
  aroma_ = 10.0;
  color_ = 10.0;

  quality_score_final_ = 0.0;
  normalize();
}

/* deltaTime（秒）だけ状態を進めます。 */
void TeaBatch::update(double delta_seconds) {
  if (process_ == ProcessState::FINISHED) {
    return;
  }

  /*
    1フレームで工程残り時間を超える dt が来ることがあります。
    その場合、超過分は次工程へ繰り越さないと工程境界の挙動が不連続になります。
  */
  double remaining_dt = std::max(0.0, delta_seconds);
  const double k = model_scale(model_);

  while (remaining_dt > 0.0 && process_ != ProcessState::FINISHED) {
    const double remaining_stage = stage_remaining_seconds();
    if (remaining_stage <= 0.0) {
      advance_stage_if_needed();
      continue;
    }

    const double step = std::min(remaining_dt, remaining_stage);
    stage_elapsed_seconds_ += step;
    total_elapsed_seconds_ += step;
    remaining_dt -= step;

    /*
      状態更新は「工程ごとに連続時間の微小更新」を行います。
      - STEAMING: 緩和 + 微加湿 + 飽和増加
      - ROLLING: 冷却緩和 + 脱水 + 飽和増加
      - DRYING: 指数乾燥 + 過熱時の香気劣化
    */
    if (process_ == ProcessState::STEAMING) {
      const double target_temp_c = 95.0;
      const double heat_k = 0.08 * k;
      const double moisture_gain_per_s = 0.0008 * k;
      const double aroma_gain_per_s = 1.0 * k;
      const double color_gain_per_s = 0.2 * k;

      temperature_c_ += (target_temp_c - temperature_c_) * heat_k * step;
      moisture_ += moisture_gain_per_s * step;
      aroma_ += aroma_gain_per_s * step * (1.0 - aroma_ / 100.0);
      color_ += color_gain_per_s * step * (1.0 - color_ / 100.0);
    } else if (process_ == ProcessState::ROLLING) {
      const double target_temp_c = 70.0;
      const double cool_k = 0.05 * k;
      const double moisture_loss_k = 0.0015 * k;
      const double aroma_gain_per_s = 0.6 * k;
      const double color_gain_per_s = 0.3 * k;

      temperature_c_ += (target_temp_c - temperature_c_) * cool_k * step;
      moisture_ -= moisture_loss_k * step * (0.4 + 0.6 * moisture_);
      aroma_ += aroma_gain_per_s * step * (1.0 - aroma_ / 100.0);
      color_ += color_gain_per_s * step * (1.0 - color_ / 100.0);
    } else if (process_ == ProcessState::DRYING) {
      const double target_temp_c = 60.0;
      const double temp_k = 0.07 * k;
      const double dry_k = 0.05 * k;
      const double aroma_recover_per_s = 0.2 * k;
      const double overheat_c = 70.0;
      const double aroma_damage_k = 0.02 * k;
      const double color_gain_per_s = 0.15 * k;

      temperature_c_ += (target_temp_c - temperature_c_) * temp_k * step;
      moisture_ *= std::exp(-dry_k * step);

      if (temperature_c_ > overheat_c) {
        aroma_ -= aroma_damage_k * (temperature_c_ - overheat_c) * step;
      } else {
        aroma_ += aroma_recover_per_s * step * (1.0 - aroma_ / 100.0);
      }

      color_ += color_gain_per_s * step * (1.0 - color_ / 100.0);
    }

    normalize();
    advance_stage_if_needed();
  }

  if (process_ == ProcessState::FINISHED && quality_score_final_ == 0.0) {
    quality_score_final_ = quality_score();
  }
}

/* 現在工程を返します。 */
ProcessState TeaBatch::process() const {
  return process_;
}

/* 経過時間（秒）を返します。 */
int TeaBatch::elapsed_seconds() const {
  return static_cast<int>(std::floor(total_elapsed_seconds_));
}

/* 各状態量を返します。 */
double TeaBatch::moisture() const {
  return moisture_;
}

double TeaBatch::temperature_c() const {
  return temperature_c_;
}

double TeaBatch::aroma() const {
  return aroma_;
}

double TeaBatch::color() const {
  return color_;
}

/* FINISHED のときに品質スコアを返します（それ以外は現在値で算出）。 */
double TeaBatch::quality_score() const {
  /*
    要件式:
      qualityScore = aroma * 0.4
                   + color * 0.4
                   + (1.0 - moisture) * 100 * 0.2
  */
  const double score =
      aroma_ * 0.4 + color_ * 0.4 + (1.0 - moisture_) * 100.0 * 0.2;
  return std::clamp(score, 0.0, 100.0);
}

/* 品質ステータス（GOOD/OK/BAD）を返します。 */
std::string TeaBatch::quality_status() const {
  const double score =
      (process_ == ProcessState::FINISHED) ? quality_score_final_
                                           : quality_score();
  if (score >= 80.0) {
    return "GOOD";
  }
  if (score >= 60.0) {
    return "OK";
  }
  return "BAD";
}

/* 値域へクランプします。 */
void TeaBatch::normalize() {
  moisture_ = std::clamp(moisture_, 0.0, 1.0);
  aroma_ = std::clamp(aroma_, 0.0, 100.0);
  color_ = std::clamp(color_, 0.0, 100.0);
}

/* 現在工程の残り時間（秒）を返します。 */
double TeaBatch::stage_remaining_seconds() const {
  /*
    工程時間は固定（例示）とし、シンプルにします。
    STEAMING: 30s, ROLLING: 30s, DRYING: 60s
  */
  const double steam_s = 30.0;
  const double roll_s = 30.0;
  const double dry_s = 60.0;

  if (process_ == ProcessState::STEAMING) {
    return std::max(0.0, steam_s - stage_elapsed_seconds_);
  }
  if (process_ == ProcessState::ROLLING) {
    return std::max(0.0, roll_s - stage_elapsed_seconds_);
  }
  if (process_ == ProcessState::DRYING) {
    return std::max(0.0, dry_s - stage_elapsed_seconds_);
  }
  return 0.0;
}

/* 工程の閾値で次工程へ進めます。 */
void TeaBatch::advance_stage_if_needed() {
  if (process_ == ProcessState::FINISHED) {
    return;
  }

  if (stage_remaining_seconds() > 0.0) {
    return;
  }

  stage_elapsed_seconds_ = 0.0;

  if (process_ == ProcessState::STEAMING) {
    process_ = ProcessState::ROLLING;
  } else if (process_ == ProcessState::ROLLING) {
    process_ = ProcessState::DRYING;
  } else if (process_ == ProcessState::DRYING) {
    process_ = ProcessState::FINISHED;
  }
}

} /* namespace tea_gui */



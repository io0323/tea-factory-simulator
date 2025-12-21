#include "TeaBatch.h"

#include <algorithm>
#include <cmath>

namespace tea_gui {

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

  const double dt = std::max(0.0, delta_seconds);
  stage_elapsed_seconds_ += dt;
  total_elapsed_seconds_ += dt;

  /*
    状態更新は「工程ごとに連続時間の微小更新」を行います。
    ここでは複雑な物理モデルは避け、読みやすい決定論式にします。

    - STEAMING:
      温度は目標値へ緩和（一次遅れ）
      moisture は微増、aroma は飽和しながら増加

    - ROLLING:
      温度は目標値へ緩和（冷却寄り）
      moisture は減少（含水率が高いほど抜けやすい）
      aroma/color は飽和しながら緩やかに増加

    - DRYING:
      moisture は指数減衰（乾くほど減りにくい）
      過熱時は aroma が劣化
  */
  if (process_ == ProcessState::STEAMING) {
    double target_temp_c = 95.0;
    double heat_k = 0.08;
    double moisture_gain_per_s = 0.0008;
    double aroma_gain_per_s = 1.0;
    double color_gain_per_s = 0.2;

    if (model_ == ModelType::GENTLE) {
      heat_k = 0.05;
      moisture_gain_per_s = 0.0006;
      aroma_gain_per_s = 0.7;
    } else if (model_ == ModelType::AGGRESSIVE) {
      heat_k = 0.11;
      moisture_gain_per_s = 0.0011;
      aroma_gain_per_s = 1.4;
    }

    temperature_c_ += (target_temp_c - temperature_c_) * heat_k * dt;
    moisture_ += moisture_gain_per_s * dt;
    aroma_ += aroma_gain_per_s * dt * (1.0 - aroma_ / 100.0);
    color_ += color_gain_per_s * dt * (1.0 - color_ / 100.0);
  } else if (process_ == ProcessState::ROLLING) {
    double target_temp_c = 70.0;
    double cool_k = 0.05;
    double moisture_loss_k = 0.0015;
    double aroma_gain_per_s = 0.6;
    double color_gain_per_s = 0.3;

    if (model_ == ModelType::GENTLE) {
      cool_k = 0.04;
      moisture_loss_k = 0.0010;
      aroma_gain_per_s = 0.4;
    } else if (model_ == ModelType::AGGRESSIVE) {
      cool_k = 0.07;
      moisture_loss_k = 0.0022;
      aroma_gain_per_s = 0.9;
    }

    temperature_c_ += (target_temp_c - temperature_c_) * cool_k * dt;
    moisture_ -= moisture_loss_k * dt * (0.4 + 0.6 * moisture_);
    aroma_ += aroma_gain_per_s * dt * (1.0 - aroma_ / 100.0);
    color_ += color_gain_per_s * dt * (1.0 - color_ / 100.0);
  } else if (process_ == ProcessState::DRYING) {
    double target_temp_c = 60.0;
    double temp_k = 0.07;
    double dry_k = 0.05;
    double aroma_recover_per_s = 0.2;
    double overheat_c = 70.0;
    double aroma_damage_k = 0.02;
    double color_gain_per_s = 0.15;

    if (model_ == ModelType::GENTLE) {
      temp_k = 0.06;
      dry_k = 0.035;
      aroma_damage_k = 0.015;
    } else if (model_ == ModelType::AGGRESSIVE) {
      temp_k = 0.09;
      dry_k = 0.07;
      aroma_damage_k = 0.03;
    }

    temperature_c_ += (target_temp_c - temperature_c_) * temp_k * dt;
    moisture_ *= std::exp(-dry_k * dt);

    if (temperature_c_ > overheat_c) {
      aroma_ -= aroma_damage_k * (temperature_c_ - overheat_c) * dt;
    } else {
      aroma_ += aroma_recover_per_s * dt * (1.0 - aroma_ / 100.0);
    }

    color_ += color_gain_per_s * dt * (1.0 - color_ / 100.0);
  }

  normalize();
  advance_stage_if_needed();

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



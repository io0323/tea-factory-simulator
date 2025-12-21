#include "process/RollingProcess.h"

namespace tea {

/* パラメータを指定して構築します。 */
RollingProcess::RollingProcess(RollingParams params) : params_(params) {
}

/* 既定モデルで構築します。 */
RollingProcess::RollingProcess()
    : params_(make_model(ModelType::DEFAULT).rolling) {
}

/* 揉捻工程の種別を返します。 */
ProcessState RollingProcess::state() const {
  return ProcessState::ROLLING;
}

/* 揉捻工程の 1 ステップ更新です。 */
void RollingProcess::apply_step(TeaLeaf& leaf, int dt_seconds) const {
  /*
    揉捻は「圧力/摩擦で水分が抜ける + 香気が少し増える」挙動を
    シンプルに表します。
    - 温度: 目標温度へ近づく緩和（放熱/冷却の近似）
    - 水分: 現在の水分が多いほど抜けやすい（弱い非線形）
    - 香気/色: 上限 100 へ近づく飽和モデル
  */
  const double dt = static_cast<double>(dt_seconds);
  leaf.temperature_c +=
      (params_.target_temp_c - leaf.temperature_c) * params_.cool_k * dt;
  leaf.moisture -=
      params_.moisture_loss_k * dt * (0.4 + 0.6 * leaf.moisture);
  leaf.aroma += params_.aroma_gain_per_s * dt * (1.0 - leaf.aroma / 100.0);
  leaf.color += params_.color_gain_per_s * dt * (1.0 - leaf.color / 100.0);

  normalize(leaf);
}

} /* namespace tea */



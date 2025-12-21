#include "process/RollingProcess.h"

namespace tea {

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
  const double target_temp_c = 70.0;
  const double cool_k = 0.05;
  const double moisture_loss_k = 0.0015;
  const double aroma_gain_per_s = 0.6;
  const double color_gain_per_s = 0.3;

  const double dt = static_cast<double>(dt_seconds);
  leaf.temperature_c += (target_temp_c - leaf.temperature_c) * cool_k * dt;
  leaf.moisture -= moisture_loss_k * dt * (0.4 + 0.6 * leaf.moisture);
  leaf.aroma += aroma_gain_per_s * dt * (1.0 - leaf.aroma / 100.0);
  leaf.color += color_gain_per_s * dt * (1.0 - leaf.color / 100.0);

  normalize(leaf);
}

} /* namespace tea */



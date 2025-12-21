#include "process/DryingProcess.h"

#include <cmath>

namespace tea {

/* 乾燥工程の種別を返します。 */
ProcessState DryingProcess::state() const {
  return ProcessState::DRYING;
}

/* 乾燥工程の 1 ステップ更新です。 */
void DryingProcess::apply_step(TeaLeaf& leaf, int dt_seconds) const {
  /*
    乾燥は「水分が指数関数的に減る」近似が扱いやすいので、指数減衰で
    モデル化します。
    - 水分: moisture(t+dt) = moisture(t) * exp(-k * dt)
      （含水率が高いほど初期に大きく減り、乾くほど減りにくい）
    - 温度: 目標温度へ近づく緩和（制御された乾燥の近似）
    - 香気: 過熱時（閾値超え）に劣化、通常は僅かに整う
  */
  const double target_temp_c = 60.0;
  const double temp_k = 0.07;
  const double dry_k = 0.05;
  const double aroma_recover_per_s = 0.2;
  const double overheat_c = 70.0;
  const double aroma_damage_k = 0.02;
  const double color_gain_per_s = 0.15;

  const double dt = static_cast<double>(dt_seconds);
  leaf.temperature_c += (target_temp_c - leaf.temperature_c) * temp_k * dt;
  leaf.moisture *= std::exp(-dry_k * dt);

  if (leaf.temperature_c > overheat_c) {
    leaf.aroma -= aroma_damage_k * (leaf.temperature_c - overheat_c) * dt;
  } else {
    leaf.aroma += aroma_recover_per_s * dt * (1.0 - leaf.aroma / 100.0);
  }

  leaf.color += color_gain_per_s * dt * (1.0 - leaf.color / 100.0);

  normalize(leaf);
}

} /* namespace tea */



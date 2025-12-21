#include "process/SteamingProcess.h"

namespace tea {

/* パラメータを指定して構築します。 */
SteamingProcess::SteamingProcess(SteamingParams params) : params_(params) {
}

/* 既定モデルで構築します。 */
SteamingProcess::SteamingProcess()
    : params_(make_model(ModelType::DEFAULT).steaming) {
}

/* 蒸し工程の種別を返します。 */
ProcessState SteamingProcess::state() const {
  return ProcessState::STEAMING;
}

/* 蒸し工程の 1 ステップ更新です。 */
void SteamingProcess::apply_step(TeaLeaf& leaf, int dt_seconds) const {
  /*
    蒸しは「目標温度へ近づく」緩和モデルで表現します。
    - 温度: dT = k * (T_target - T) * dt
      （加熱の立ち上がりが滑らかで、過大なオーバーシュートを避ける）
    - 水分: 蒸気付与による僅かな増加（定率）
    - 香気/色: 上限 100 へ近づく飽和モデル（増分は残り量に比例）
  */
  const double dt = static_cast<double>(dt_seconds);
  leaf.temperature_c +=
      (params_.target_temp_c - leaf.temperature_c) * params_.heat_k * dt;
  leaf.moisture += params_.moisture_gain_per_s * dt;
  leaf.aroma += params_.aroma_gain_per_s * dt * (1.0 - leaf.aroma / 100.0);
  leaf.color += params_.color_gain_per_s * dt * (1.0 - leaf.color / 100.0);

  normalize(leaf);
}

} /* namespace tea */



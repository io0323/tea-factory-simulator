/*
 * @file SteamingProcess.cpp
 * @brief 蒸し工程のシミュレーションロジックを定義
 *
 * このファイルは、お茶の製造プロセスにおける蒸し工程の物理モデルと
 * その状態更新ロジックを実装するSteamingProcessクラスを含んでいます。
 * 温度の緩和、水分量の増加、香気と色の飽和増加などをシミュレートします。
 */

#include "process/SteamingProcess.h"

namespace tea {

/*
 * @brief SteamingProcessのコンストラクタです。
 *
 * 指定されたSteamingParamsパラメータを使用して蒸し工程を構築します。
 *
 * @param params 蒸し工程のパラメータ
 */
SteamingProcess::SteamingProcess(SteamingParams params) : params_(params) {
}

/*
 * @brief SteamingProcessの既定コンストラクタです。
 *
 * デフォルトモデルの蒸し工程パラメータを使用して構築します。
 */
SteamingProcess::SteamingProcess()
    : params_(make_model(ModelType::DEFAULT).steaming) {
}

/*
 * @brief 蒸し工程の種別を返します。
 *
 * @return ProcessState::STEAMING
 */
ProcessState SteamingProcess::state() const {
  return ProcessState::STEAMING;
}

/*
 * @brief 蒸し工程の1ステップ更新を適用します。
 *
 * 指定された時間間隔（dt_seconds）に基づいて、茶葉の状態（温度、水分、香気、色）を
 * 更新します。温度は目標値へ近づくように緩和され、水分は微増し、香気と色は
 * 上限に近づく飽和モデルで増加します。
 *
 * @param leaf 更新するTeaLeafオブジェクトへの参照
 * @param dt_seconds 更新する時間間隔（秒）
 */
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

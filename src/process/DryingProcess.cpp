/*
 * @file DryingProcess.cpp
 * @brief 乾燥工程のシミュレーションロジックを定義
 *
 * このファイルは、お茶の製造プロセスにおける乾燥工程の物理モデルと
 * その状態更新ロジックを実装するDryingProcessクラスを含んでいます。
 * 水分量の指数減衰、温度の緩和、香気への影響などをシミュレートします。
 */

#include "process/DryingProcess.h"

#include <cmath> // For std::exp

namespace tea {

/*
 * @brief DryingProcessのコンストラクタです。
 *
 * 指定されたDryingParamsパラメータを使用して乾燥工程を構築します。
 *
 * @param params 乾燥工程のパラメータ
 */
DryingProcess::DryingProcess(DryingParams params) : params_(params) {
}

/*
 * @brief DryingProcessの既定コンストラクタです。
 *
 * デフォルトモデルの乾燥工程パラメータを使用して構築します。
 */
DryingProcess::DryingProcess()
    : params_(make_model(ModelType::DEFAULT).drying) {
}

/*
 * @brief 乾燥工程の種別を返します。
 *
 * @return ProcessState::DRYING
 */
ProcessState DryingProcess::state() const {
  return ProcessState::DRYING;
}

/*
 * @brief 乾燥工程の1ステップ更新を適用します。
 *
 * 指定された時間間隔（dt_seconds）に基づいて、茶葉の状態（温度、水分、香気、色）を
 * 更新します。水分は指数減衰し、温度は目標値へ緩和、香気は過熱時に劣化し、
 * それ以外の場合は僅かに整います。
 *
 * @param leaf 更新するTeaLeafオブジェクトへの参照
 * @param dt_seconds 更新する時間間隔（秒）
 */
void DryingProcess::apply_step(TeaLeaf& leaf, int dt_seconds) const {
  /*
    乾燥は「水分が指数関数的に減る」近似が扱いやすいので、指数減衰で
    モデル化します。
    - 水分: moisture(t+dt) = moisture(t) * exp(-k * dt)
      （含水率が高いほど初期に大きく減り、乾くほど減りにくい）
    - 温度: 目標温度へ近づく緩和（制御された乾燥の近似）
    - 香気: 過熱時（閾値超え）に劣化、通常は僅かに整う
  */
  const double dt = static_cast<double>(dt_seconds);
  leaf.temperature_c +=
      (params_.target_temp_c - leaf.temperature_c) * params_.temp_k * dt;
  leaf.moisture *= std::exp(-params_.dry_k * dt);

  if (leaf.temperature_c > params_.overheat_c) {
    leaf.aroma -=
        params_.aroma_damage_k * (leaf.temperature_c - params_.overheat_c) * dt;
  } else {
    leaf.aroma +=
        params_.aroma_recover_per_s * dt * (1.0 - leaf.aroma / 100.0);
  }

  leaf.color +=
      params_.color_gain_per_s * dt * (1.0 - leaf.color / 100.0);

  normalize(leaf);
}

} /* namespace tea */

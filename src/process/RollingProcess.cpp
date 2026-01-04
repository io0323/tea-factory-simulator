/*
 * @file RollingProcess.cpp
 * @brief 揉捻工程のシミュレーションロジックを定義
 *
 * このファイルは、お茶の製造プロセスにおける揉捻工程の物理モデルと
 * その状態更新ロジックを実装するRollingProcessクラスを含んでいます。
 * 温度の冷却、水分量の減少、香気と色の増加などをシミュレートします。
 */

#include "process/RollingProcess.h"

namespace tea {

/*
 * @brief RollingProcessのコンストラクタです。
 *
 * 指定されたRollingParamsパラメータを使用して揉捻工程を構築します。
 *
 * @param params 揉捻工程のパラメータ
 */
RollingProcess::RollingProcess(RollingParams params) : params_(params) {
}

/*
 * @brief RollingProcessの既定コンストラクタです。
 *
 * デフォルトモデルの揉捻工程パラメータを使用して構築します。
 */
RollingProcess::RollingProcess()
    : params_(make_model(ModelType::DEFAULT).rolling) {
}

/*
 * @brief 揉捻工程の種別を返します。
 *
 * @return ProcessState::ROLLING
 */
ProcessState RollingProcess::state() const {
  return ProcessState::ROLLING;
}

/*
 * @brief 揉捻工程の1ステップ更新を適用します。
 *
 * 指定された時間間隔（dt_seconds）に基づいて、茶葉の状態（温度、水分、香気、色）を
 * 更新します。温度は目標値へ緩和され、水分は減少し、香気と色は飽和しながら増加します。
 *
 * @param leaf 更新するTeaLeafオブジェクトへの参照
 * @param dt_seconds 更新する時間間隔（秒）
 */
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

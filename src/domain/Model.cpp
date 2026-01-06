/*
 * @file Model.cpp
 * @brief シミュレーションモデルのパラメータとユーティリティ関数を提供
 *
 * このファイルは、異なるモデルタイプ（DEFAULT, GENTLE, AGGRESSIVE）に
 * 対応するシミュレーションパラメータ（蒸し、揉捻、乾燥工程の係数）を
 * 生成する機能と、モデルタイプを表示文字列に変換する機能を提供します。
 */

#include "domain/Model.h"

namespace tea {

/*
 * @brief モデル種別から工程別パラメータを構築します。
 *
 * 各モデルタイプ（DEFAULT, GENTLE, AGGRESSIVE）に応じた
 * シミュレーションパラメータ（熱、水分、香気などの係数）を設定します。
 *
 * @param type 構築するモデルのタイプ
 * @return 構築されたModelParams構造体
 */
ModelParams make_model(ModelType type) {
  /*
    係数は「挙動の違いが分かりやすい」ことを優先し、過度に複雑化しません。
    - GENTLE: 変化を緩やかに（熱/乾燥/香気変化を弱める）
    - AGGRESSIVE: 変化を強めに（熱/乾燥/香気変化を強める）
  */
  ModelParams p;

  if (type == ModelType::GENTLE) {
    p.steaming.heat_k = 0.05;
    p.steaming.moisture_gain_per_s = 0.0006;
    p.steaming.aroma_gain_per_s = 0.7;

    p.rolling.cool_k = 0.04;
    p.rolling.moisture_loss_k = 0.0010;
    p.rolling.aroma_gain_per_s = 0.4;

    p.drying.temp_k = 0.06;
    p.drying.dry_k = 0.035;
    p.drying.aroma_damage_k = 0.015;
    return p;
  }

  if (type == ModelType::AGGRESSIVE) {
    p.steaming.heat_k = 0.11;
    p.steaming.moisture_gain_per_s = 0.0011;
    p.steaming.aroma_gain_per_s = 1.4;

    p.rolling.cool_k = 0.07;
    p.rolling.moisture_loss_k = 0.0022;
    p.rolling.aroma_gain_per_s = 0.9;

    p.drying.temp_k = 0.09;
    p.drying.dry_k = 0.07;
    p.drying.aroma_damage_k = 0.03;
    return p;
  }

  return p;
}

/*
 * @brief モデルタイプを表示用の文字列に変換します。
 *
 * @param type 変換するModelType
 * @return モデル名を表す文字列
 */
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

} /* namespace tea */

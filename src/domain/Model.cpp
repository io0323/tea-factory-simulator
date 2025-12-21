#include "domain/Model.h"

namespace tea {

/* モデル種別から工程別パラメータを構築します。 */
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

} /* namespace tea */



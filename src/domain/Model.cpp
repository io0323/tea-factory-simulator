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

  if (type == ModelType::DEFAULT) {
    return p;
  }

  /*
    DEFAULT を基準に、モデルごとに倍率で挙動を変えます。
    これにより「一部係数だけがDEFAULTのまま」という不整合を避けます。
  */
  const double k =
      (type == ModelType::GENTLE) ? 0.75 :
      (type == ModelType::AGGRESSIVE) ? 1.25 : 1.0;

  p.steaming.heat_k *= k;
  p.steaming.moisture_gain_per_s *= k;
  p.steaming.aroma_gain_per_s *= k;
  p.steaming.color_gain_per_s *= k;

  p.rolling.cool_k *= k;
  p.rolling.moisture_loss_k *= k;
  p.rolling.aroma_gain_per_s *= k;
  p.rolling.color_gain_per_s *= k;

  p.drying.temp_k *= k;
  p.drying.dry_k *= k;
  p.drying.aroma_recover_per_s *= k;
  p.drying.aroma_damage_k *= k;
  p.drying.color_gain_per_s *= k;

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



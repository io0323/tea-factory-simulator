#pragma once

namespace tea {

/* シミュレーションモデル（式の係数セット）を識別します。 */
enum class ModelType {
  DEFAULT,
  GENTLE,
  AGGRESSIVE
};

/* 蒸し工程のパラメータです。 */
struct SteamingParams final {
  double target_temp_c = 95.0;
  double heat_k = 0.08;
  double moisture_gain_per_s = 0.0008;
  double aroma_gain_per_s = 1.0;
  double color_gain_per_s = 0.2;
};

/* 揉捻工程のパラメータです。 */
struct RollingParams final {
  double target_temp_c = 70.0;
  double cool_k = 0.05;
  double moisture_loss_k = 0.0015;
  double aroma_gain_per_s = 0.6;
  double color_gain_per_s = 0.3;
};

/* 乾燥工程のパラメータです。 */
struct DryingParams final {
  double target_temp_c = 60.0;
  double temp_k = 0.07;
  double dry_k = 0.05;
  double aroma_recover_per_s = 0.2;
  double overheat_c = 70.0;
  double aroma_damage_k = 0.02;
  double color_gain_per_s = 0.15;
};

/* 工程別パラメータのセットです。 */
struct ModelParams final {
  SteamingParams steaming;
  RollingParams rolling;
  DryingParams drying;
};

/* モデル種別から工程別パラメータを構築します。 */
ModelParams make_model(ModelType type);

/* 表示用のモデル名を返します。 */
const char* to_string(ModelType type);

} /* namespace tea */




/*
 * @file test_processes.cpp
 * @brief 各工程（蒸し/揉捻/乾燥）の基本挙動の検証
 *
 * 外部テストフレームワークに依存せず、CTest から実行できる最小の検証を行います。
 */

#include "domain/Model.h"
#include "domain/TeaLeaf.h"
#include "process/DryingProcess.h"
#include "process/RollingProcess.h"
#include "process/SteamingProcess.h"

#include "test_utils.h"

namespace {

/*
 * @brief 蒸し工程: 温度が上がり、香気/色が増え、水分が増えることを検証します。
 *
 * @return 成功なら true
 */
bool test_steaming_increases_temp_and_aroma() {
  const tea::ModelParams m = tea::make_model(tea::ModelType::DEFAULT);
  const tea::SteamingProcess proc(m.steaming);
  tea::TeaLeaf leaf;

  const double t0 = leaf.temperature_c;
  const double a0 = leaf.aroma;
  const double c0 = leaf.color;
  const double m0 = leaf.moisture;

  proc.apply_step(leaf, 10);

  bool ok = true;
  ok = tea_test::expect(leaf.temperature_c > t0, "temperature should increase")
       && ok;
  ok = tea_test::expect(leaf.aroma >= a0, "aroma should not decrease") && ok;
  ok = tea_test::expect(leaf.color >= c0, "color should not decrease") && ok;
  ok = tea_test::expect(leaf.moisture >= m0, "moisture should not decrease")
       && ok;
  ok = tea_test::in_bounds(leaf) && ok;
  return ok;
}

/*
 * @brief 揉捻工程: 水分が減り、温度が目標へ向かい、香気/色が増えることを検証します。
 *
 * @return 成功なら true
 */
bool test_rolling_decreases_moisture() {
  const tea::ModelParams m = tea::make_model(tea::ModelType::DEFAULT);
  const tea::RollingProcess proc(m.rolling);
  tea::TeaLeaf leaf;
  leaf.temperature_c = 95.0;

  const double t0 = leaf.temperature_c;
  const double a0 = leaf.aroma;
  const double c0 = leaf.color;
  const double m0 = leaf.moisture;

  proc.apply_step(leaf, 10);

  bool ok = true;
  ok = tea_test::expect(leaf.moisture <= m0, "moisture should not increase")
       && ok;
  ok = tea_test::expect(leaf.temperature_c < t0,
                        "temperature should cool down") && ok;
  ok = tea_test::expect(leaf.aroma >= a0, "aroma should not decrease") && ok;
  ok = tea_test::expect(leaf.color >= c0, "color should not decrease") && ok;
  ok = tea_test::in_bounds(leaf) && ok;
  return ok;
}

/*
 * @brief 乾燥工程: 水分が減り、過熱時は香気が劣化することを検証します。
 *
 * @return 成功なら true
 */
bool test_drying_decreases_moisture_and_can_damage_aroma() {
  const tea::ModelParams m = tea::make_model(tea::ModelType::DEFAULT);
  const tea::DryingProcess proc(m.drying);
  tea::TeaLeaf leaf;
  leaf.temperature_c = m.drying.overheat_c + 20.0;
  leaf.aroma = 80.0;

  const double a0 = leaf.aroma;
  const double m0 = leaf.moisture;

  proc.apply_step(leaf, 5);

  bool ok = true;
  ok = tea_test::expect(leaf.moisture < m0, "moisture should decrease") && ok;
  ok = tea_test::expect(leaf.aroma <= a0,
                        "aroma should not increase when overheated") && ok;
  ok = tea_test::in_bounds(leaf) && ok;
  return ok;
}

} /* namespace */

/*
 * @brief テストのエントリポイントです。
 *
 * @return 0: 成功, 1: 失敗
 */
int main() {
  bool ok = true;
  ok = test_steaming_increases_temp_and_aroma() && ok;
  ok = test_rolling_decreases_moisture() && ok;
  ok = test_drying_decreases_moisture_and_can_damage_aroma() && ok;

  if (!ok) {
    return 1;
  }
  std::cout << "process_tests: OK\n";
  return 0;
}


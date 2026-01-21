/*
 * @file test_model.cpp
 * @brief make_model() の係数セット差分（DEFAULT/GENTLE/AGGRESSIVE）の検証
 *
 * 外部テストフレームワークに依存せず、CTest から実行できる最小の検証を行います。
 */

#include <iostream>

#include "domain/Model.h"
#include "test_utils.h"

namespace {

/*
 * @brief GENTLE/AGGRESSIVE が DEFAULT に対して係数倍率で変化することを検証します。
 *
 * @return 成功なら true
 */
bool test_model_scaling_is_consistent() {
  const tea::ModelParams def = tea::make_model(tea::ModelType::DEFAULT);
  const tea::ModelParams gentle = tea::make_model(tea::ModelType::GENTLE);
  const tea::ModelParams aggr = tea::make_model(tea::ModelType::AGGRESSIVE);

  bool ok = true;

  /* STEAMING */
  ok = tea_test::expect(gentle.steaming.heat_k < def.steaming.heat_k,
                        "gentle steaming.heat_k should be smaller") && ok;
  ok = tea_test::expect(aggr.steaming.heat_k > def.steaming.heat_k,
                        "aggressive steaming.heat_k should be larger") && ok;

  ok = tea_test::expect(gentle.steaming.aroma_gain_per_s <
                            def.steaming.aroma_gain_per_s,
                        "gentle steaming.aroma_gain_per_s should be smaller")
       && ok;
  ok = tea_test::expect(aggr.steaming.aroma_gain_per_s >
                            def.steaming.aroma_gain_per_s,
                        "aggressive steaming.aroma_gain_per_s should be larger")
       && ok;

  /* ROLLING */
  ok = tea_test::expect(gentle.rolling.moisture_loss_k <
                            def.rolling.moisture_loss_k,
                        "gentle rolling.moisture_loss_k should be smaller")
       && ok;
  ok = tea_test::expect(aggr.rolling.moisture_loss_k >
                            def.rolling.moisture_loss_k,
                        "aggressive rolling.moisture_loss_k should be larger")
       && ok;

  /* DRYING */
  ok = tea_test::expect(gentle.drying.dry_k < def.drying.dry_k,
                        "gentle drying.dry_k should be smaller") && ok;
  ok = tea_test::expect(aggr.drying.dry_k > def.drying.dry_k,
                        "aggressive drying.dry_k should be larger") && ok;

  ok = tea_test::expect(gentle.drying.aroma_damage_k <
                            def.drying.aroma_damage_k,
                        "gentle drying.aroma_damage_k should be smaller")
       && ok;
  ok = tea_test::expect(aggr.drying.aroma_damage_k >
                            def.drying.aroma_damage_k,
                        "aggressive drying.aroma_damage_k should be larger")
       && ok;

  return ok;
}

/*
 * @brief to_string(ModelType) が安定した文字列を返すことを検証します。
 *
 * @return 成功なら true
 */
bool test_model_to_string() {
  bool ok = true;
  ok = tea_test::expect(std::string(tea::to_string(tea::ModelType::DEFAULT)) ==
                            "default",
                        "default to_string should match") && ok;
  ok = tea_test::expect(std::string(tea::to_string(tea::ModelType::GENTLE)) ==
                            "gentle",
                        "gentle to_string should match") && ok;
  ok = tea_test::expect(
      std::string(tea::to_string(tea::ModelType::AGGRESSIVE)) == "aggressive",
      "aggressive to_string should match") && ok;
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
  ok = test_model_scaling_is_consistent() && ok;
  ok = test_model_to_string() && ok;

  if (!ok) {
    return 1;
  }
  std::cout << "model_tests: OK\n";
  return 0;
}


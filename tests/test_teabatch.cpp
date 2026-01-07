/*
 * @file test_teabatch.cpp
 * @brief GUI版 TeaBatch の工程遷移とモデル差分の検証
 *
 * 外部テストフレームワークに依存せず、CTest から実行できる最小の検証を行います。
 */

#include <cmath>
#include <iostream>

#include "TeaBatch.h"

namespace {

/*
 * @brief 条件が偽ならエラー表示し、失敗扱いにします。
 *
 * @param ok 検証結果
 * @param msg 失敗時のメッセージ
 * @return ok が真なら true
 */
bool expect(bool ok, const char* msg) {
  if (ok) {
    return true;
  }
  std::cerr << "EXPECT FAILED: " << (msg ? msg : "(null)") << '\n';
  return false;
}

/*
 * @brief 2つの実数が近いことを検証します。
 *
 * @param a 値1
 * @param b 値2
 * @param eps 許容誤差
 * @return 近いなら true
 */
bool nearly(double a, double b, double eps) {
  return std::fabs(a - b) <= eps;
}

/*
 * @brief 工程境界を跨ぐ dt を与えても、分割適用と一致することを検証します。
 *
 * 例: 31秒を1回で与えるのと、30秒+1秒で与えるのが同じ結果になるべきです。
 *
 * @return 成功なら true
 */
bool test_stage_boundary_carryover() {
  tea_gui::TeaBatch a;
  tea_gui::TeaBatch b;
  a.reset();
  b.reset();

  a.update(31.0);
  b.update(30.0);
  b.update(1.0);

  const double eps = 1e-9;
  bool ok = true;
  ok = expect(a.process() == b.process(), "process should match") && ok;
  ok = expect(a.elapsed_seconds() == b.elapsed_seconds(),
              "elapsed_seconds should match") && ok;
  ok = expect(nearly(a.moisture(), b.moisture(), eps), "moisture should match")
       && ok;
  ok = expect(nearly(a.temperature_c(), b.temperature_c(), eps),
              "temperature should match") && ok;
  ok = expect(nearly(a.aroma(), b.aroma(), eps), "aroma should match") && ok;
  ok = expect(nearly(a.color(), b.color(), eps), "color should match") && ok;
  return ok;
}

/*
 * @brief 十分な時間を進めると FINISHED に到達することを検証します。
 *
 * @return 成功なら true
 */
bool test_reaches_finished() {
  tea_gui::TeaBatch b;
  b.reset();
  b.update(30.0);
  b.update(30.0);
  b.update(60.0);
  return expect(b.process() == tea_gui::ProcessState::FINISHED,
                "TeaBatch should reach FINISHED after 120s");
}

/*
 * @brief モデル差分（倍率）が挙動に反映されることを検証します。
 *
 * @return 成功なら true
 */
bool test_model_scaling_effect() {
  tea_gui::TeaBatch gentle;
  tea_gui::TeaBatch aggr;
  gentle.set_model(tea_gui::ModelType::GENTLE);
  aggr.set_model(tea_gui::ModelType::AGGRESSIVE);
  gentle.reset();
  aggr.reset();

  gentle.update(10.0);
  aggr.update(10.0);

  return expect(aggr.aroma() > gentle.aroma(),
                "AGGRESSIVE should increase aroma faster than GENTLE");
}

} /* namespace */

/*
 * @brief テストのエントリポイントです。
 *
 * @return 0: 成功, 1: 失敗
 */
int main() {
  bool ok = true;
  ok = test_stage_boundary_carryover() && ok;
  ok = test_reaches_finished() && ok;
  ok = test_model_scaling_effect() && ok;

  if (!ok) {
    return 1;
  }
  std::cout << "unit_tests: OK\n";
  return 0;
}



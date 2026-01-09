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
  return expect(b.process() == tea::ProcessState::FINISHED,
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
  gentle.set_model(tea::ModelType::GENTLE);
  aggr.set_model(tea::ModelType::AGGRESSIVE);
  gentle.reset();
  aggr.reset();

  gentle.update(10.0);
  aggr.update(10.0);

  return expect(aggr.aroma() > gentle.aroma(),
                "AGGRESSIVE should increase aroma faster than GENTLE");
}

/*
 * @brief dt=0.1 を10回積み上げるのが 1.0 を1回と一致することを検証します。
 *
 * @return 成功なら true
 */
bool test_fractional_dt_accumulation() {
  tea_gui::TeaBatch a;
  tea_gui::TeaBatch b;
  a.reset();
  b.reset();

  b.update(1.0);
  for (int i = 0; i < 10; ++i) {
    a.update(0.1);
  }

  const double eps = 1e-6;
  bool ok = true;
  ok = expect(a.elapsed_seconds() == b.elapsed_seconds(),
              "elapsed_seconds should match (0.1s x 10 ~= 1.0s)") && ok;
  ok = expect(a.process() == b.process(),
              "process should match (0.1s x 10 ~= 1.0s)") && ok;
  ok = expect(nearly(a.moisture(), b.moisture(), eps),
              "moisture should match (0.1s x 10 ~= 1.0s)") && ok;
  ok = expect(nearly(a.temperature_c(), b.temperature_c(), eps),
              "temperature should match (0.1s x 10 ~= 1.0s)") && ok;
  ok = expect(nearly(a.aroma(), b.aroma(), eps),
              "aroma should match (0.1s x 10 ~= 1.0s)") && ok;
  ok = expect(nearly(a.color(), b.color(), eps),
              "color should match (0.1s x 10 ~= 1.0s)") && ok;
  return ok;
}

/*
 * @brief 工程境界を複数回跨ぐ dt でも分割適用と一致することを検証します。
 *
 * 例: 61秒を1回で与えるのと、30+30+1で与えるのが一致するべきです。
 *
 * @return 成功なら true
 */
bool test_multiple_stage_boundaries() {
  tea_gui::TeaBatch a;
  tea_gui::TeaBatch b;
  a.reset();
  b.reset();

  a.update(61.0);
  b.update(30.0);
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
 * @brief 完了後に余分な dt を与えても状態が進まないことを検証します。
 *
 * @return 成功なら true
 */
bool test_overrun_after_finished() {
  tea_gui::TeaBatch a;
  tea_gui::TeaBatch b;
  a.reset();
  b.reset();

  a.update(125.0);
  b.update(120.0);
  b.update(5.0);

  const double eps = 1e-9;
  bool ok = true;
  ok = expect(a.process() == tea::ProcessState::FINISHED,
              "a should be FINISHED") && ok;
  ok = expect(b.process() == tea::ProcessState::FINISHED,
              "b should be FINISHED") && ok;
  ok = expect(a.elapsed_seconds() == 120, "a elapsed should be 120") && ok;
  ok = expect(b.elapsed_seconds() == 120, "b elapsed should be 120") && ok;
  ok = expect(nearly(a.moisture(), b.moisture(), eps), "moisture should match")
       && ok;
  ok = expect(nearly(a.temperature_c(), b.temperature_c(), eps),
              "temperature should match") && ok;
  ok = expect(nearly(a.aroma(), b.aroma(), eps), "aroma should match") && ok;
  ok = expect(nearly(a.color(), b.color(), eps), "color should match") && ok;
  return ok;
}

/*
 * @brief 途中でモデルを切り替えても工程/経過が壊れないことを検証します。
 *
 * @return 成功なら true
 */
bool test_model_switch_regression() {
  tea_gui::TeaBatch switched;
  tea_gui::TeaBatch baseline;

  switched.set_model(tea::ModelType::GENTLE);
  baseline.set_model(tea::ModelType::GENTLE);
  switched.reset();
  baseline.reset();

  switched.update(31.0);
  baseline.update(31.0);

  const tea::ProcessState p0 = baseline.process();
  const int e0 = baseline.elapsed_seconds();

  switched.set_model(tea::ModelType::AGGRESSIVE);

  switched.update(10.0);
  baseline.update(10.0);

  bool ok = true;
  ok = expect(switched.process() == p0, "process should not reset") && ok;
  ok = expect(switched.elapsed_seconds() == e0 + 10,
              "elapsed_seconds should continue") && ok;
  ok = expect(switched.aroma() > baseline.aroma(),
              "AGGRESSIVE after switch should increase aroma faster") && ok;
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
  ok = test_stage_boundary_carryover() && ok;
  ok = test_reaches_finished() && ok;
  ok = test_model_scaling_effect() && ok;
  ok = test_fractional_dt_accumulation() && ok;
  ok = test_multiple_stage_boundaries() && ok;
  ok = test_overrun_after_finished() && ok;
  ok = test_model_switch_regression() && ok;

  if (!ok) {
    return 1;
  }
  std::cout << "unit_tests: OK\n";
  return 0;
}




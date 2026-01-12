/*
 * @file test_simulator.cpp
 * @brief tea::Simulator の工程遷移と dt 分割の検証
 *
 * 外部テストフレームワークに依存せず、CTest から実行できる最小の検証を行います。
 */

#include <iostream>

#include "simulation/Simulator.h"

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
 * @brief 茶葉状態が定義域に収まっていることを検証します。
 *
 * @param leaf 対象の茶葉
 * @return 定義域に収まるなら true
 */
bool in_bounds(const tea::TeaLeaf& leaf) {
  bool ok = true;
  ok = expect(leaf.moisture >= 0.0 && leaf.moisture <= 1.0,
              "moisture should be within [0,1]") && ok;
  ok = expect(leaf.aroma >= 0.0 && leaf.aroma <= 100.0,
              "aroma should be within [0,100]") && ok;
  ok = expect(leaf.color >= 0.0 && leaf.color <= 100.0,
              "color should be within [0,100]") && ok;
  return ok;
}

/*
 * @brief dt が工程時間で割り切れなくても、合計時間ぴったりで完了することを検証します。
 *
 * @return 成功なら true
 */
bool test_dt_is_split_to_fit_stage_duration() {
  tea::SimulationConfig config;
  config.dt_seconds = 7;
  config.steaming_seconds = 10;
  config.rolling_seconds = 6;
  config.drying_seconds = 3;

  tea::Simulator sim(config);
  const int total = config.steaming_seconds + config.rolling_seconds +
                    config.drying_seconds;

  int last_elapsed = 0;
  bool ok = true;
  while (sim.step(config.dt_seconds, nullptr)) {
    const int elapsed = sim.elapsed_seconds();
    ok = expect(elapsed > last_elapsed, "elapsed should increase") && ok;
    ok = expect(elapsed - last_elapsed <= config.dt_seconds,
                "step should not exceed dt_seconds") && ok;
    ok = expect(elapsed <= total, "elapsed should not exceed total") && ok;
    ok = in_bounds(sim.leaf()) && ok;
    last_elapsed = elapsed;
  }

  ok = expect(sim.elapsed_seconds() == total,
              "elapsed_seconds should reach total duration") && ok;
  ok = expect(sim.current_process() == tea::ProcessState::FINISHED,
              "current_process should be FINISHED after completion") && ok;
  return ok;
}

/*
 * @brief 工程が STEAMING→ROLLING→DRYING の順で進行することを検証します。
 *
 * @return 成功なら true
 */
bool test_process_order_progresses() {
  tea::SimulationConfig config;
  config.dt_seconds = 4;
  config.steaming_seconds = 5;
  config.rolling_seconds = 5;
  config.drying_seconds = 5;

  tea::Simulator sim(config);
  bool seen_steaming = false;
  bool seen_rolling = false;
  bool seen_drying = false;

  bool ok = true;
  while (sim.step(config.dt_seconds, nullptr)) {
    const tea::ProcessState state = sim.current_process();
    if (state == tea::ProcessState::STEAMING) {
      seen_steaming = true;
    } else if (state == tea::ProcessState::ROLLING) {
      seen_rolling = true;
    } else if (state == tea::ProcessState::DRYING) {
      seen_drying = true;
    }
    ok = in_bounds(sim.leaf()) && ok;
  }

  ok = expect(seen_steaming, "should observe STEAMING") && ok;
  ok = expect(seen_rolling, "should observe ROLLING") && ok;
  ok = expect(seen_drying, "should observe DRYING") && ok;
  ok = expect(sim.current_process() == tea::ProcessState::FINISHED,
              "should be FINISHED at end") && ok;
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
  ok = test_dt_is_split_to_fit_stage_duration() && ok;
  ok = test_process_order_progresses() && ok;

  if (!ok) {
    return 1;
  }
  std::cout << "simulator_tests: OK\n";
  return 0;
}


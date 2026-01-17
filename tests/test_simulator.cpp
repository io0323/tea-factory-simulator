/*
 * @file test_simulator.cpp
 * @brief tea::Simulator の工程遷移と dt 分割の検証
 *
 * 外部テストフレームワークに依存せず、CTest から実行できる最小の検証を行います。
 */

#include "simulation/Simulator.h"

#include "test_utils.h"

namespace {

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
    ok = tea_test::expect(elapsed > last_elapsed, "elapsed should increase")
         && ok;
    ok = tea_test::expect(elapsed - last_elapsed <= config.dt_seconds,
                          "step should not exceed dt_seconds") && ok;
    ok = tea_test::expect(elapsed <= total, "elapsed should not exceed total")
         && ok;
    ok = tea_test::in_bounds(sim.leaf()) && ok;
    last_elapsed = elapsed;
  }

  ok = tea_test::expect(sim.elapsed_seconds() == total,
                        "elapsed_seconds should reach total duration") && ok;
  ok = tea_test::expect(sim.current_process() == tea::ProcessState::FINISHED,
                        "current_process should be FINISHED after completion")
       && ok;
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
    ok = tea_test::in_bounds(sim.leaf()) && ok;
  }

  ok = tea_test::expect(seen_steaming, "should observe STEAMING") && ok;
  ok = tea_test::expect(seen_rolling, "should observe ROLLING") && ok;
  ok = tea_test::expect(seen_drying, "should observe DRYING") && ok;
  ok = tea_test::expect(sim.current_process() == tea::ProcessState::FINISHED,
                        "should be FINISHED at end") && ok;
  return ok;
}

/*
 * @brief dt が 0 以下の場合、進めず false を返すことを検証します。
 *
 * @return 成功なら true
 */
bool test_dt_non_positive_is_rejected() {
  tea::SimulationConfig config;
  config.dt_seconds = 1;
  config.steaming_seconds = 5;
  config.rolling_seconds = 5;
  config.drying_seconds = 5;
  tea::Simulator sim(config);

  const bool ok0 = tea_test::expect(!sim.step(0, nullptr),
                                    "dt=0 should be rejected");
  const bool ok1 = tea_test::expect(!sim.step(-1, nullptr),
                                    "dt=-1 should be rejected");
  const bool ok2 = tea_test::expect(sim.elapsed_seconds() == 0,
                                    "elapsed_seconds should remain 0");
  return ok0 && ok1 && ok2;
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
  ok = test_dt_non_positive_is_rejected() && ok;

  if (!ok) {
    return 1;
  }
  std::cout << "simulator_tests: OK\n";
  return 0;
}


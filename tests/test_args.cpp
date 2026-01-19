/*
 * @file test_args.cpp
 * @brief CLI引数パーサの回帰テスト
 *
 * 外部テストフレームワークに依存せず、CTest から実行できる最小の検証を行います。
 */

#include <string>
#include <vector>

#include "cli/Args.h"

#include "test_utils.h"

namespace {

/*
 * @brief argv を組み立てて parse_args を呼びます。
 *
 * @param args argv の要素（argv[0] 含む）
 * @return 解析結果
 */
tea_cli::Args parse_from(const std::vector<std::string>& args) {
  std::vector<char*> argv;
  argv.reserve(args.size());
  for (const std::string& s : args) {
    argv.push_back(const_cast<char*>(s.c_str()));
  }
  return tea_cli::parse_args(static_cast<int>(argv.size()), argv.data());
}

/*
 * @brief dt が工程時間より大きくても弾かれないことを検証します。
 *
 * @return 成功なら true
 */
bool test_dt_can_exceed_stage_seconds() {
  const tea_cli::Args args = parse_from(
      {"tea_factory_simulator_cli", "--dt", "120"});
  bool ok = true;
  ok = tea_test::expect(!args.error.has_value(),
                        "dt=120 should be accepted") && ok;
  ok = tea_test::expect(args.dt_seconds == 120, "dt_seconds should be 120")
       && ok;
  return ok;
}

/*
 * @brief 不正値はエラーになることを検証します。
 *
 * @return 成功なら true
 */
bool test_invalid_dt_is_rejected() {
  const tea_cli::Args args = parse_from(
      {"tea_factory_simulator_cli", "--dt", "0"});
  return tea_test::expect(args.error.has_value(), "dt=0 should be rejected");
}

/*
 * @brief 値が不足している場合にエラーになることを検証します。
 *
 * @return 成功なら true
 */
bool test_missing_value_is_rejected() {
  const tea_cli::Args args = parse_from({"tea_factory_simulator_cli", "--dt"});
  return tea_test::expect(args.error.has_value(),
                          "missing value for --dt should be rejected");
}

/*
 * @brief 未知の引数がエラーになることを検証します。
 *
 * @return 成功なら true
 */
bool test_unknown_argument_is_rejected() {
  const tea_cli::Args args = parse_from(
      {"tea_factory_simulator_cli", "--unknown"});
  return tea_test::expect(args.error.has_value(),
                          "unknown argument should be rejected");
}

/*
 * @brief model の妥当/不正値を検証します。
 *
 * @return 成功なら true
 */
bool test_model_validation() {
  bool ok = true;
  {
    const tea_cli::Args args = parse_from(
        {"tea_factory_simulator_cli", "--model", "gentle"});
    ok = tea_test::expect(!args.error.has_value(),
                          "model=gentle should be accepted") && ok;
    ok = tea_test::expect(args.model == "gentle", "model should be gentle")
         && ok;
  }
  {
    const tea_cli::Args args = parse_from(
        {"tea_factory_simulator_cli", "--model", "invalid"});
    ok = tea_test::expect(args.error.has_value(),
                          "invalid model should be rejected") && ok;
  }
  return ok;
}

/*
 * @brief batches の境界（最大128）を検証します。
 *
 * @return 成功なら true
 */
bool test_batches_bounds() {
  bool ok = true;
  {
    const tea_cli::Args args = parse_from(
        {"tea_factory_simulator_cli", "--batches", "128"});
    ok = tea_test::expect(!args.error.has_value(),
                          "batches=128 should be accepted") && ok;
    ok = tea_test::expect(args.batches == 128, "batches should be 128") && ok;
  }
  {
    const tea_cli::Args args = parse_from(
        {"tea_factory_simulator_cli", "--batches", "129"});
    ok = tea_test::expect(args.error.has_value(),
                          "batches=129 should be rejected") && ok;
  }
  return ok;
}

/*
 * @brief csv パスの空文字が拒否されることを検証します。
 *
 * @return 成功なら true
 */
bool test_csv_path_must_not_be_empty() {
  const tea_cli::Args args = parse_from(
      {"tea_factory_simulator_cli", "--csv", ""});
  return tea_test::expect(args.error.has_value(),
                          "empty csv path should be rejected");
}

} /* namespace */

/*
 * @brief テストのエントリポイントです。
 *
 * @return 0: 成功, 1: 失敗
 */
int main() {
  bool ok = true;
  ok = test_dt_can_exceed_stage_seconds() && ok;
  ok = test_invalid_dt_is_rejected() && ok;
  ok = test_missing_value_is_rejected() && ok;
  ok = test_unknown_argument_is_rejected() && ok;
  ok = test_model_validation() && ok;
  ok = test_batches_bounds() && ok;
  ok = test_csv_path_must_not_be_empty() && ok;

  if (!ok) {
    return 1;
  }
  std::cout << "cli_args_tests: OK\n";
  return 0;
}


/*
 * @file test_args.cpp
 * @brief CLI引数パーサの回帰テスト
 *
 * 外部テストフレームワークに依存せず、CTest から実行できる最小の検証を行います。
 */

#include <iostream>
#include <string>
#include <vector>

#include "cli/Args.h"

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
  ok = expect(!args.error.has_value(), "dt=120 should be accepted") && ok;
  ok = expect(args.dt_seconds == 120, "dt_seconds should be 120") && ok;
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
  return expect(args.error.has_value(), "dt=0 should be rejected");
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

  if (!ok) {
    return 1;
  }
  std::cout << "cli_args_tests: OK\n";
  return 0;
}


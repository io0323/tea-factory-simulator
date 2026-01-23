/*
 * @file test_process_state.cpp
 * @brief ProcessState::to_string の回帰テスト
 *
 * 外部テストフレームワークに依存せず、CTest から実行できる最小の検証を行います。
 */

#include <iostream>
#include <string>

#include "domain/ProcessState.h"
#include "test_utils.h"

namespace {

/*
 * @brief 全列挙値が安定した文字列へ変換されることを検証します。
 *
 * @return 成功なら true
 */
bool test_to_string_matches_expected() {
  bool ok = true;
  ok = tea_test::expect(std::string(tea::to_string(tea::ProcessState::STEAMING))
                            == "STEAMING",
                        "STEAMING to_string should match") && ok;
  ok = tea_test::expect(std::string(tea::to_string(tea::ProcessState::ROLLING))
                            == "ROLLING",
                        "ROLLING to_string should match") && ok;
  ok = tea_test::expect(std::string(tea::to_string(tea::ProcessState::DRYING))
                            == "DRYING",
                        "DRYING to_string should match") && ok;
  ok = tea_test::expect(std::string(tea::to_string(tea::ProcessState::FINISHED))
                            == "FINISHED",
                        "FINISHED to_string should match") && ok;
  return ok;
}

} /* namespace */

/*
 * @brief テストのエントリポイントです。
 *
 * @return 0: 成功, 1: 失敗
 */
int main() {
  if (!test_to_string_matches_expected()) {
    return 1;
  }
  std::cout << "process_state_tests: OK\n";
  return 0;
}


/*
 * @file test_csvwriter.cpp
 * @brief CsvWriter の品質スコア/ステータス算出の検証
 *
 * 外部テストフレームワークに依存せず、CTest から実行できる最小の検証を行います。
 */

#include <string>

#include "io/CsvWriter.h"

#include "test_utils.h"

namespace {

/*
 * @brief quality_score が要件式どおりで、[0,100] にクランプされることを検証します。
 *
 * @return 成功なら true
 */
bool test_quality_score_formula_and_clamp() {
  bool ok = true;

  {
    const double score = tea_io::CsvWriter::quality_score(0.75, 10.0, 10.0);
    const double expected =
        10.0 * 0.4 + 10.0 * 0.4 + (1.0 - 0.75) * 100.0 * 0.2;
    ok = tea_test::expect(tea_test::nearly(score, expected, 1e-12),
                          "score formula should match") && ok;
  }

  {
    const double score = tea_io::CsvWriter::quality_score(1.0, -1000.0, -1000.0);
    ok = tea_test::expect(score == 0.0, "score should clamp to 0") && ok;
  }

  {
    const double score = tea_io::CsvWriter::quality_score(0.0, 1000.0, 1000.0);
    ok = tea_test::expect(score == 100.0, "score should clamp to 100") && ok;
  }

  return ok;
}

/*
 * @brief quality_status の境界値（80/60）を検証します。
 *
 * @return 成功なら true
 */
bool test_quality_status_thresholds() {
  bool ok = true;
  ok = tea_test::expect(
      std::string(tea_io::CsvWriter::quality_status(80.0)) == "GOOD",
      "80 should be GOOD") && ok;
  ok = tea_test::expect(
      std::string(tea_io::CsvWriter::quality_status(79.999)) == "OK",
      "just under 80 should be OK") && ok;
  ok = tea_test::expect(
      std::string(tea_io::CsvWriter::quality_status(60.0)) == "OK",
      "60 should be OK") && ok;
  ok = tea_test::expect(
      std::string(tea_io::CsvWriter::quality_status(59.999)) == "BAD",
      "just under 60 should be BAD") && ok;
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
  ok = test_quality_score_formula_and_clamp() && ok;
  ok = test_quality_status_thresholds() && ok;

  if (!ok) {
    return 1;
  }
  std::cout << "csvwriter_tests: OK\n";
  return 0;
}


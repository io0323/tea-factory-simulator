/*
 * @file test_csvwriter.cpp
 * @brief CsvWriter の品質スコア/ステータス算出の検証
 *
 * 外部テストフレームワークに依存せず、CTest から実行できる最小の検証を行います。
 */

#include <cmath>
#include <iostream>

#include "io/CsvWriter.h"

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
    ok = expect(nearly(score, expected, 1e-12), "score formula should match")
         && ok;
  }

  {
    const double score = tea_io::CsvWriter::quality_score(1.0, -1000.0, -1000.0);
    ok = expect(score == 0.0, "score should clamp to 0") && ok;
  }

  {
    const double score = tea_io::CsvWriter::quality_score(0.0, 1000.0, 1000.0);
    ok = expect(score == 100.0, "score should clamp to 100") && ok;
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
  ok = expect(std::string(tea_io::CsvWriter::quality_status(80.0)) == "GOOD",
              "80 should be GOOD") && ok;
  ok = expect(std::string(tea_io::CsvWriter::quality_status(79.999)) == "OK",
              "just under 80 should be OK") && ok;
  ok = expect(std::string(tea_io::CsvWriter::quality_status(60.0)) == "OK",
              "60 should be OK") && ok;
  ok = expect(std::string(tea_io::CsvWriter::quality_status(59.999)) == "BAD",
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


/*
 * @file test_tealeaf.cpp
 * @brief TeaLeaf の clamp/normalize の境界値検証
 *
 * 外部テストフレームワークに依存せず、CTest から実行できる最小の検証を行います。
 */

#include <iostream>

#include "domain/TeaLeaf.h"
#include "test_utils.h"

namespace {

/*
 * @brief clamp が境界値に収まることを検証します。
 *
 * @return 成功なら true
 */
bool test_clamp_bounds() {
  bool ok = true;
  ok = tea_test::expect(tea::clamp(-1.0, 0.0, 1.0) == 0.0,
                        "clamp below min should return min") && ok;
  ok = tea_test::expect(tea::clamp(2.0, 0.0, 1.0) == 1.0,
                        "clamp above max should return max") && ok;
  ok = tea_test::expect(tea::clamp(0.25, 0.0, 1.0) == 0.25,
                        "clamp within range should return value") && ok;
  return ok;
}

/*
 * @brief normalize が moisture/aroma/color を定義域へクランプすることを検証します。
 *
 * @return 成功なら true
 */
bool test_normalize_clamps_fields() {
  tea::TeaLeaf leaf;
  leaf.moisture = -0.5;
  leaf.aroma = 200.0;
  leaf.color = -10.0;
  leaf.temperature_c = -999.0; /* 温度は正規化対象外 */

  tea::normalize(leaf);

  bool ok = true;
  ok = tea_test::expect(leaf.moisture == 0.0, "moisture should clamp to 0")
       && ok;
  ok = tea_test::expect(leaf.aroma == 100.0, "aroma should clamp to 100")
       && ok;
  ok = tea_test::expect(leaf.color == 0.0, "color should clamp to 0") && ok;
  ok = tea_test::expect(leaf.temperature_c == -999.0,
                        "temperature_c should not be changed") && ok;
  ok = tea_test::in_bounds(leaf) && ok;
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
  ok = test_clamp_bounds() && ok;
  ok = test_normalize_clamps_fields() && ok;

  if (!ok) {
    return 1;
  }
  std::cout << "tealeaf_tests: OK\n";
  return 0;
}


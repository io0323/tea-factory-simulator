/*
 * @file test_utils.h
 * @brief テストで共通利用する最小ユーティリティ
 *
 * 外部テストフレームワークに依存せず、簡易な expect/nearly などを提供します。
 */

#pragma once

#include <cmath>
#include <iostream>

#include "domain/TeaLeaf.h"

namespace tea_test {

/*
 * @brief 条件が偽ならエラー表示し、失敗扱いにします。
 *
 * @param ok 検証結果
 * @param msg 失敗時のメッセージ
 * @return ok が真なら true
 */
inline bool expect(bool ok, const char* msg) {
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
inline bool nearly(double a, double b, double eps) {
  return std::fabs(a - b) <= eps;
}

/*
 * @brief 値が [min_v, max_v] に収まることを検証します。
 *
 * @param v 値
 * @param min_v 最小値
 * @param max_v 最大値
 * @param msg 失敗時メッセージ
 * @return 範囲内なら true
 */
inline bool expect_in_range(double v,
                            double min_v,
                            double max_v,
                            const char* msg) {
  return expect(v >= min_v && v <= max_v, msg);
}

/*
 * @brief 茶葉状態が定義域に収まっていることを検証します。
 *
 * @param leaf 対象の茶葉
 * @return 定義域に収まるなら true
 */
inline bool in_bounds(const tea::TeaLeaf& leaf) {
  bool ok = true;
  ok = expect_in_range(leaf.moisture, 0.0, 1.0,
                       "moisture should be within [0,1]") && ok;
  ok = expect_in_range(leaf.aroma, 0.0, 100.0,
                       "aroma should be within [0,100]") && ok;
  ok = expect_in_range(leaf.color, 0.0, 100.0,
                       "color should be within [0,100]") && ok;
  return ok;
}

} /* namespace tea_test */


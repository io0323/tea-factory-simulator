#pragma once

#include <algorithm>

namespace tea {

/* 茶葉の物理状態を表すドメインモデルです。 */
struct TeaLeaf final {
  double moisture = 0.75;      /* 水分率 [0.0, 1.0] */
  double temperature_c = 25.0; /* 温度 [°C] */
  double aroma = 10.0;         /* 香気 [0, 100] */
  double color = 10.0;         /* 色指標 [0, 100] */
};

/* 値を [min_v, max_v] に収めます。 */
inline double clamp(double v, double min_v, double max_v) {
  return std::max(min_v, std::min(v, max_v));
}

/* 茶葉の状態を定義域へ正規化します。 */
inline void normalize(TeaLeaf& leaf) {
  leaf.moisture = clamp(leaf.moisture, 0.0, 1.0);
  leaf.aroma = clamp(leaf.aroma, 0.0, 100.0);
  leaf.color = clamp(leaf.color, 0.0, 100.0);
}

} /* namespace tea */




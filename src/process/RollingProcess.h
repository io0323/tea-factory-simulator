#pragma once

#include "process/IProcess.h"

namespace tea {

/* 揉捻工程: 水分を減らしつつ、香気と色を緩やかに変化させる工程です。 */
class RollingProcess final : public IProcess {
 public:
  /* 工程の種別を返します。 */
  ProcessState state() const override;

  /* 1 ステップ分の更新を行います。 */
  void apply_step(TeaLeaf& leaf, int dt_seconds) const override;
};

} /* namespace tea */



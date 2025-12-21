#pragma once

#include "domain/ProcessState.h"
#include "domain/TeaLeaf.h"

namespace tea {

/* 各製造工程が 1 ステップで茶葉へ与える影響を表すインターフェースです。 */
class IProcess {
 public:
  /* 仮想デストラクタです。 */
  virtual ~IProcess() = default;

  /* 工程の状態（種別）を返します。 */
  virtual ProcessState state() const = 0;

  /* 1 ステップ分（dt 秒）だけ茶葉の状態を更新します。 */
  virtual void apply_step(TeaLeaf& leaf, int dt_seconds) const = 0;
};

} /* namespace tea */



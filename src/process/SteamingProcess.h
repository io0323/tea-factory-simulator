#pragma once

#include "process/IProcess.h"

namespace tea {

/* 蒸し工程: 温度上昇 + しっとり化 + 香気の立ち上がりを表す工程です。 */
class SteamingProcess final : public IProcess {
 public:
  /* 工程の種別を返します。 */
  ProcessState state() const override;

  /* 1 ステップ分の更新を行います。 */
  void apply_step(TeaLeaf& leaf, int dt_seconds) const override;
};

} /* namespace tea */



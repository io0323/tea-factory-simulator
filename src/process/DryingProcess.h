#pragma once

#include "domain/Model.h"
#include "process/IProcess.h"

namespace tea {

/* 乾燥工程: 水分を強く減らし、過熱時は香気が劣化する工程です。 */
class DryingProcess final : public IProcess {
 public:
  /* パラメータを指定して構築します。 */
  explicit DryingProcess(DryingParams params);

  /* 既定モデルで構築します。 */
  DryingProcess();

  /* 工程の種別を返します。 */
  ProcessState state() const override;

  /* 1 ステップ分の更新を行います。 */
  void apply_step(TeaLeaf& leaf, int dt_seconds) const override;

 private:
  DryingParams params_;
};

} /* namespace tea */



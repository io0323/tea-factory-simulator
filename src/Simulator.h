#pragma once

#include "TeaBatch.h"

namespace tea_gui {

/*
  シミュレーションの実行制御（start/pause/reset）を担当します。
  TeaBatch を保持し、UI からは操作と参照のみを行えるようにします。
*/
class Simulator final {
 public:
  /* 既定状態で構築します。 */
  Simulator();

  /* 実行を開始します。 */
  void start();

  /* 実行を一時停止します。 */
  void pause();

  /* 初期状態へ戻します（停止状態になります）。 */
  void reset();

  /* 実行中なら deltaTime（秒）だけ更新します。 */
  void update(double delta_seconds);

  /* 実行中かどうかを返します。 */
  bool is_running() const;

  /* TeaBatch を参照します（UI 用）。 */
  const TeaBatch& batch() const;

 private:
  bool running_ = false;
  TeaBatch batch_;
};

} /* namespace tea_gui */



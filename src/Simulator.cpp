#include "Simulator.h"

namespace tea_gui {

/* 既定状態で構築します。 */
Simulator::Simulator() = default;

/* 実行を開始します。 */
void Simulator::start() {
  if (batch_.process() == ProcessState::FINISHED) {
    return;
  }
  running_ = true;
}

/* 実行を一時停止します。 */
void Simulator::pause() {
  running_ = false;
}

/* 初期状態へ戻します（停止状態になります）。 */
void Simulator::reset() {
  running_ = false;
  batch_.reset();
}

/* 実行中なら deltaTime（秒）だけ更新します。 */
void Simulator::update(double delta_seconds) {
  if (!running_) {
    return;
  }
  batch_.update(delta_seconds);
  if (batch_.process() == ProcessState::FINISHED) {
    running_ = false;
  }
}

/* 実行中かどうかを返します。 */
bool Simulator::is_running() const {
  return running_;
}

/* TeaBatch を参照します（UI 用）。 */
const TeaBatch& Simulator::batch() const {
  return batch_;
}

} /* namespace tea_gui */



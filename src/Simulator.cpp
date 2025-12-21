#include "Simulator.h"

namespace tea_gui {

/* 既定状態で構築します。 */
Simulator::Simulator() {
  batch_.set_model(model_);
}

/* モデル（係数セット）を設定します（停止状態で適用します）。 */
void Simulator::set_model(ModelType type) {
  if (running_) {
    return;
  }
  model_ = type;
  batch_.set_model(model_);
  batch_.reset();
}

/* 現在モデルを返します。 */
ModelType Simulator::model() const {
  return model_;
}

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
  batch_.set_model(model_);
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



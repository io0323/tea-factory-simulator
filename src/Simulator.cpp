#include "Simulator.h"

namespace tea_gui {

/* 既定状態で構築します。 */
Simulator::Simulator() {
  set_batch_count(1);
}

/* モデル（係数セット）を設定します（停止状態で適用します）。 */
void Simulator::set_model(ModelType type) {
  if (running_) {
    return;
  }
  model_ = type;
  for (TeaBatch& b : batches_) {
    b.set_model(model_);
    b.reset();
  }
}

/* 現在モデルを返します。 */
ModelType Simulator::model() const {
  return model_;
}

/* 実行を開始します。 */
void Simulator::start() {
  if (batches_.empty()) {
    return;
  }
  if (batches_.front().process() == ProcessState::FINISHED) {
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
  for (TeaBatch& b : batches_) {
    b.set_model(model_);
    b.reset();
  }
}

/* 実行中なら deltaTime（秒）だけ更新します。 */
void Simulator::update(double delta_seconds) {
  if (!running_) {
    return;
  }
  bool any_active = false;
  for (TeaBatch& b : batches_) {
    if (b.process() != ProcessState::FINISHED) {
      b.update(delta_seconds);
    }
    if (b.process() != ProcessState::FINISHED) {
      any_active = true;
    }
  }
  running_ = any_active;
}

/* 実行中かどうかを返します。 */
bool Simulator::is_running() const {
  return running_;
}

/* TeaBatch を参照します（UI 用）。 */
const TeaBatch& Simulator::batch() const {
  return batches_.front();
}

/* バッチ数を停止状態で変更します（内部状態はリセットされます）。 */
void Simulator::set_batch_count(int count) {
  if (running_) {
    return;
  }
  batch_count_ = (count < 1) ? 1 : count;
  batches_.clear();
  batches_.resize(static_cast<std::size_t>(batch_count_));
  for (TeaBatch& b : batches_) {
    b.set_model(model_);
    b.reset();
  }
}

/* バッチ数を返します。 */
int Simulator::batch_count() const {
  return batch_count_;
}

/* 指定バッチを参照します（UI 用）。 */
const TeaBatch& Simulator::batch_at(int index) const {
  const int clamped = (index < 0) ? 0 : (index >= batch_count_ ? batch_count_ - 1 : index);
  return batches_[static_cast<std::size_t>(clamped)];
}

} /* namespace tea_gui */



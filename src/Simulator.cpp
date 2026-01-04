/*
 * @file Simulator.cpp
 * @brief シミュレーションの管理とバッチ処理
 *
 * このファイルは、GUIアプリケーションで使用されるシミュレータの
 * 主要なロジックを実装しています。複数のTeaBatchの状態を管理し、
 * シミュレーションの開始、一時停止、リセット、モデル設定などの機能を提供します。
 */

#include "Simulator.h"

namespace tea_gui {

/*
 * @brief Simulatorの既定コンストラクタです。
 *
 * デフォルトで1つのバッチで構築されます。
 */
Simulator::Simulator() {
  set_batch_count(1);
}

/*
 * @brief シミュレーションモデル（係数セット）を設定します。
 *
 * シミュレーションが実行中でない場合にのみモデルを適用できます。
 * モデル変更後、すべてのバッチは初期状態にリセットされます。
 *
 * @param type 設定するモデルのタイプ
 */
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

/*
 * @brief 現在のシミュレーションモデルを返します。
 *
 * @return 現在のModelType
 */
ModelType Simulator::model() const {
  return model_;
}

/*
 * @brief シミュレーションの実行を開始します。
 *
 * バッチが存在し、かつ最初のバッチが終了状態でない場合にのみ開始されます。
 */
void Simulator::start() {
  if (batches_.empty()) {
    return;
  }
  if (batches_.front().process() == ProcessState::FINISHED) {
    return;
  }
  running_ = true;
}

/*
 * @brief シミュレーションの実行を一時停止します。
 */
void Simulator::pause() {
  running_ = false;
}

/*
 * @brief シミュレーションを初期状態にリセットします。
 *
 * シミュレーションは停止状態になり、すべてのバッチが初期状態に戻ります。
 */
void Simulator::reset() {
  running_ = false;
  for (TeaBatch& b : batches_) {
    b.set_model(model_);
    b.reset();
  }
}

/*
 * @brief 実行中のシミュレーションをdeltaTime（秒）だけ更新します。
 *
 * シミュレーションが実行中の場合、すべてのバッチの状態を更新します。
 * すべてのバッチが終了した場合、シミュレーションは自動的に停止します。
 *
 * @param delta_seconds 更新する時間間隔（秒）
 */
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

/*
 * @brief シミュレーションが現在実行中かどうかを返します。
 *
 * @return 実行中の場合はtrue、そうでない場合はfalse
 */
bool Simulator::is_running() const {
  return running_;
}

/*
 * @brief シミュレーション内の最初のTeaBatchを参照します（UI表示用）。
 *
 * @return 最初のTeaBatchの定数参照
 */
const TeaBatch& Simulator::batch() const {
  return batches_.front();
}

/*
 * @brief シミュレーションのバッチ数を変更します。
 *
 * シミュレーションが実行中でない場合にのみ変更可能です。
 * バッチ数変更後、内部状態はリセットされます。
 *
 * @param count 設定するバッチ数（1未満の場合は1にクランプ）
 */
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

/*
 * @brief 現在のバッチ数を返します。
 *
 * @return バッチ数
 */
int Simulator::batch_count() const {
  return batch_count_;
}

/*
 * @brief 指定されたインデックスのTeaBatchを参照します（UI表示用）。
 *
 * インデックスが無効な場合、範囲内にクランプされます。
 *
 * @param index 参照するバッチのインデックス
 * @return 指定されたTeaBatchの定数参照
 */
const TeaBatch& Simulator::batch_at(int index) const {
  const int clamped = (index < 0) ? 0 : (index >= batch_count_ ? batch_count_ - 1 : index);
  return batches_[static_cast<std::size_t>(clamped)];
}

} /* namespace tea_gui */

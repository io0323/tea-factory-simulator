#pragma once

#include <vector>

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

  /* モデル（係数セット）を設定します（停止状態で適用します）。 */
  void set_model(ModelType type);

  /* 現在モデルを返します。 */
  ModelType model() const;

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

  /* バッチ数を停止状態で変更します（内部状態はリセットされます）。 */
  void set_batch_count(int count);

  /* バッチ数を返します。 */
  int batch_count() const;

  /* 指定バッチを参照します（UI 用）。 */
  const TeaBatch& batch_at(int index) const;

 private:
  bool running_ = false;
  ModelType model_ = ModelType::DEFAULT;
  int batch_count_ = 1;
  std::vector<TeaBatch> batches_;
};

} /* namespace tea_gui */



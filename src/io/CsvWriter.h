#pragma once

#include <fstream>
#include <string>

namespace tea_io {

/*
  CSV へシミュレーション状態を書き出す軽量ユーティリティです。
  標準ライブラリのみで、ヘッダ1行 + 以降のレコードを追記します。
*/
class CsvWriter final {
 public:
  /* 出力先パスを指定して構築します。 */
  explicit CsvWriter(const std::string& path);

  /* ヘッダ行を書き込みます（新規ファイル作成時のみ推奨）。 */
  void write_header();

  /* 1 行分のデータを書き込みます。 */
  void write_row(const std::string& process,
                 int elapsed_seconds,
                 double moisture,
                 double temperature_c,
                 double aroma,
                 double color);

  /* 品質スコア（0-100）を要件式で算出します。 */
  static double quality_score(double moisture, double aroma, double color);

  /* 品質ステータス（GOOD/OK/BAD）を返します。 */
  static const char* quality_status(double score);

 private:
  std::ofstream ofs_;
  bool header_written_ = false;
};

} /* namespace tea_io */




/*
 * @file CsvWriter.cpp
 * @brief CSVファイルへのデータ書き込み機能を提供
 *
 * このファイルは、シミュレーション結果をCSV形式で出力するための
 * CsvWriterクラスの実装を含んでいます。ヘッダ行の書き込み、データ行の書き込み、
 * 品質スコアの計算、品質ステータスの判定などの機能を提供します。
 */

#include "io/CsvWriter.h"

#include <algorithm> // For std::clamp
#include <iomanip>   // For std::fixed, std::setprecision

namespace te-io {

/*
 * @brief 出力先パスを指定してCsvWriterを構築します。
 *
 * 指定されたパスにファイルを開き、既存の内容を上書きします。
 *
 * @param path CSVファイルの出力パス
 */
CsvWriter::CsvWriter(const std::string& path)
    : ofs_(path, std::ios::out | std::ios::trunc) {
}

/*
 * @brief ヘッダ行をCSVファイルに書き込みます。
 *
 * ファイルがオープンされており、まだヘッダが書き込まれていない場合にのみ、
 * ヘッダ行を書き込みます。
 */
void CsvWriter::write_header() {
  if (!ofs_.is_open() || header_written_) {
    return;
  }
  ofs_ << "process,elapsedSeconds,moisture,temperatureC,aroma,color,"
          "qualityScore,qualityStatus\n";
  header_written_ = true;
}

/*
 * @brief 1行分のデータをCSVファイルに書き込みます。
 *
 * ファイルがオープンされていることを確認し、必要であればヘッダを書き込んだ後、
 * 指定されたシミュレーションデータを1行としてCSVファイルに追記します。
 *
 * @param process 現在の工程名
 * @param elapsed_seconds 経過時間（秒）
 * @param moisture 水分量
 * @param temperature_c 温度（摂氏）
 * @param aroma 香気
 * @param color 色
 */
void CsvWriter::write_row(const std::string& process,
                          int elapsed_seconds,
                          double moisture,
                          double temperature_c,
                          double aroma,
                          double color) {
  if (!ofs_.is_open()) {
    return;
  }
  if (!header_written_) {
    write_header();
  }

  const double score = quality_score(moisture, aroma, color);
  const char* status = quality_status(score);

  ofs_ << process << ',';
  ofs_ << elapsed_seconds << ',';
  ofs_ << std::fixed << std::setprecision(6) << moisture << ',';
  ofs_ << std::fixed << std::setprecision(3) << temperature_c << ',';
  ofs_ << std::fixed << std::setprecision(3) << aroma << ',';
  ofs_ << std::fixed << std::setprecision(3) << color << ',';
  ofs_ << std::fixed << std::setprecision(2) << score << ',';
  ofs_ << status << '\n';
}

/*
 * @brief 品質スコア（0-100）を要件式で算出します。
 *
 * 要件式:
 *   qualityScore = aroma * 0.4
 *                + color * 0.4
 *                + (1.0 - moisture) * 100 * 0.2
 *
 * @param moisture 水分量
 * @param aroma 香気
 * @param color 色
 * @return 算出された品質スコア
 */
double CsvWriter::quality_score(double moisture, double aroma, double color) {
  const double score =
      aroma * 0.4 + color * 0.4 + (1.0 - moisture) * 100.0 * 0.2;
  return std::clamp(score, 0.0, 100.0);
}

/*
 * @brief 品質ステータス（GOOD/OK/BAD）を返します。
 *
 * 品質スコアに基づいて、GOOD (80以上), OK (60以上), BAD (それ未満) の
 * いずれかのステータスを返します。
 *
 * @param score 品質スコア
 * @return 品質ステータス文字列
 */
const char* CsvWriter::quality_status(double score) {
  if (score >= 80.0) {
    return "GOOD";
  }
  if (score >= 60.0) {
    return "OK";
  }
  return "BAD";
}

} /* namespace te-io */

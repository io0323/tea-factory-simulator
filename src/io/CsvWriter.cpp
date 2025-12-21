#include "io/CsvWriter.h"

#include <algorithm>
#include <iomanip>

namespace tea_io {

/* 出力先パスを指定して構築します。 */
CsvWriter::CsvWriter(const std::string& path)
    : ofs_(path, std::ios::out | std::ios::trunc) {
}

/* ヘッダ行を書き込みます。 */
void CsvWriter::write_header() {
  if (!ofs_.is_open() || header_written_) {
    return;
  }
  ofs_ << "process,elapsedSeconds,moisture,temperatureC,aroma,color,"
          "qualityScore,qualityStatus\n";
  header_written_ = true;
}

/* 1 行分のデータを書き込みます。 */
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

/* 品質スコア（0-100）を要件式で算出します。 */
double CsvWriter::quality_score(double moisture, double aroma, double color) {
  /*
    要件式:
      qualityScore = aroma * 0.4
                   + color * 0.4
                   + (1.0 - moisture) * 100 * 0.2
  */
  const double score =
      aroma * 0.4 + color * 0.4 + (1.0 - moisture) * 100.0 * 0.2;
  return std::clamp(score, 0.0, 100.0);
}

/* 品質ステータス（GOOD/OK/BAD）を返します。 */
const char* CsvWriter::quality_status(double score) {
  if (score >= 80.0) {
    return "GOOD";
  }
  if (score >= 60.0) {
    return "OK";
  }
  return "BAD";
}

} /* namespace tea_io */



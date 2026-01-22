/*
 * @file test_csvwriter_io.cpp
 * @brief CsvWriter のファイルI/O（header/write_row）の検証
 *
 * 外部テストフレームワークに依存せず、CTest から実行できる最小の検証を行います。
 */

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "io/CsvWriter.h"
#include "test_utils.h"

namespace {

/*
 * @brief ファイル全体を読み込み、行単位で返します。
 *
 * @param path 読み込み対象パス
 * @return ファイル内容（行ごと）
 */
std::vector<std::string> read_lines(const std::string& path) {
  std::vector<std::string> lines;
  std::ifstream ifs(path);
  std::string line;
  while (std::getline(ifs, line)) {
    lines.push_back(line);
  }
  return lines;
}

/*
 * @brief headerが1回だけ書かれ、行が追記されることを検証します。
 *
 * @return 成功なら true
 */
bool test_header_written_once_and_rows_appended() {
  const std::string path = "csvwriter_io_test.csv";
  std::remove(path.c_str());

  {
    tea_io::CsvWriter w(path);
    w.write_header();
    w.write_header(); /* 二重呼び出ししても1回だけのはず */
    w.write_row("STEAMING", 1, 0.75, 25.0, 10.0, 10.0);
    w.write_row("STEAMING", 2, 0.70, 30.0, 12.0, 11.0);
  }

  const auto lines = read_lines(path);
  std::remove(path.c_str());

  bool ok = true;
  ok = tea_test::expect(lines.size() == 3, "file should have 3 lines") && ok;
  if (lines.size() >= 1) {
    ok = tea_test::expect(
        lines[0] ==
            "process,elapsedSeconds,moisture,temperatureC,aroma,color,"
            "qualityScore,qualityStatus",
        "header line should match exactly") && ok;
  }
  if (lines.size() >= 2) {
    ok = tea_test::expect(lines[1].find("STEAMING,1,") == 0,
                          "first row should start with process/time") && ok;
  }
  if (lines.size() >= 3) {
    ok = tea_test::expect(lines[2].find("STEAMING,2,") == 0,
                          "second row should start with process/time") && ok;
  }
  return ok;
}

} /* namespace */

/*
 * @brief テストのエントリポイントです。
 *
 * @return 0: 成功, 1: 失敗
 */
int main() {
  bool ok = true;
  ok = test_header_written_once_and_rows_appended() && ok;

  if (!ok) {
    return 1;
  }
  std::cout << "csvwriter_io_tests: OK\n";
  return 0;
}


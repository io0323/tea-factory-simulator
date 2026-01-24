/*
 * @file test_csvwriter_io.cpp
 * @brief CsvWriter のファイルI/O（header/write_row）の検証
 *
 * 外部テストフレームワークに依存せず、CTest から実行できる最小の検証を行います。
 */

#include <chrono>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "io/CsvWriter.h"
#include "test_utils.h"

namespace {

/*
 * @brief スコープ終了時にファイルを削除するガードです。
 */
class ScopedFile final {
 public:
  /* 生成したファイルパスを保持します。 */
  explicit ScopedFile(std::string path) : path_(std::move(path)) {
  }

  /* コピーは禁止します（2重削除防止）。 */
  ScopedFile(const ScopedFile&) = delete;
  ScopedFile& operator=(const ScopedFile&) = delete;

  /* ムーブは許可します。 */
  ScopedFile(ScopedFile&&) = default;
  ScopedFile& operator=(ScopedFile&&) = default;

  /* デストラクタで後始末します（失敗しても無視）。 */
  ~ScopedFile() {
    if (!path_.empty()) {
      std::remove(path_.c_str());
    }
  }

  /* パスを返します。 */
  const std::string& path() const {
    return path_;
  }

 private:
  std::string path_;
};

/*
 * @brief ほぼ一意なテスト用CSVファイル名を生成します。
 */
std::string make_temp_csv_path() {
  using clock = std::chrono::steady_clock;
  const auto now = clock::now().time_since_epoch().count();
  std::ostringstream oss;
  oss << "csvwriter_io_test_" << now << "_" << &oss << ".csv";
  return oss.str();
}

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
  ScopedFile file(make_temp_csv_path());

  {
    tea_io::CsvWriter w(file.path());
    w.write_header();
    w.write_header(); /* 二重呼び出ししても1回だけのはず */
    w.write_row("STEAMING", 1, 0.75, 25.0, 10.0, 10.0);
    w.write_row("STEAMING", 2, 0.70, 30.0, 12.0, 11.0);
  }

  const auto lines = read_lines(file.path());

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


#pragma once

#include <optional>
#include <string>

namespace tea_cli {

/*
  CLI引数を解釈して、実行設定へ変換します。
  依存を増やさず、最小限のオプションだけ扱います。
*/
struct Args final {
  int dt_seconds = 1;
  int steaming_seconds = 30;
  int rolling_seconds = 30;
  int drying_seconds = 60;

  bool csv_enabled = true;
  std::string csv_path = "tea_factory_cli.csv";

  bool show_help = false;
  std::optional<std::string> error;
};

/* 引数をパースします。失敗時は Args::error に理由を設定します。 */
Args parse_args(int argc, char** argv);

/* ヘルプ文字列を返します。 */
const char* help_text();

} /* namespace tea_cli */



/*
 * @file Args.cpp
 * @brief CLI引数のパースとヘルプテキストの管理
 *
 * このファイルは、コマンドライン引数を解析し、シミュレーション設定に
 * 必要なパラメータを抽出する機能を提供します。また、CLIのヘルプテキストも
 * 定義されています。
 */

#include "cli/Args.h"

#include <cstdlib> // For std::strtol
#include <string>  // For std::string
#include <optional> // For std::optional

namespace tea_cli {

namespace {

/*
 * @brief 文字列を正の整数へ変換します。
 *
 * 変換に失敗した場合、または結果が正の整数でない場合はstd::nulloptを返します。
 *
 * @param s 変換する文字列
 * @return 変換された正の整数、またはstd::nullopt
 */
std::optional<int> parse_positive_int(const char* s) {
  if (s == nullptr || *s == '\0') {
    return std::nullopt;
  }
  char* end = nullptr;
  const long v = std::strtol(s, &end, 10);
  if (end == s || *end != '\0') {
    return std::nullopt;
  }
  if (v <= 0 || v > 24 * 60 * 60) { // 1日（秒）を最大値とする
    return std::nullopt;
  }
  return static_cast<int>(v);
}

} /* namespace */

/*
 * @brief コマンドライン引数をパースします。
 *
 * 引数を解析し、Args構造体に格納します。パースに失敗した場合は、
 * Args::errorにエラー理由を設定します。
 *
 * @param argc 引数の数
 * @param argv 引数の文字列配列
 * @return パース結果を含むArgs構造体
 */
Args parse_args(int argc, char** argv) {
  Args args;

  for (int i = 1; i < argc; ++i) {
    const std::string a = argv[i] ? argv[i] : "";

    if (a == "-h" || a == "--help") {
      args.show_help = true;
      return args;
    }

    if (a == "--no-csv") {
      args.csv_enabled = false;
      continue;
    }

    if (a == "--dt" || a == "--steaming" || a == "--rolling" ||
        a == "--drying" || a == "--csv" || a == "--model" ||
        a == "--batches") {
      if (i + 1 >= argc) {
        args.error = "Missing value for " + a;
        return args;
      }
      const char* v = argv[++i];

      if (a == "--csv") {
        args.csv_path = v ? v : "";
        if (args.csv_path.empty()) {
          args.error = "CSV path is empty";
          return args;
        }
        continue;
      }

      if (a == "--model") {
        args.model = v ? v : "";
        if (args.model != "default" && args.model != "gentle" &&
            args.model != "aggressive") {
          args.error = "Invalid model: " + args.model;
          return args;
        }
        continue;
      }

      if (a == "--batches") {
        const auto parsed = parse_positive_int(v);
        if (!parsed.has_value() || *parsed > 128) {
          args.error = "Invalid batches: " + std::string(v ? v : "");
          return args;
        }
        args.batches = *parsed;
        continue;
      }

      const auto parsed = parse_positive_int(v);
      if (!parsed.has_value()) {
        args.error = "Invalid value for " + a + ": " + (v ? v : "");
        return args;
      }

      if (a == "--dt") {
        args.dt_seconds = *parsed;
      } else if (a == "--steaming") {
        args.steaming_seconds = *parsed;
      } else if (a == "--rolling") {
        args.rolling_seconds = *parsed;
      } else if (a == "--drying") {
        args.drying_seconds = *parsed;
      }
      continue;
    }

    args.error = "Unknown argument: " + a;
    return args;
  }

  if (args.dt_seconds <= 0) {
    args.error = "dt_seconds must be > 0";
    return args;
  }
  if (args.steaming_seconds <= 0 || args.rolling_seconds <= 0 ||
      args.drying_seconds <= 0) {
    args.error = "stage seconds must be > 0";
    return args;
  }

  return args;
}

/*
 * @brief CLIのヘルプテキストを返します。
 *
 * コマンドラインオプションとその説明を含む文字列を返します。
 *
 * @return ヘルプテキスト
 */
const char* help_text() {
  return
      "TeaFactory Simulator (CLI)\n"
      "\n"
      "Usage:\n"
      "  tea_factory_simulator_cli [options]\n"
      "\n"
      "Options:\n"
      "  --dt <sec>        Time step seconds (default: 1)\n"
      "  --steaming <sec>  Steaming duration (default: 30)\n"
      "  --rolling <sec>   Rolling duration (default: 30)\n"
      "  --drying <sec>    Drying duration (default: 60)\n"
      "  --model <name>    Model: default|gentle|aggressive\n"
      "  --batches <n>     Batch count (default: 1, max: 128)\n"
      "  --csv <path>      CSV output path (default: tea_factory_cli.csv)\n"
      "  --no-csv          Disable CSV output\n"
      "  -h, --help        Show help\n";
}

} /* namespace tea_cli */

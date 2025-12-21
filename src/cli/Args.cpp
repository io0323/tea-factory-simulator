#include "cli/Args.h"

#include <cstdlib>

namespace tea_cli {

namespace {

/* 文字列を正の整数へ変換します（失敗時はnullopt）。 */
std::optional<int> parse_positive_int(const char* s) {
  if (s == nullptr || *s == '\0') {
    return std::nullopt;
  }
  char* end = nullptr;
  const long v = std::strtol(s, &end, 10);
  if (end == s || *end != '\0') {
    return std::nullopt;
  }
  if (v <= 0 || v > 24 * 60 * 60) {
    return std::nullopt;
  }
  return static_cast<int>(v);
}

} /* namespace */

/* 引数をパースします。失敗時は Args::error に理由を設定します。 */
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
        a == "--drying" || a == "--csv" || a == "--model") {
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
  if (args.dt_seconds > args.steaming_seconds ||
      args.dt_seconds > args.rolling_seconds ||
      args.dt_seconds > args.drying_seconds) {
    /*
      dt が工程時間より大きいと、工程が1ステップも進まない可能性があるため
      ここで弾きます（次タスクで柔軟化してもOK）。
    */
    args.error = "dt must be <= each stage seconds";
    return args;
  }

  return args;
}

/* ヘルプ文字列を返します。 */
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
      "  --csv <path>      CSV output path (default: tea_factory_cli.csv)\n"
      "  --no-csv          Disable CSV output\n"
      "  -h, --help        Show help\n";
}

} /* namespace tea_cli */



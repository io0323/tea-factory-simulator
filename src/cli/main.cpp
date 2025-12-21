#include <iostream>

#include "cli/Args.h"
#include "io/CsvWriter.h"
#include "domain/Model.h"
#include "simulation/Simulator.h"

/* CLI エントリポイントです。引数に応じて設定し、ログ/CSVを出力します。 */
int main(int argc, char** argv) {
  const tea_cli::Args args = tea_cli::parse_args(argc, argv);
  if (args.error.has_value()) {
    std::cerr << "Error: " << *args.error << "\n\n";
    std::cerr << tea_cli::help_text();
    return 2;
  }
  if (args.show_help) {
    std::cout << tea_cli::help_text();
    return 0;
  }

  tea::SimulationConfig config;
  config.dt_seconds = args.dt_seconds;
  config.steaming_seconds = args.steaming_seconds;
  config.rolling_seconds = args.rolling_seconds;
  config.drying_seconds = args.drying_seconds;
  if (args.model == "gentle") {
    config.model = tea::ModelType::GENTLE;
  } else if (args.model == "aggressive") {
    config.model = tea::ModelType::AGGRESSIVE;
  } else {
    config.model = tea::ModelType::DEFAULT;
  }

  tea::Simulator simulator(config);

  if (!args.csv_enabled) {
    simulator.run(std::cout, nullptr);
    return 0;
  }

  tea_io::CsvWriter csv(args.csv_path);
  csv.write_header();
  simulator.run(std::cout, &csv);
  return 0;
}



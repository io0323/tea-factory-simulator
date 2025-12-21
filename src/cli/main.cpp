#include <iostream>

#include "io/CsvWriter.h"
#include "simulation/Simulator.h"

/* CLI エントリポイントです。既定設定で全工程を実行してログ出力します。 */
int main() {
  tea::Simulator simulator;

  /*
    CSV はカレントディレクトリに tea_factory_cli.csv として出力します。
    次タスク（CLI引数化）でパス指定に対応します。
  */
  tea_io::CsvWriter csv("tea_factory_cli.csv");
  csv.write_header();
  simulator.run(std::cout, &csv);
  return 0;
}



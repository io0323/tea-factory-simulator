#include <iostream>

#include "simulation/Simulator.h"

/* CLI エントリポイントです。既定設定で全工程を実行してログ出力します。 */
int main() {
  tea::Simulator simulator;
  simulator.run(std::cout);
  return 0;
}



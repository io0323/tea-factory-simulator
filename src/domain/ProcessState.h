#pragma once

namespace tea {

/* 製造工程の状態を表す列挙です。 */
enum class ProcessState {
  STEAMING,
  ROLLING,
  DRYING,
  FINISHED
};

/* 工程名をログ出力用の文字列に変換します。 */
inline const char* to_string(ProcessState s) {
  switch (s) {
    case ProcessState::STEAMING:
      return "STEAMING";
    case ProcessState::ROLLING:
      return "ROLLING";
    case ProcessState::DRYING:
      return "DRYING";
    case ProcessState::FINISHED:
      return "FINISHED";
  }
  return "UNKNOWN";
}

} /* namespace tea */




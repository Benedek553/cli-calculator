#pragma once

#include "cli_output.hpp"

#include <optional>
#include <string>
#include <utility>
#include <vector>

enum class CliActionType {
  None,
  Eval,
  SquareRoot,
  Divisors,
  Convert,
  UnitConvert,
  PrimeFactorization,
  SolveLinear,
  SolveQuadratic,
  MatrixAdd,
  MatrixSubtract,
  MatrixMultiply,
  Statistics,
  GraphValues,
  GraphCsv,
  Version,
  Variables,
  SetVariable,
  UnsetVariable,
  ReportBug,
  Help,
  Repl,
  Batch
};

struct CliAction {
  CliActionType type = CliActionType::None;
  std::vector<std::string> params;
};

struct CliParseResult {
  bool colorsEnabled = true;
  OutputFormat outputFormat = OutputFormat::Text;
  bool sawNonColorArgument = false;
  bool useBigInt = false;
  bool useBigDouble = false;
  std::optional<CliAction> action;
};

struct CliParseError {
  std::string message;
  std::string actionId;
  int exitCode = 1;
};

class CliParser {
public:
  std::pair<CliParseResult, std::optional<CliParseError>>
  parse(int argc, char **argv) const;
};

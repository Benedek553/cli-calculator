#pragma once

#include "cli_parser.hpp"

#include <optional>

class CalculatorApp {
public:
  int run(int argc, char **argv);

private:
  static std::optional<int> executeCliAction(const CliParseResult &parseResult);

  static int dispatchAction(const CliAction &action, OutputFormat format,
                            bool useBigInt, bool useBigDouble);
};

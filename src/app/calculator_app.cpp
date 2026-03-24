#include "calculator_app.hpp"

#include "ansi_colors.hpp"
#include "cli_batch.hpp"
#include "cli_commands.hpp"
#include "cli_output.hpp"
#include "cli_repl.hpp"
#include "core/variables.hpp"
#include "menu_handlers.hpp"

#include <iostream>

std::optional<int>
CalculatorApp::executeCliAction(const CliParseResult &parseResult) {
  if (!parseResult.action) {
    if (parseResult.outputFormat != OutputFormat::Text &&
        parseResult.sawNonColorArgument) {
      printStructuredError(std::cerr, parseResult.outputFormat, "output",
                           "structured output requires a CLI action flag");
      return 1;
    }
    return std::nullopt;
  }
  return dispatchAction(*parseResult.action, parseResult.outputFormat,
                        parseResult.useBigInt, parseResult.useBigDouble);
}

int CalculatorApp::dispatchAction(const CliAction &action,
                                  OutputFormat format, bool useBigInt,
                                  bool useBigDouble) {
  switch (action.type) {
  case CliActionType::Eval:
    return runEval(action.params.empty() ? "" : action.params.front(), format,
                   nullptr, useBigInt, useBigDouble);
  case CliActionType::SquareRoot:
    return runSquareRoot(action.params.empty() ? "" : action.params.front(),
                         format);
  case CliActionType::Divisors:
    return runDivisors(action.params.empty() ? "" : action.params.front(),
                       format);
  case CliActionType::Convert:
    if (action.params.size() < 3) {
      printStructuredError(std::cerr, format, "convert",
                           "missing arguments after --convert");
      return 2;
    }
    return runConvert(action.params[0], action.params[1], action.params[2],
                      format);
  case CliActionType::UnitConvert:
    if (action.params.size() < 4) {
      printStructuredError(std::cerr, format, "unit-convert",
                           "missing arguments after --unit-convert");
      return 2;
    }
    return runUnitConvert(action.params[0], action.params[1], action.params[2],
                          action.params[3], format);
  case CliActionType::PrimeFactorization:
    return runPrimeFactorization(
        action.params.empty() ? "" : action.params.front(), format);
  case CliActionType::SolveLinear:
    if (action.params.size() < 2) {
      printStructuredError(std::cerr, format, "solve-linear",
                           "missing arguments after --solve-linear");
      return 2;
    }
    return runSolveLinear(action.params[0], action.params[1], format);
  case CliActionType::SolveQuadratic:
    if (action.params.size() < 3) {
      printStructuredError(std::cerr, format, "solve-quadratic",
                           "missing arguments after --solve-quadratic");
      return 2;
    }
    return runSolveQuadratic(action.params[0], action.params[1], action.params[2],
                             format);
  case CliActionType::MatrixAdd:
    if (action.params.size() < 2) {
      printStructuredError(std::cerr, format, "matrix-add",
                           "missing arguments after --matrix-add");
      return 2;
    }
    return runMatrixAdd(action.params[0], action.params[1], format);
  case CliActionType::MatrixSubtract:
    if (action.params.size() < 2) {
      printStructuredError(std::cerr, format, "matrix-subtract",
                           "missing arguments after --matrix-subtract");
      return 2;
    }
    return runMatrixSubtract(action.params[0], action.params[1], format);
  case CliActionType::MatrixMultiply:
    if (action.params.size() < 2) {
      printStructuredError(std::cerr, format, "matrix-multiply",
                           "missing arguments after --matrix-multiply");
      return 2;
    }
    return runMatrixMultiply(action.params[0], action.params[1], format);
  case CliActionType::Statistics:
    return runStatistics(action.params, format);
  case CliActionType::GraphValues:
    return runGraphValues(action.params, format);
  case CliActionType::GraphCsv:
    return runGraphCsv(action.params, format);
  case CliActionType::Version:
    return runVersion(format);
  case CliActionType::Variables:
    return runListVariables(format);
  case CliActionType::SetVariable:
    if (action.params.size() < 2) {
      printStructuredError(std::cerr, format, "set-variable",
                           "missing arguments after --set-variable");
      return 2;
    }
    return runSetVariable(action.params[0], action.params[1], format);
  case CliActionType::UnsetVariable:
    if (action.params.empty()) {
      printStructuredError(std::cerr, format, "unset-variable",
                           "missing arguments after --unset-variable");
      return 2;
    }
    return runUnsetVariable(action.params[0], format);
  case CliActionType::ReportBug:
    return runReportBug(format);
  case CliActionType::Help:
    return runHelp(format);
  case CliActionType::Repl:
    if (format != OutputFormat::Text) {
      printStructuredError(std::cerr, format, "repl",
                           "REPL mode only supports plain-text output.");
      return 1;
    }
    return runRepl(format);
  case CliActionType::Batch: {
    BatchState batchState;
    if (action.params.empty()) {
      return processBatchFileInteractive(format, batchState);
    }
    return processBatchFile(action.params.front(), format, batchState);
  }
  case CliActionType::None:
    break;
  }
  return 0;
}

int CalculatorApp::run(int argc, char **argv) {
  CliParser parser;
  auto [parseResult, parseError] = parser.parse(argc, argv);

  setColorsEnabled(parseResult.colorsEnabled);

  if (parseError) {
    if (parseResult.outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << RESET << parseError->message << '\n';
    } else {
      printStructuredError(std::cerr, parseResult.outputFormat,
                           parseError->actionId, parseError->message);
    }
    return parseError->exitCode;
  }

  if (!globalVariableStore().load()) {
    std::cerr << RED
              << "Warning: unable to load vars.toml; variable changes will not "
                 "persist."
              << RESET << '\n';
  }

  if (auto exitCode = executeCliAction(parseResult)) {
    return *exitCode;
  }

  std::cout << BOLD << BLUE << "Welcome to the CLI Calculator!" << RESET
            << '\n';
  runInteractiveCalculator();
  return 0;
}

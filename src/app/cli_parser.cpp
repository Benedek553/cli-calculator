#include "cli_parser.hpp"

#include <algorithm>
#include <cctype>
#include <iterator>
#include <string>
#include <utility>

namespace {
bool isNoColorFlag(const std::string &arg) {
  return arg == "--no-color" || arg == "-nc";
}

bool parseOutputFormatToken(const std::string &token, OutputFormat &format) {
  std::string normalized;
  normalized.reserve(token.size());
  std::transform(token.begin(), token.end(), std::back_inserter(normalized),
                 [](unsigned char ch) {
                   return static_cast<char>(std::tolower(ch));
                 });
  if (normalized == "json") {
    format = OutputFormat::Json;
    return true;
  }
  if (normalized == "xml") {
    format = OutputFormat::Xml;
    return true;
  }
  if (normalized == "yaml" || normalized == "yml") {
    format = OutputFormat::Yaml;
    return true;
  }
  if (normalized == "text") {
    format = OutputFormat::Text;
    return true;
  }
  return false;
}

CliParseError makeError(std::string message, std::string actionId,
                        int exitCode) {
  return CliParseError{std::move(message), std::move(actionId), exitCode};
}

CliAction makeAction(CliActionType type,
                     std::vector<std::string> params = {}) {
  CliAction action;
  action.type = type;
  action.params = std::move(params);
  return action;
}
} // namespace

std::pair<CliParseResult, std::optional<CliParseError>>
CliParser::parse(int argc, char **argv) const {
  CliParseResult result;

  for (int i = 1; i < argc; ++i) {
    std::string arg(argv[i]);
    if (isNoColorFlag(arg)) {
      result.colorsEnabled = false;
      continue;
    }
    if (arg == "--bigint") {
      if (result.useBigDouble) {
        return {result,
                makeError("cannot combine --bigint and --bigdouble.",
                          "bigint", 1)};
      }
      result.useBigInt = true;
      result.sawNonColorArgument = true;
      continue;
    }
    if (arg == "--bigdouble") {
      if (result.useBigInt) {
        return {result,
                makeError("cannot combine --bigint and --bigdouble.",
                          "bigdouble", 1)};
      }
      result.useBigDouble = true;
      result.sawNonColorArgument = true;
      continue;
    }
    if (arg == "--output") {
      result.sawNonColorArgument = true;
      if (i + 1 >= argc) {
        return {result,
                makeError("missing format after --output.", "output", 1)};
      }
      std::string formatToken(argv[i + 1]);
      if (!parseOutputFormatToken(formatToken, result.outputFormat)) {
        std::string message =
            "unsupported output format: " + formatToken + '.';
        return {result, makeError(message, "output", 1)};
      }
      ++i;
      continue;
    }
    if (!arg.empty()) {
      result.sawNonColorArgument = true;
    }
  }

  for (int i = 1; i < argc; ++i) {
    std::string arg(argv[i]);
    if (isNoColorFlag(arg)) {
      continue;
    }
    if (arg == "--bigint") {
      continue;
    }
    if (arg == "--bigdouble") {
      continue;
    }
    if (arg == "--output") {
      ++i;
      continue;
    }

    result.sawNonColorArgument = true;

    if (result.action) {
      break;
    }

    if (arg == "--batch" || arg == "--batch-file" || arg == "-b") {
      std::vector<std::string> params;
      if (i + 1 < argc) {
        params.emplace_back(argv[i + 1]);
        ++i;
      }
      result.action = makeAction(CliActionType::Batch, params);
      break;
    }

    if (arg == "--eval" || arg == "-e") {
      if (i + 1 >= argc) {
        std::string message = "missing expression after " + arg;
        return {result, makeError(message, "eval", 1)};
      }
      result.action =
          makeAction(CliActionType::Eval, {std::string(argv[i + 1])});
      break;
    }

    if (arg == "--square-root" || arg == "-sqrt") {
      if (i + 1 >= argc) {
        std::string message = "missing value after " + arg;
        return {result, makeError(message, "square-root", 1)};
      }
      result.action =
          makeAction(CliActionType::SquareRoot, {std::string(argv[i + 1])});
      break;
    }

    if (arg == "--divisors" || arg == "-d") {
      if (i + 1 >= argc) {
        std::string message = "missing value after " + arg;
        return {result, makeError(message, "divisors", 2)};
      }
      result.action =
          makeAction(CliActionType::Divisors, {std::string(argv[i + 1])});
      break;
    }

    if (arg == "--convert" || arg == "-c") {
      if (i + 3 >= argc) {
        std::string message = "missing arguments after " + arg;
        return {result, makeError(message, "convert", 2)};
      }
      result.action = makeAction(CliActionType::Convert,
                                 {std::string(argv[i + 1]),
                                  std::string(argv[i + 2]),
                                  std::string(argv[i + 3])});
      break;
    }

    if (arg == "--unit-convert") {
      if (i + 4 >= argc) {
        std::string message = "missing arguments after " + arg;
        return {result, makeError(message, "unit-convert", 2)};
      }
      result.action = makeAction(CliActionType::UnitConvert,
                                 {std::string(argv[i + 1]),
                                  std::string(argv[i + 2]),
                                  std::string(argv[i + 3]),
                                  std::string(argv[i + 4])});
      break;
    }

    if (arg == "--prime-factorization" || arg == "-pf") {
      if (i + 1 >= argc) {
        std::string message = "missing arguments after " + arg;
        return {result, makeError(message, "prime-factorization", 2)};
      }
      result.action = makeAction(CliActionType::PrimeFactorization,
                                 {std::string(argv[i + 1])});
      break;
    }

    if (arg == "--solve-linear") {
      if (i + 2 >= argc) {
        std::string message = "missing arguments after " + arg;
        return {result, makeError(message, "solve-linear", 2)};
      }
      result.action = makeAction(CliActionType::SolveLinear,
                                 {std::string(argv[i + 1]),
                                  std::string(argv[i + 2])});
      break;
    }

    if (arg == "--solve-quadratic") {
      if (i + 3 >= argc) {
        std::string message = "missing arguments after " + arg;
        return {result, makeError(message, "solve-quadratic", 2)};
      }
      result.action = makeAction(CliActionType::SolveQuadratic,
                                 {std::string(argv[i + 1]),
                                  std::string(argv[i + 2]),
                                  std::string(argv[i + 3])});
      break;
    }

    if (arg == "--matrix-add") {
      if (i + 2 >= argc) {
        std::string message = "missing arguments after " + arg;
        return {result, makeError(message, "matrix-add", 2)};
      }
      result.action = makeAction(CliActionType::MatrixAdd,
                                 {std::string(argv[i + 1]),
                                  std::string(argv[i + 2])});
      break;
    }

    if (arg == "--matrix-subtract") {
      if (i + 2 >= argc) {
        std::string message = "missing arguments after " + arg;
        return {result, makeError(message, "matrix-subtract", 2)};
      }
      result.action = makeAction(CliActionType::MatrixSubtract,
                                 {std::string(argv[i + 1]),
                                  std::string(argv[i + 2])});
      break;
    }

    if (arg == "--matrix-multiply") {
      if (i + 2 >= argc) {
        std::string message = "missing arguments after " + arg;
        return {result, makeError(message, "matrix-multiply", 2)};
      }
      result.action = makeAction(CliActionType::MatrixMultiply,
                                 {std::string(argv[i + 1]),
                                  std::string(argv[i + 2])});
      break;
    }

    if (arg == "--stats" || arg == "--statistics") {
      std::vector<std::string> params;
      for (int j = i + 1; j < argc; ++j) {
        std::string token(argv[j]);
        if (token == "--output" || isNoColorFlag(token)) {
          break;
        }
        params.emplace_back(std::move(token));
      }
      result.action = makeAction(CliActionType::Statistics, params);
      break;
    }

    if (arg == "--graph-values") {
      std::vector<std::string> params;
      for (int j = i + 1; j < argc; ++j) {
        std::string token(argv[j]);
        if (token == "--output" || isNoColorFlag(token)) {
          break;
        }
        params.emplace_back(std::move(token));
      }
      result.action = makeAction(CliActionType::GraphValues, params);
      break;
    }

    if (arg == "--graph-csv") {
      std::vector<std::string> params;
      for (int j = i + 1; j < argc; ++j) {
        std::string token(argv[j]);
        if (token == "--output" || isNoColorFlag(token)) {
          break;
        }
        params.emplace_back(std::move(token));
      }
      result.action = makeAction(CliActionType::GraphCsv, params);
      break;
    }

    if (arg == "--version" || arg == "-v") {
      result.action = makeAction(CliActionType::Version);
      break;
    }

    if (arg == "--variables" || arg == "--list-variables") {
      result.action = makeAction(CliActionType::Variables);
      break;
    }

    if (arg == "--set-variable") {
      if (i + 2 >= argc) {
        std::string message = "missing arguments after " + arg;
        return {result, makeError(message, "set-variable", 2)};
      }
      result.action = makeAction(CliActionType::SetVariable,
                                 {std::string(argv[i + 1]),
                                  std::string(argv[i + 2])});
      break;
    }

    if (arg == "--unset-variable") {
      if (i + 1 >= argc) {
        std::string message = "missing arguments after " + arg;
        return {result, makeError(message, "unset-variable", 2)};
      }
      result.action = makeAction(CliActionType::UnsetVariable,
                                 {std::string(argv[i + 1])});
      break;
    }

    if (arg == "--report-bug") {
      result.action = makeAction(CliActionType::ReportBug);
      break;
    }

    if (arg == "--help" || arg == "-h") {
      result.action = makeAction(CliActionType::Help);
      break;
    }

    if (arg == "--repl" || arg == "-r") {
      result.action = makeAction(CliActionType::Repl);
      break;
    }

    return {result,
            makeError("unknown argument: " + arg, "unknown-argument", 1)};
  }

  return {result, std::nullopt};
}

#include "cli_commands.hpp"
#include "ansi_colors.hpp"
#include "cli_numeric.hpp"
#include "core/graph_png.hpp"
#include "core/matrix.hpp"
#include "core/parse_utils.hpp"
#include "core/statistics.hpp"
#include "core/unit_conversion.hpp"
#include "core/variables.hpp"
#include "divisors.hpp"
#include "expression.hpp"
#include "math_utils.hpp"
#include "numeral_conversion.hpp"
#include "prime_factors.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <complex>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <vector>

#ifndef CLI_CALCULATOR_VERSION
#define CLI_CALCULATOR_VERSION "1.7"
#endif

namespace {
std::string trimCopy(const std::string &text) {
  auto isSpace = [](unsigned char ch) { return std::isspace(ch) != 0; };
  auto begin = std::find_if_not(text.begin(), text.end(), isSpace);
  if (begin == text.end()) {
    return "";
  }
  auto rbegin = std::find_if_not(text.rbegin(), text.rend(), isSpace);
  return {begin, rbegin.base()};
}

std::string toLowerCopy(std::string value) {
  std::transform(
      value.begin(), value.end(), value.begin(),
      [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
  return value;
}

std::string normalizeLookupToken(const std::string &token) {
  std::string normalized;
  normalized.reserve(token.size());
  for (unsigned char ch : token) {
    if (std::isalnum(ch) != 0 || ch == '^') {
      normalized.push_back(static_cast<char>(std::tolower(ch)));
    }
  }
  return normalized;
}

const LinearCategory *findLinearCategory(const std::string &token) {
  std::string normalized = normalizeLookupToken(token);
  for (const auto &category : linearCategories()) {
    if (normalizeLookupToken(category.name) == normalized) {
      return &category;
    }
  }
  return nullptr;
}

const LinearUnit *findLinearUnit(const LinearCategory &category,
                                 const std::string &token) {
  std::string normalized = normalizeLookupToken(token);
  for (const auto &unit : category.units) {
    if (normalizeLookupToken(unit.name) == normalized ||
        normalizeLookupToken(unit.symbol) == normalized) {
      return &unit;
    }
  }
  return nullptr;
}

const TemperatureUnit *findTemperatureUnit(const std::string &token) {
  std::string normalized = normalizeLookupToken(token);
  for (const auto &unit : temperatureUnits()) {
    if (normalizeLookupToken(unit.name) == normalized ||
        normalizeLookupToken(unit.symbol) == normalized) {
      return &unit;
    }
  }
  return nullptr;
}

bool parseValueList(const std::string &input, std::vector<double> &values,
                    std::string &error) {
  values.clear();
  std::string sanitized = input;
  std::replace(sanitized.begin(), sanitized.end(), ',', ' ');
  std::istringstream stream(sanitized);
  std::string token;
  while (stream >> token) {
    double value = 0.0;
    if (!resolveDoubleArgument(token, value, error)) {
      return false;
    }
    values.push_back(value);
  }
  if (values.empty()) {
    error = "please provide at least one numeric value";
    return false;
  }
  return true;
}

std::string joinTokens(const std::vector<std::string> &tokens,
                       std::size_t start = 0) {
  std::ostringstream joined;
  for (std::size_t idx = start; idx < tokens.size(); ++idx) {
    if (idx > start) {
      joined << ' ';
    }
    joined << tokens[idx];
  }
  return joined.str();
}

bool parseMatrix(const std::string &input, Matrix &matrix,
                 std::string &error) {
  matrix.clear();
  std::string normalized = input;
  std::replace(normalized.begin(), normalized.end(), '|', ';');
  std::vector<std::string> rows;
  std::string row;
  std::istringstream rowStream(normalized);
  while (std::getline(rowStream, row, ';')) {
    row = trimCopy(row);
    if (!row.empty()) {
      rows.push_back(row);
    }
  }
  if (rows.empty()) {
    error = "matrix input is empty";
    return false;
  }
  std::size_t expectedColumns = 0;
  for (const auto &rowText : rows) {
    std::string sanitized = rowText;
    std::replace(sanitized.begin(), sanitized.end(), ',', ' ');
    std::istringstream valueStream(sanitized);
    std::string token;
    std::vector<double> rowValues;
    while (valueStream >> token) {
      double value = 0.0;
      if (!resolveDoubleArgument(token, value, error)) {
        return false;
      }
      rowValues.push_back(value);
    }
    if (rowValues.empty()) {
      error = "matrix rows must contain at least one value";
      return false;
    }
    if (expectedColumns == 0) {
      expectedColumns = rowValues.size();
    } else if (rowValues.size() != expectedColumns) {
      error = "matrix rows have inconsistent column counts";
      return false;
    }
    matrix.push_back(std::move(rowValues));
  }
  return true;
}

void printMatrix(const Matrix &matrix) {
  std::cout << GREEN << "Resulting matrix:" << RESET << '\n';
  std::streamsize previousPrecision = std::cout.precision();
  std::ios::fmtflags previousFlags = std::cout.flags();
  std::cout << std::fixed << std::setprecision(4);
  for (const auto &row : matrix) {
    std::cout << "  ";
    for (const auto &value : row) {
      std::cout << std::setw(12) << value;
    }
    std::cout << '\n';
  }
  std::cout.precision(previousPrecision);
  std::cout.flags(previousFlags);
}

std::string jsonMatrix(const Matrix &matrix) {
  std::ostringstream json;
  json << '[';
  for (std::size_t r = 0; r < matrix.size(); ++r) {
    if (r > 0) {
      json << ',';
    }
    json << '[';
    for (std::size_t c = 0; c < matrix[r].size(); ++c) {
      if (c > 0) {
        json << ',';
      }
      json << matrix[r][c];
    }
    json << ']';
  }
  json << ']';
  return json.str();
}

std::string xmlMatrix(const Matrix &matrix) {
  std::ostringstream xml;
  xml << "<matrix>";
  for (const auto &row : matrix) {
    xml << "<row>";
    for (double value : row) {
      xml << "<value>" << value << "</value>";
    }
    xml << "</row>";
  }
  xml << "</matrix>";
  return xml.str();
}

std::string yamlMatrix(const Matrix &matrix) {
  std::ostringstream yaml;
  yaml << '[';
  for (std::size_t rowIdx = 0; rowIdx < matrix.size(); ++rowIdx) {
    if (rowIdx > 0) {
      yaml << ", ";
    }
    yaml << '[';
    for (std::size_t colIdx = 0; colIdx < matrix[rowIdx].size(); ++colIdx) {
      if (colIdx > 0) {
        yaml << ", ";
      }
      yaml << matrix[rowIdx][colIdx];
    }
    yaml << ']';
  }
  yaml << ']';
  return yaml.str();
}

std::string ensurePngExtension(std::string path) {
  std::string lowered = toLowerCopy(path);
  if (lowered.size() < 4 ||
      lowered.substr(lowered.size() - 4) != ".png") {
    path += ".png";
  }
  return path;
}

bool parseGraphHeight(const std::string &token, std::size_t &height,
                      std::string &error) {
  try {
    long long parsed = std::stoll(token);
    if (parsed < 2 || parsed > 20) {
      error = "height must be between 2 and 20";
      return false;
    }
    height = static_cast<std::size_t>(parsed);
    return true;
  } catch (const std::exception &) {
    error = "invalid height value";
    return false;
  }
}

void openUrl(const std::string &url) {
#if defined(__linux__)
    int ret = std::system("command -v snapctl >/dev/null 2>&1");
    if (ret == 0) {
        std::system(("snapctl user-open \"" + url + "\"").c_str());
    } else {
        std::system(("xdg-open \"" + url + "\"").c_str());
    }

#elif defined(_WIN32)
    std::system(("start \"\" \"" + url + "\"").c_str());

#elif defined(__APPLE__)
    std::system(("open \"" + url + "\"").c_str());

#else
    std::cout << YELLOW << "Warning: unable to open URL on this platform. Please open the following URL manually:\n"
              << url << RESET << std::endl;
#endif
}
} // namespace

int runEval(const std::string &expression, OutputFormat outputFormat,
            std::optional<double> *lastResult) {
  if (lastResult) {
    lastResult->reset();
  }
  try {
    double result =
        evaluateExpression(expression, globalVariableStore().variables());
    if (outputFormat == OutputFormat::Text) {
      std::cout << GREEN << "Result: " << RESET << result << '\n';
    } else {
      std::ostringstream jsonPayload;
      jsonPayload << "\"expression\":\"" << jsonEscape(expression)
                  << "\",\"result\":" << result;
      std::ostringstream xmlPayload;
      xmlPayload << "<expression>" << xmlEscape(expression)
                 << "</expression><result>" << result << "</result>";
      std::ostringstream yamlPayload;
      yamlPayload << "expression: " << yamlEscape(expression) << '\n'
                  << "result: " << result;
      printStructuredSuccess(std::cout, outputFormat, "eval", jsonPayload.str(),
                             xmlPayload.str(), yamlPayload.str());
    }
    if (lastResult) {
      *lastResult = result;
    }
    return 0;
  } catch (const std::exception &ex) {
    if (outputFormat == OutputFormat::Text) {
      std::cout << RED << "Error: " << RESET << ex.what() << '\n';
    } else {
      printStructuredError(std::cout, outputFormat, "eval", ex.what());
    }
    if (lastResult) {
      lastResult->reset();
    }
    return 1;
  }
}

int runSquareRoot(const std::string &number, OutputFormat outputFormat,
                  std::optional<double> *lastResult) {
  if (lastResult) {
    lastResult->reset();
  }
  double value = 0.0;
  std::string parseError;
  if (!resolveDoubleArgument(number, value, parseError)) {
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << parseError << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "square-root", parseError);
    }
    return 1;
  }
  if (value < 0.0) {
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: square root undefined for negative values."
                << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "square-root",
                           "square root undefined for negative values.");
    }
    return 1;
  }
  double result = std::sqrt(value);
  if (outputFormat == OutputFormat::Text) {
    std::cout << GREEN << "Result: " << RESET << result << '\n';
  } else {
    std::ostringstream jsonPayload;
    jsonPayload << "\"value\":" << value << ",\"result\":" << result;
    std::ostringstream xmlPayload;
    xmlPayload << "<value>" << value << "</value><result>" << result
               << "</result>";
    std::ostringstream yamlPayload;
    yamlPayload << "value: " << value << '\n' << "result: " << result;
    printStructuredSuccess(std::cout, outputFormat, "square-root",
                           jsonPayload.str(), xmlPayload.str(),
                           yamlPayload.str());
  }
  if (lastResult) {
    *lastResult = result;
  }
  return 0;
}

int runVersion(OutputFormat outputFormat) {
  constexpr const char *version = CLI_CALCULATOR_VERSION;
  if (outputFormat == OutputFormat::Text) {
    std::cout << BOLD << BLUE << "CLI Calculator version " << RESET << version
              << '\n';
  } else {
    std::ostringstream jsonPayload;
    jsonPayload << "\"version\":\"" << jsonEscape(version) << "\"";

    std::ostringstream xmlPayload;
    xmlPayload << "<version>" << xmlEscape(version) << "</version>";

    std::ostringstream yamlPayload;
    yamlPayload << "version: " << yamlEscape(version);

    printStructuredSuccess(std::cout, outputFormat, "version",
                           jsonPayload.str(), xmlPayload.str(),
                           yamlPayload.str());
  }
  return 0;
}

int runListVariables(OutputFormat outputFormat) {
  const auto &vars = globalVariableStore().variables();
  if (outputFormat == OutputFormat::Text) {
    if (vars.empty()) {
      std::cout << YELLOW << "No variables stored." << RESET << '\n';
      return 0;
    }

    std::cout << GREEN << "Stored variables:" << RESET << '\n';
    for (const auto &entry : vars) {
      std::cout << " - " << entry.first << " = " << entry.second << '\n';
    }
  } else {
    std::ostringstream jsonPayload;
    jsonPayload << "\"variables\":[";
    bool first = true;
    for (const auto &entry : vars) {
      if (!first) {
        jsonPayload << ',';
      }
      first = false;
      jsonPayload << "{\"name\":\"" << jsonEscape(entry.first)
                  << "\",\"value\":" << entry.second << "}";
    }
    jsonPayload << ']';

    std::ostringstream xmlPayload;
    xmlPayload << "<variables>";
    for (const auto &entry : vars) {
      xmlPayload << "<variable name=\"" << xmlEscape(entry.first) << "\">"
                 << entry.second << "</variable>";
    }
    xmlPayload << "</variables>";

    std::ostringstream yamlPayload;
    yamlPayload << "variables:";
    for (const auto &entry : vars) {
      yamlPayload << "\n  - name: " << yamlEscape(entry.first)
                  << "\n    value: " << entry.second;
    }
    if (vars.empty()) {
      yamlPayload << " []";
    }

    printStructuredSuccess(std::cout, outputFormat, "variables",
                           jsonPayload.str(), xmlPayload.str(),
                           yamlPayload.str());
  }
  return 0;
}

int runDivisors(const std::string &input, OutputFormat outputFormat) {
  long long n = 0;
  std::string parseError;
  if (!resolveIntegerArgument(input, n, parseError)) {
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << parseError << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "divisors", parseError);
    }
    return 1;
  }

  std::vector<long long> result = calculateDivisors(n);
  if (outputFormat == OutputFormat::Text) {
    std::cout << GREEN << "Divisors: " << RESET;
    for (std::size_t idx = 0; idx < result.size(); ++idx) {
      if (idx > 0) {
        std::cout << ", ";
      }
      std::cout << result[idx];
    }
    std::cout << '\n';
  } else {
    std::ostringstream jsonPayload;
    jsonPayload << "\"number\":" << n << ",\"divisors\":[";
    for (std::size_t idx = 0; idx < result.size(); ++idx) {
      if (idx > 0) {
        jsonPayload << ',';
      }
      jsonPayload << result[idx];
    }
    jsonPayload << ']';

    std::ostringstream xmlPayload;
    xmlPayload << "<number>" << n << "</number><divisors>";
    for (long long value : result) {
      xmlPayload << "<divisor>" << value << "</divisor>";
    }
    xmlPayload << "</divisors>";

    std::ostringstream yamlPayload;
    yamlPayload << "number: " << n << '\n' << "divisors:";
    for (long long value : result) {
      yamlPayload << "\n  - " << value;
    }

    printStructuredSuccess(std::cout, outputFormat, "divisors",
                           jsonPayload.str(), xmlPayload.str(),
                           yamlPayload.str());
  }
  return 0;
}

int runConvert(const std::string &fromBaseStr, const std::string &toBaseStr,
               const std::string &valueStr, OutputFormat outputFormat) {
  long long fromBaseValue = 0;
  long long toBaseValue = 0;
  std::string parseError;
  if (!resolveIntegerArgument(fromBaseStr, fromBaseValue, parseError)) {
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << parseError << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "convert", parseError);
    }
    return 1;
  }
  if (!resolveIntegerArgument(toBaseStr, toBaseValue, parseError)) {
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << parseError << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "convert", parseError);
    }
    return 1;
  }
  if (fromBaseValue < std::numeric_limits<int>::min() ||
      fromBaseValue > std::numeric_limits<int>::max() ||
      toBaseValue < std::numeric_limits<int>::min() ||
      toBaseValue > std::numeric_limits<int>::max()) {
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: base is out of supported range." << RESET
                << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "convert",
                           "base is out of supported range.");
    }
    return 1;
  }
  int fromBase = static_cast<int>(fromBaseValue);
  int toBase = static_cast<int>(toBaseValue);
  if ((fromBase != 2 && fromBase != 10 && fromBase != 16) ||
      (toBase != 2 && toBase != 10 && toBase != 16)) {
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: bases must be 2, 10, or 16." << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "convert",
                           "bases must be 2, 10, or 16.");
    }
    return 2;
  }

  std::string resolvedValue;
  if (!resolveIntegerStringArgument(valueStr, resolvedValue, parseError)) {
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << parseError << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "convert", parseError);
    }
    return 1;
  }

  try {
    long long decimalValue = parseInteger(resolvedValue, fromBase);
    std::string converted = formatInteger(decimalValue, toBase);
    if (outputFormat == OutputFormat::Text) {
      std::cout << GREEN << "Result: " << RESET << converted << '\n';
    } else {
      std::ostringstream jsonPayload;
      jsonPayload << "\"fromBase\":" << fromBase << ",\"toBase\":" << toBase;
      jsonPayload << ",\"input\":\"" << jsonEscape(resolvedValue)
                  << "\",\"result\":\"" << jsonEscape(converted) << "\"";

      std::ostringstream xmlPayload;
      xmlPayload << "<fromBase>" << fromBase << "</fromBase><toBase>" << toBase
                 << "</toBase>";
      xmlPayload << "<input>" << xmlEscape(resolvedValue) << "</input><result>"
                 << xmlEscape(converted) << "</result>";

      std::ostringstream yamlPayload;
      yamlPayload << "fromBase: " << fromBase << '\n'
                  << "toBase: " << toBase << '\n'
                  << "input: " << yamlEscape(resolvedValue) << '\n'
                  << "result: " << yamlEscape(converted);

      printStructuredSuccess(std::cout, outputFormat, "convert",
                             jsonPayload.str(), xmlPayload.str(),
                             yamlPayload.str());
    }
    return 0;
  } catch (const std::exception &ex) {
    std::string message =
        std::string("unable to perform conversion: ") + ex.what();
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << message << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "convert", message);
    }
    return 1;
  }
}

int runUnitConvert(const std::string &categoryToken,
                   const std::string &fromToken,
                   const std::string &toToken, const std::string &valueStr,
                   OutputFormat outputFormat) {
  double value = 0.0;
  std::string parseError;
  if (!resolveDoubleArgument(valueStr, value, parseError)) {
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << parseError << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "unit-convert",
                           parseError);
    }
    return 1;
  }

  std::string categoryNormalized = normalizeLookupToken(categoryToken);
  if (categoryNormalized == "temperature" || categoryNormalized == "temp") {
    const TemperatureUnit *from = findTemperatureUnit(fromToken);
    const TemperatureUnit *to = findTemperatureUnit(toToken);
    if (!from || !to) {
      std::string message =
          "unknown temperature unit. Use C, F, or K (or full names).";
      if (outputFormat == OutputFormat::Text) {
        std::cerr << RED << "Error: " << message << RESET << '\n';
      } else {
        printStructuredError(std::cerr, outputFormat, "unit-convert", message);
      }
      return 2;
    }
    if (from == to) {
      std::string message = "source and target units are identical.";
      if (outputFormat == OutputFormat::Text) {
        std::cerr << YELLOW << message << RESET << '\n';
      } else {
        printStructuredError(std::cerr, outputFormat, "unit-convert", message);
      }
      return 1;
    }
    double result = convertTemperature(value, *from, *to);
    if (outputFormat == OutputFormat::Text) {
      std::cout << GREEN << "Result: " << RESET << value << ' ' << from->symbol
                << " = " << result << ' ' << to->symbol << '\n';
    } else {
      std::ostringstream jsonPayload;
      jsonPayload << "\"category\":\"temperature\",\"from\":\""
                  << jsonEscape(from->symbol) << "\",\"to\":\""
                  << jsonEscape(to->symbol) << "\",\"value\":" << value
                  << ",\"result\":" << result;
      std::ostringstream xmlPayload;
      xmlPayload << "<category>temperature</category><from>"
                 << xmlEscape(from->symbol) << "</from><to>"
                 << xmlEscape(to->symbol) << "</to><value>" << value
                 << "</value><result>" << result << "</result>";
      std::ostringstream yamlPayload;
      yamlPayload << "category: temperature\nfrom: " << yamlEscape(from->symbol)
                  << "\nto: " << yamlEscape(to->symbol)
                  << "\nvalue: " << value << "\nresult: " << result;
      printStructuredSuccess(std::cout, outputFormat, "unit-convert",
                             jsonPayload.str(), xmlPayload.str(),
                             yamlPayload.str());
    }
    return 0;
  }

  const LinearCategory *category = findLinearCategory(categoryToken);
  if (!category) {
    std::string message =
        "unknown category. Use length, mass, volume, or temperature.";
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << message << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "unit-convert", message);
    }
    return 2;
  }

  const LinearUnit *from = findLinearUnit(*category, fromToken);
  const LinearUnit *to = findLinearUnit(*category, toToken);
  if (!from || !to) {
    std::string message = "unknown unit for category " + category->name + ".";
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << message << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "unit-convert", message);
    }
    return 2;
  }
  if (from == to) {
    std::string message = "source and target units are identical.";
    if (outputFormat == OutputFormat::Text) {
      std::cerr << YELLOW << message << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "unit-convert", message);
    }
    return 1;
  }

  double result = convertLinearValue(value, *from, *to);
  if (outputFormat == OutputFormat::Text) {
    std::cout << GREEN << "Result: " << RESET << value << ' ' << from->symbol
              << " = " << result << ' ' << to->symbol << '\n';
  } else {
    std::ostringstream jsonPayload;
    jsonPayload << "\"category\":\"" << jsonEscape(category->name)
                << "\",\"from\":\"" << jsonEscape(from->symbol)
                << "\",\"to\":\"" << jsonEscape(to->symbol)
                << "\",\"value\":" << value << ",\"result\":" << result;
    std::ostringstream xmlPayload;
    xmlPayload << "<category>" << xmlEscape(category->name)
               << "</category><from>" << xmlEscape(from->symbol)
               << "</from><to>" << xmlEscape(to->symbol)
               << "</to><value>" << value << "</value><result>" << result
               << "</result>";
    std::ostringstream yamlPayload;
    yamlPayload << "category: " << yamlEscape(category->name)
                << "\nfrom: " << yamlEscape(from->symbol)
                << "\nto: " << yamlEscape(to->symbol)
                << "\nvalue: " << value << "\nresult: " << result;
    printStructuredSuccess(std::cout, outputFormat, "unit-convert",
                           jsonPayload.str(), xmlPayload.str(),
                           yamlPayload.str());
  }
  return 0;
}

int runPrimeFactorization(const std::string &input, OutputFormat outputFormat) {
  long long value = 0;
  std::string parseError;
  if (!resolveIntegerArgument(input, value, parseError)) {
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << parseError << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "prime-factorization",
                           parseError);
    }
    return 1;
  }

  if (value == 0 || value == 1 || value == -1) {
    std::string noFactorsMessage =
        std::to_string(value) + " has no prime factors.";
    if (outputFormat == OutputFormat::Text) {
      std::cout << YELLOW << value << " has no prime factors." << RESET << '\n';
    } else {
      std::ostringstream jsonPayload;
      jsonPayload << "\"value\":" << value << ",\"message\":\""
                  << jsonEscape(noFactorsMessage) << "\"";
      std::ostringstream xmlPayload;
      xmlPayload << "<value>" << value << "</value><message>"
                 << xmlEscape(noFactorsMessage) << "</message>";
      std::ostringstream yamlPayload;
      yamlPayload << "value: " << value << '\n'
                  << "message: " << yamlEscape(noFactorsMessage);
      printStructuredSuccess(std::cout, outputFormat, "prime-factorization",
                             jsonPayload.str(), xmlPayload.str(),
                             yamlPayload.str());
    }
    return 0;
  }

  long long absValue = value < 0 ? -value : value;
  try {
    auto factors = calculatePrimeFactors(absValue);
    std::vector<std::string> parts;
    if (value < 0) {
      parts.push_back("-1");
    }
    for (const auto &factor : factors) {
      std::ostringstream part;
      part << factor.first;
      if (factor.second > 1) {
        part << '^' << factor.second;
      }
      parts.push_back(part.str());
    }
    if (outputFormat == OutputFormat::Text) {
      std::cout << GREEN << "Prime factorization: " << RESET;
      for (std::size_t idx = 0; idx < parts.size(); ++idx) {
        if (idx > 0) {
          std::cout << " * ";
        }
        std::cout << parts[idx];
      }
      std::cout << '\n';
    } else {
      std::ostringstream jsonPayload;
      jsonPayload << "\"value\":" << value << ",\"parts\":[";
      for (std::size_t idx = 0; idx < parts.size(); ++idx) {
        if (idx > 0) {
          jsonPayload << ',';
        }
        jsonPayload << "\"" << jsonEscape(parts[idx]) << "\"";
      }
      jsonPayload << ']';

      std::ostringstream xmlPayload;
      xmlPayload << "<value>" << value << "</value><parts>";
      for (const auto &part : parts) {
        xmlPayload << "<part>" << xmlEscape(part) << "</part>";
      }
      xmlPayload << "</parts>";

      std::ostringstream yamlPayload;
      yamlPayload << "value: " << value << '\n' << "parts:";
      for (const auto &part : parts) {
        yamlPayload << "\n  - " << yamlEscape(part);
      }

      printStructuredSuccess(std::cout, outputFormat, "prime-factorization",
                             jsonPayload.str(), xmlPayload.str(),
                             yamlPayload.str());
    }
  } catch (const std::exception &ex) {
    if (outputFormat == OutputFormat::Text) {
      std::cout << RED << "Error: " << RESET << ex.what() << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "prime-factorization",
                           ex.what());
    }
    return 1;
  }
  return 0;
}

int runSolveLinear(const std::string &aStr, const std::string &bStr,
                   OutputFormat outputFormat) {
  double a = 0.0;
  double b = 0.0;
  std::string parseError;
  if (!resolveDoubleArgument(aStr, a, parseError) ||
      !resolveDoubleArgument(bStr, b, parseError)) {
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << parseError << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "solve-linear",
                           parseError);
    }
    return 1;
  }

  if (isApproximatelyZero(a)) {
    if (isApproximatelyZero(b)) {
      if (outputFormat == OutputFormat::Text) {
        std::cout << CYAN << "Every real number is a solution." << RESET
                  << '\n';
      } else {
        printStructuredSuccess(std::cout, outputFormat, "solve-linear",
                               "\"status\":\"all-real\"",
                               "<status>all-real</status>",
                               "status: all-real");
      }
      return 0;
    }
    if (outputFormat == OutputFormat::Text) {
      std::cout << RED << "No solution exists for this equation." << RESET
                << '\n';
    } else {
      printStructuredSuccess(std::cout, outputFormat, "solve-linear",
                             "\"status\":\"none\"",
                             "<status>none</status>", "status: none");
    }
    return 0;
  }

  double result = -b / a;
  if (outputFormat == OutputFormat::Text) {
    std::cout << GREEN << "Solution: x = " << RESET << result << '\n';
  } else {
    std::ostringstream jsonPayload;
    jsonPayload << "\"status\":\"single\",\"x\":" << result;
    std::ostringstream xmlPayload;
    xmlPayload << "<status>single</status><x>" << result << "</x>";
    std::ostringstream yamlPayload;
    yamlPayload << "status: single\nx: " << result;
    printStructuredSuccess(std::cout, outputFormat, "solve-linear",
                           jsonPayload.str(), xmlPayload.str(),
                           yamlPayload.str());
  }
  return 0;
}

int runSolveQuadratic(const std::string &aStr, const std::string &bStr,
                      const std::string &cStr, OutputFormat outputFormat) {
  double a = 0.0;
  double b = 0.0;
  double c = 0.0;
  std::string parseError;
  if (!resolveDoubleArgument(aStr, a, parseError) ||
      !resolveDoubleArgument(bStr, b, parseError) ||
      !resolveDoubleArgument(cStr, c, parseError)) {
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << parseError << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "solve-quadratic",
                           parseError);
    }
    return 1;
  }

  constexpr double epsilon = 1e-9;
  if (isApproximatelyZero(a, epsilon)) {
    if (outputFormat == OutputFormat::Text) {
      std::cout << YELLOW
                << "Coefficient 'a' is zero; falling back to a linear equation."
                << RESET << '\n';
    }
    if (isApproximatelyZero(b)) {
      if (isApproximatelyZero(c)) {
        if (outputFormat == OutputFormat::Text) {
          std::cout << CYAN << "Every real number is a solution." << RESET
                    << '\n';
        } else {
          printStructuredSuccess(std::cout, outputFormat, "solve-quadratic",
                                 "\"fallback\":true,\"status\":\"all-real\"",
                                 "<fallback>true</fallback><status>all-real</status>",
                                 "fallback: true\nstatus: all-real");
        }
        return 0;
      }
      if (outputFormat == OutputFormat::Text) {
        std::cout << RED << "No solution exists for this equation." << RESET
                  << '\n';
      } else {
        printStructuredSuccess(std::cout, outputFormat, "solve-quadratic",
                               "\"fallback\":true,\"status\":\"none\"",
                               "<fallback>true</fallback><status>none</status>",
                               "fallback: true\nstatus: none");
      }
      return 0;
    }
    double result = -c / b;
    if (outputFormat == OutputFormat::Text) {
      std::cout << GREEN << "Solution: x = " << RESET << result << '\n';
    } else {
      std::ostringstream jsonPayload;
      jsonPayload << "\"fallback\":true,\"status\":\"single\",\"x\":" << result;
      std::ostringstream xmlPayload;
      xmlPayload << "<fallback>true</fallback><status>single</status><x>"
                 << result << "</x>";
      std::ostringstream yamlPayload;
      yamlPayload << "fallback: true\nstatus: single\nx: " << result;
      printStructuredSuccess(std::cout, outputFormat, "solve-quadratic",
                             jsonPayload.str(), xmlPayload.str(),
                             yamlPayload.str());
    }
    return 0;
  }

  double discriminant = b * b - 4.0 * a * c;
  if (discriminant > epsilon) {
    double sqrtDisc = std::sqrt(discriminant);
    double denom = 2.0 * a;
    double x1 = (-b + sqrtDisc) / denom;
    double x2 = (-b - sqrtDisc) / denom;
    if (outputFormat == OutputFormat::Text) {
      std::cout << CYAN << "Two real solutions:" << RESET << '\n';
      std::cout << GREEN << " x1 = " << RESET << x1 << '\n';
      std::cout << GREEN << " x2 = " << RESET << x2 << '\n';
    } else {
      std::ostringstream jsonPayload;
      jsonPayload << "\"status\":\"two-real\",\"solutions\":["
                  << x1 << ',' << x2 << "]";
      std::ostringstream xmlPayload;
      xmlPayload << "<status>two-real</status><solutions>"
                 << "<value>" << x1 << "</value><value>" << x2
                 << "</value></solutions>";
      std::ostringstream yamlPayload;
      yamlPayload << "status: two-real\nsolutions:\n  - " << x1
                  << "\n  - " << x2;
      printStructuredSuccess(std::cout, outputFormat, "solve-quadratic",
                             jsonPayload.str(), xmlPayload.str(),
                             yamlPayload.str());
    }
    return 0;
  }

  if (isApproximatelyZero(discriminant, epsilon)) {
    double root = -b / (2.0 * a);
    if (outputFormat == OutputFormat::Text) {
      std::cout << GREEN << "One real solution (double root): x = " << RESET
                << root << '\n';
    } else {
      std::ostringstream jsonPayload;
      jsonPayload << "\"status\":\"double-root\",\"x\":" << root;
      std::ostringstream xmlPayload;
      xmlPayload << "<status>double-root</status><x>" << root << "</x>";
      std::ostringstream yamlPayload;
      yamlPayload << "status: double-root\nx: " << root;
      printStructuredSuccess(std::cout, outputFormat, "solve-quadratic",
                             jsonPayload.str(), xmlPayload.str(),
                             yamlPayload.str());
    }
    return 0;
  }

  std::complex<double> sqrtDisc =
      std::sqrt(std::complex<double>(discriminant, 0.0));
  std::complex<double> denom(2.0 * a, 0.0);
  std::complex<double> x1 = (-b + sqrtDisc) / denom;
  std::complex<double> x2 = (-b - sqrtDisc) / denom;
  if (outputFormat == OutputFormat::Text) {
    auto printComplex = [](const std::complex<double> &value) {
      double realPart = value.real();
      double imagPart = value.imag();
      std::cout << GREEN << realPart << RESET;
      if (!isApproximatelyZero(imagPart)) {
        if (imagPart >= 0) {
          std::cout << " + " << GREEN << imagPart << RESET << "i";
        } else {
          std::cout << " - " << GREEN << std::abs(imagPart) << RESET << "i";
        }
      }
    };

    std::cout << "Two complex solutions:\n x1 = ";
    printComplex(x1);
    std::cout << "\n x2 = ";
    printComplex(x2);
    std::cout << '\n';
  } else {
    std::ostringstream jsonPayload;
    jsonPayload << "\"status\":\"complex\",\"solutions\":["
                << "{\"real\":" << x1.real() << ",\"imag\":" << x1.imag()
                << "},{\"real\":" << x2.real() << ",\"imag\":" << x2.imag()
                << "}]";
    std::ostringstream xmlPayload;
    xmlPayload << "<status>complex</status><solutions>"
               << "<solution><real>" << x1.real() << "</real><imag>"
               << x1.imag() << "</imag></solution>"
               << "<solution><real>" << x2.real() << "</real><imag>"
               << x2.imag() << "</imag></solution></solutions>";
    std::ostringstream yamlPayload;
    yamlPayload << "status: complex\nsolutions:\n  - real: " << x1.real()
                << "\n    imag: " << x1.imag()
                << "\n  - real: " << x2.real() << "\n    imag: " << x2.imag();
    printStructuredSuccess(std::cout, outputFormat, "solve-quadratic",
                           jsonPayload.str(), xmlPayload.str(),
                           yamlPayload.str());
  }
  return 0;
}

int runMatrixAdd(const std::string &lhsStr, const std::string &rhsStr,
                 OutputFormat outputFormat) {
  Matrix lhs;
  Matrix rhs;
  std::string error;
  if (!parseMatrix(lhsStr, lhs, error) || !parseMatrix(rhsStr, rhs, error)) {
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << error << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "matrix-add", error);
    }
    return 1;
  }
  if (lhs.size() != rhs.size() ||
      (!lhs.empty() && !rhs.empty() && lhs[0].size() != rhs[0].size())) {
    std::string message = "matrices must have the same dimensions";
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << message << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "matrix-add", message);
    }
    return 2;
  }
  Matrix result = addMatrices(lhs, rhs);
  if (outputFormat == OutputFormat::Text) {
    printMatrix(result);
  } else {
    printStructuredSuccess(std::cout, outputFormat, "matrix-add",
                           "\"result\":" + jsonMatrix(result),
                           "<result>" + xmlMatrix(result) + "</result>",
                           "result: " + yamlMatrix(result));
  }
  return 0;
}

int runMatrixSubtract(const std::string &lhsStr, const std::string &rhsStr,
                      OutputFormat outputFormat) {
  Matrix lhs;
  Matrix rhs;
  std::string error;
  if (!parseMatrix(lhsStr, lhs, error) || !parseMatrix(rhsStr, rhs, error)) {
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << error << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "matrix-subtract", error);
    }
    return 1;
  }
  if (lhs.size() != rhs.size() ||
      (!lhs.empty() && !rhs.empty() && lhs[0].size() != rhs[0].size())) {
    std::string message = "matrices must have the same dimensions";
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << message << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "matrix-subtract", message);
    }
    return 2;
  }
  Matrix result = subtractMatrices(lhs, rhs);
  if (outputFormat == OutputFormat::Text) {
    printMatrix(result);
  } else {
    printStructuredSuccess(std::cout, outputFormat, "matrix-subtract",
                           "\"result\":" + jsonMatrix(result),
                           "<result>" + xmlMatrix(result) + "</result>",
                           "result: " + yamlMatrix(result));
  }
  return 0;
}

int runMatrixMultiply(const std::string &lhsStr, const std::string &rhsStr,
                      OutputFormat outputFormat) {
  Matrix lhs;
  Matrix rhs;
  std::string error;
  if (!parseMatrix(lhsStr, lhs, error) || !parseMatrix(rhsStr, rhs, error)) {
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << error << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "matrix-multiply", error);
    }
    return 1;
  }
  if (lhs.empty() || rhs.empty() || lhs[0].size() != rhs.size()) {
    std::string message =
        "matrix A columns must match matrix B rows for multiplication";
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << message << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "matrix-multiply", message);
    }
    return 2;
  }
  Matrix result = multiplyMatrices(lhs, rhs);
  if (outputFormat == OutputFormat::Text) {
    printMatrix(result);
  } else {
    printStructuredSuccess(std::cout, outputFormat, "matrix-multiply",
                           "\"result\":" + jsonMatrix(result),
                           "<result>" + xmlMatrix(result) + "</result>",
                           "result: " + yamlMatrix(result));
  }
  return 0;
}

int runStatistics(const std::vector<std::string> &tokens,
                  OutputFormat outputFormat) {
  if (tokens.empty()) {
    std::string message = "missing values after --stats";
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << message << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "stats", message);
    }
    return 2;
  }
  std::string error;
  std::vector<double> values;
  if (!parseValueList(joinTokens(tokens), values, error)) {
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << error << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "stats", error);
    }
    return 1;
  }

  try {
    StatisticsSummary summary = calculateStatistics(values);
    double percentile25 = calculatePercentile(values, 25.0);
    double percentile75 = calculatePercentile(values, 75.0);
    if (outputFormat == OutputFormat::Text) {
      std::streamsize previousPrecision = std::cout.precision();
      std::ios::fmtflags previousFlags = std::cout.flags();
      std::cout << std::fixed << std::setprecision(4);

      std::cout << GREEN << "Summary:" << RESET << '\n';
      std::cout << "  Count: " << summary.count << '\n';
      std::cout << "  Sum: " << summary.sum << '\n';
      std::cout << "  Mean: " << summary.mean << '\n';
      std::cout << "  Median: " << summary.median << '\n';
      std::cout << "  Minimum: " << summary.minimum << '\n';
      std::cout << "  Maximum: " << summary.maximum << '\n';
      std::cout << "  Range: " << summary.range << '\n';
      std::cout << "  Variance: " << summary.variance << '\n';
      std::cout << "  Standard deviation: " << summary.standardDeviation
                << '\n';
      std::cout << "  25th percentile (Q1): " << percentile25 << '\n';
      std::cout << "  75th percentile (Q3): " << percentile75 << '\n';
      if (summary.modes.empty()) {
        std::cout << "  Mode: No repeating values detected.\n";
      } else {
        std::cout << "  Mode(s): ";
        for (std::size_t idx = 0; idx < summary.modes.size(); ++idx) {
          if (idx > 0) {
            std::cout << ", ";
          }
          std::cout << summary.modes[idx];
        }
        std::cout << '\n';
      }

      std::cout.precision(previousPrecision);
      std::cout.flags(previousFlags);
    } else {
      std::ostringstream jsonPayload;
      jsonPayload << "\"count\":" << summary.count << ",\"sum\":"
                  << summary.sum << ",\"mean\":" << summary.mean
                  << ",\"median\":" << summary.median
                  << ",\"minimum\":" << summary.minimum
                  << ",\"maximum\":" << summary.maximum
                  << ",\"range\":" << summary.range
                  << ",\"variance\":" << summary.variance
                  << ",\"standardDeviation\":" << summary.standardDeviation
                  << ",\"percentile25\":" << percentile25
                  << ",\"percentile75\":" << percentile75 << ",\"modes\":[";
      for (std::size_t idx = 0; idx < summary.modes.size(); ++idx) {
        if (idx > 0) {
          jsonPayload << ',';
        }
        jsonPayload << summary.modes[idx];
      }
      jsonPayload << ']';

      std::ostringstream xmlPayload;
      xmlPayload << "<count>" << summary.count << "</count><sum>"
                 << summary.sum << "</sum><mean>" << summary.mean
                 << "</mean><median>" << summary.median << "</median><minimum>"
                 << summary.minimum << "</minimum><maximum>" << summary.maximum
                 << "</maximum><range>" << summary.range
                 << "</range><variance>" << summary.variance
                 << "</variance><standardDeviation>"
                 << summary.standardDeviation << "</standardDeviation>"
                 << "<percentile25>" << percentile25 << "</percentile25>"
                 << "<percentile75>" << percentile75 << "</percentile75>"
                 << "<modes>";
      for (double mode : summary.modes) {
        xmlPayload << "<value>" << mode << "</value>";
      }
      xmlPayload << "</modes>";

      std::ostringstream yamlPayload;
      yamlPayload << "count: " << summary.count << "\nsum: " << summary.sum
                  << "\nmean: " << summary.mean << "\nmedian: "
                  << summary.median << "\nminimum: " << summary.minimum
                  << "\nmaximum: " << summary.maximum << "\nrange: "
                  << summary.range << "\nvariance: " << summary.variance
                  << "\nstandardDeviation: " << summary.standardDeviation
                  << "\npercentile25: " << percentile25
                  << "\npercentile75: " << percentile75 << "\nmodes:";
      if (summary.modes.empty()) {
        yamlPayload << " []";
      } else {
        for (double mode : summary.modes) {
          yamlPayload << "\n  - " << mode;
        }
      }

      printStructuredSuccess(std::cout, outputFormat, "stats",
                             jsonPayload.str(), xmlPayload.str(),
                             yamlPayload.str());
    }
  } catch (const std::exception &ex) {
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Failed to calculate statistics: " << RESET
                << ex.what() << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "stats", ex.what());
    }
    return 1;
  }
  return 0;
}

int runGraphValues(const std::vector<std::string> &tokens,
                   OutputFormat outputFormat) {
  if (tokens.size() < 2) {
    std::string message =
        "usage: --graph-values <output.png> <values...> [--height N]";
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << message << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "graph-values", message);
    }
    return 2;
  }

  std::string outputPath = ensurePngExtension(tokens.front());
  std::size_t height = 10;
  std::vector<std::string> valueTokens;
  std::string error;
  for (std::size_t idx = 1; idx < tokens.size(); ++idx) {
    const std::string &token = tokens[idx];
    if (token.rfind("--height=", 0) == 0) {
      if (!parseGraphHeight(token.substr(9), height, error)) {
        if (outputFormat == OutputFormat::Text) {
          std::cerr << RED << "Error: " << error << RESET << '\n';
        } else {
          printStructuredError(std::cerr, outputFormat, "graph-values", error);
        }
        return 1;
      }
      continue;
    }
    if (token == "--height") {
      if (idx + 1 >= tokens.size()) {
        std::string message = "missing value after --height";
        if (outputFormat == OutputFormat::Text) {
          std::cerr << RED << "Error: " << message << RESET << '\n';
        } else {
          printStructuredError(std::cerr, outputFormat, "graph-values", message);
        }
        return 1;
      }
      if (!parseGraphHeight(tokens[idx + 1], height, error)) {
        if (outputFormat == OutputFormat::Text) {
          std::cerr << RED << "Error: " << error << RESET << '\n';
        } else {
          printStructuredError(std::cerr, outputFormat, "graph-values", error);
        }
        return 1;
      }
      ++idx;
      continue;
    }
    valueTokens.push_back(token);
  }

  std::vector<double> values;
  if (!parseValueList(joinTokens(valueTokens), values, error)) {
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << error << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "graph-values", error);
    }
    return 1;
  }

  std::vector<std::string> preview = buildAsciiGraph(values, height);
  std::string pngError;
  if (!generateGraphPng(values, outputPath, pngError)) {
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Failed to create PNG: " << RESET << pngError
                << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "graph-values", pngError);
    }
    return 1;
  }

  if (outputFormat == OutputFormat::Text) {
    std::cout << GREEN << "ASCII preview:" << RESET << '\n';
    for (const auto &line : preview) {
      std::cout << line << '\n';
    }
    std::cout << GREEN << "Saved graph to '" << outputPath << "'." << RESET
              << '\n';
  } else {
    std::ostringstream jsonPayload;
    jsonPayload << "\"output\":\"" << jsonEscape(outputPath)
                << "\",\"preview\":[";
    for (std::size_t idx = 0; idx < preview.size(); ++idx) {
      if (idx > 0) {
        jsonPayload << ',';
      }
      jsonPayload << "\"" << jsonEscape(preview[idx]) << "\"";
    }
    jsonPayload << ']';

    std::ostringstream xmlPayload;
    xmlPayload << "<output>" << xmlEscape(outputPath)
               << "</output><preview>";
    for (const auto &line : preview) {
      xmlPayload << "<line>" << xmlEscape(line) << "</line>";
    }
    xmlPayload << "</preview>";

    std::ostringstream yamlPayload;
    yamlPayload << "output: " << yamlEscape(outputPath) << "\npreview:";
    if (preview.empty()) {
      yamlPayload << " []";
    } else {
      for (const auto &line : preview) {
        yamlPayload << "\n  - " << yamlEscape(line);
      }
    }

    printStructuredSuccess(std::cout, outputFormat, "graph-values",
                           jsonPayload.str(), xmlPayload.str(),
                           yamlPayload.str());
  }
  return 0;
}

int runGraphCsv(const std::vector<std::string> &tokens,
                OutputFormat outputFormat) {
  if (tokens.size() < 3) {
    std::string message =
        "usage: --graph-csv <output.png> <csv-path> <column> [--height N] "
        "[--no-headers]";
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << message << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "graph-csv", message);
    }
    return 2;
  }

  std::string outputPath = ensurePngExtension(tokens[0]);
  std::string csvPath = tokens[1];
  std::string columnSpec = tokens[2];
  bool hasHeaders = true;
  std::size_t height = 10;
  std::string error;

  for (std::size_t idx = 3; idx < tokens.size(); ++idx) {
    const std::string &token = tokens[idx];
    if (token == "--no-headers") {
      hasHeaders = false;
      continue;
    }
    if (token == "--headers") {
      hasHeaders = true;
      continue;
    }
    if (token.rfind("--height=", 0) == 0) {
      if (!parseGraphHeight(token.substr(9), height, error)) {
        if (outputFormat == OutputFormat::Text) {
          std::cerr << RED << "Error: " << error << RESET << '\n';
        } else {
          printStructuredError(std::cerr, outputFormat, "graph-csv", error);
        }
        return 1;
      }
      continue;
    }
    if (token == "--height") {
      if (idx + 1 >= tokens.size()) {
        std::string message = "missing value after --height";
        if (outputFormat == OutputFormat::Text) {
          std::cerr << RED << "Error: " << message << RESET << '\n';
        } else {
          printStructuredError(std::cerr, outputFormat, "graph-csv", message);
        }
        return 1;
      }
      if (!parseGraphHeight(tokens[idx + 1], height, error)) {
        if (outputFormat == OutputFormat::Text) {
          std::cerr << RED << "Error: " << error << RESET << '\n';
        } else {
          printStructuredError(std::cerr, outputFormat, "graph-csv", error);
        }
        return 1;
      }
      ++idx;
      continue;
    }
  }

  std::ifstream input(csvPath);
  if (!input) {
    std::string message = "unable to open '" + csvPath + "'.";
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << message << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "graph-csv", message);
    }
    return 1;
  }

  std::vector<std::vector<std::string>> rows;
  std::string line;
  while (std::getline(input, line)) {
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    rows.push_back(parseCsvLine(line));
  }
  if (rows.empty()) {
    std::string message = "CSV file is empty.";
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << message << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "graph-csv", message);
    }
    return 1;
  }

  std::vector<std::string> headers;
  std::size_t dataStartIndex = 0;
  if (hasHeaders) {
    headers = rows.front();
    dataStartIndex = 1;
    if (headers.empty()) {
      std::string message = "header row does not contain any columns.";
      if (outputFormat == OutputFormat::Text) {
        std::cerr << RED << "Error: " << message << RESET << '\n';
      } else {
        printStructuredError(std::cerr, outputFormat, "graph-csv", message);
      }
      return 1;
    }
    for (std::size_t idx = 0; idx < headers.size(); ++idx) {
      if (trimCopy(headers[idx]).empty()) {
        std::ostringstream fallback;
        fallback << "Column " << (idx + 1);
        headers[idx] = fallback.str();
      }
    }
  } else {
    if (rows.front().empty()) {
      std::string message = "unable to determine column count from first row.";
      if (outputFormat == OutputFormat::Text) {
        std::cerr << RED << "Error: " << message << RESET << '\n';
      } else {
        printStructuredError(std::cerr, outputFormat, "graph-csv", message);
      }
      return 1;
    }
    headers.resize(rows.front().size());
    for (std::size_t idx = 0; idx < rows.front().size(); ++idx) {
      std::ostringstream fallback;
      fallback << "Column " << (idx + 1);
      headers[idx] = fallback.str();
    }
  }

  std::size_t columnIndex = 0;
  bool matched = false;
  try {
    long long idx = std::stoll(columnSpec);
    if (idx >= 1 && static_cast<std::size_t>(idx) <= headers.size()) {
      columnIndex = static_cast<std::size_t>(idx - 1);
      matched = true;
    }
  } catch (const std::exception &) {
  }
  if (!matched) {
    std::string lowered = toLowerCopy(columnSpec);
    for (std::size_t idx = 0; idx < headers.size(); ++idx) {
      if (toLowerCopy(headers[idx]) == lowered) {
        columnIndex = idx;
        matched = true;
        break;
      }
    }
  }
  if (!matched) {
    std::string message = "unable to match column selection.";
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << message << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "graph-csv", message);
    }
    return 1;
  }

  std::vector<double> values;
  std::size_t skippedMissing = 0;
  std::size_t skippedInvalid = 0;
  for (std::size_t idx = dataStartIndex; idx < rows.size(); ++idx) {
    if (columnIndex >= rows[idx].size()) {
      ++skippedMissing;
      continue;
    }
    std::string cell = trimCopy(rows[idx][columnIndex]);
    if (cell.empty()) {
      ++skippedMissing;
      continue;
    }
    try {
      std::size_t processed = 0;
      double parsed = std::stod(cell, &processed);
      if (processed != cell.size()) {
        ++skippedInvalid;
        continue;
      }
      values.push_back(parsed);
    } catch (const std::exception &) {
      ++skippedInvalid;
    }
  }
  if (values.empty()) {
    std::string message = "no numeric values found in selected column.";
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << message << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "graph-csv", message);
    }
    return 1;
  }

  std::vector<std::string> preview = buildAsciiGraph(values, height);
  std::string pngError;
  if (!generateGraphPng(values, outputPath, pngError)) {
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Failed to create PNG: " << RESET << pngError
                << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "graph-csv", pngError);
    }
    return 1;
  }

  if (outputFormat == OutputFormat::Text) {
    if (skippedMissing > 0 || skippedInvalid > 0) {
      std::cout << YELLOW << "Skipped " << skippedMissing
                << " row(s) with missing values and " << skippedInvalid
                << " row(s) with invalid numbers." << RESET << '\n';
    }
    std::cout << GREEN << "ASCII preview:" << RESET << '\n';
    for (const auto &line : preview) {
      std::cout << line << '\n';
    }
    std::cout << GREEN << "Saved graph to '" << outputPath << "'." << RESET
              << '\n';
  } else {
    std::ostringstream jsonPayload;
    jsonPayload << "\"output\":\"" << jsonEscape(outputPath)
                << "\",\"preview\":[";
    for (std::size_t idx = 0; idx < preview.size(); ++idx) {
      if (idx > 0) {
        jsonPayload << ',';
      }
      jsonPayload << "\"" << jsonEscape(preview[idx]) << "\"";
    }
    jsonPayload << "],\"skippedMissing\":" << skippedMissing
                << ",\"skippedInvalid\":" << skippedInvalid;

    std::ostringstream xmlPayload;
    xmlPayload << "<output>" << xmlEscape(outputPath)
               << "</output><preview>";
    for (const auto &line : preview) {
      xmlPayload << "<line>" << xmlEscape(line) << "</line>";
    }
    xmlPayload << "</preview><skippedMissing>" << skippedMissing
               << "</skippedMissing><skippedInvalid>" << skippedInvalid
               << "</skippedInvalid>";

    std::ostringstream yamlPayload;
    yamlPayload << "output: " << yamlEscape(outputPath) << "\npreview:";
    if (preview.empty()) {
      yamlPayload << " []";
    } else {
      for (const auto &line : preview) {
        yamlPayload << "\n  - " << yamlEscape(line);
      }
    }
    yamlPayload << "\nskippedMissing: " << skippedMissing
                << "\nskippedInvalid: " << skippedInvalid;

    printStructuredSuccess(std::cout, outputFormat, "graph-csv",
                           jsonPayload.str(), xmlPayload.str(),
                           yamlPayload.str());
  }
  return 0;
}

int runSetVariable(const std::string &name, const std::string &valueStr,
                   OutputFormat outputFormat) {
  if (!VariableStore::isValidName(name)) {
    std::string message = "invalid variable name: " + name;
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << message << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "set-variable", message);
    }
    return 1;
  }

  double value = 0.0;
  std::string parseError;
  if (!resolveDoubleArgument(valueStr, value, parseError)) {
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << parseError << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "set-variable",
                           parseError);
    }
    return 1;
  }

  globalVariableStore().set(name, value);
  if (!globalVariableStore().save()) {
    std::string message = "unable to persist variable store.";
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << message << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "set-variable", message);
    }
    return 1;
  }

  if (outputFormat == OutputFormat::Text) {
    std::cout << GREEN << "Saved '" << name << "' = " << value << RESET << '\n';
  } else {
    std::ostringstream jsonPayload;
    jsonPayload << "\"variable\":\"" << jsonEscape(name) << "\",\"value\":"
                << value;
    std::ostringstream xmlPayload;
    xmlPayload << "<variable>" << xmlEscape(name) << "</variable><value>"
               << value << "</value>";
    std::ostringstream yamlPayload;
    yamlPayload << "variable: " << yamlEscape(name) << "\nvalue: " << value;
    printStructuredSuccess(std::cout, outputFormat, "set-variable",
                           jsonPayload.str(), xmlPayload.str(),
                           yamlPayload.str());
  }
  return 0;
}

int runUnsetVariable(const std::string &name, OutputFormat outputFormat) {
  if (!VariableStore::isValidName(name)) {
    std::string message = "invalid variable name: " + name;
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << message << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "unset-variable", message);
    }
    return 1;
  }

  if (!globalVariableStore().remove(name)) {
    std::string message = "variable '" + name + "' does not exist.";
    if (outputFormat == OutputFormat::Text) {
      std::cout << YELLOW << message << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "unset-variable", message);
    }
    return 1;
  }
  if (!globalVariableStore().save()) {
    std::string message = "unable to persist variable store.";
    if (outputFormat == OutputFormat::Text) {
      std::cerr << RED << "Error: " << message << RESET << '\n';
    } else {
      printStructuredError(std::cerr, outputFormat, "unset-variable", message);
    }
    return 1;
  }

  if (outputFormat == OutputFormat::Text) {
    std::cout << GREEN << "Removed variable '" << name << "'." << RESET << '\n';
  } else {
    std::ostringstream jsonPayload;
    jsonPayload << "\"variable\":\"" << jsonEscape(name) << "\"";
    std::ostringstream xmlPayload;
    xmlPayload << "<variable>" << xmlEscape(name) << "</variable>";
    std::ostringstream yamlPayload;
    yamlPayload << "variable: " << yamlEscape(name);
    printStructuredSuccess(std::cout, outputFormat, "unset-variable",
                           jsonPayload.str(), xmlPayload.str(),
                           yamlPayload.str());
  }
  return 0;
}

int runReportBug(OutputFormat outputFormat) {
  const std::string url =
      "https://github.com/Benedek553/cli-calculator/issues/"
      "new?template=bug_report.yml";
  openUrl(url);
  if (outputFormat == OutputFormat::Text) {
    std::cout << BLUE << "Opened a browser to report a bug." << RESET << '\n';
  } else {
    std::ostringstream jsonPayload;
    jsonPayload << "\"url\":\"" << jsonEscape(url) << "\"";
    std::ostringstream xmlPayload;
    xmlPayload << "<url>" << xmlEscape(url) << "</url>";
    std::ostringstream yamlPayload;
    yamlPayload << "url: " << yamlEscape(url);
    printStructuredSuccess(std::cout, outputFormat, "report-bug",
                           jsonPayload.str(), xmlPayload.str(),
                           yamlPayload.str());
  }
  return 0;
}

int runHelp(OutputFormat outputFormat) {
  const std::string helpText =
      "CLI Calculator Help\n"
      "Usage: calculator [options]\n"
      "Options:\n"
      "  -e, --eval <expression>       Evaluate the given mathematical "
      "expression.\n"
      "  --repl                        Start the interactive REPL with "
      "arrow-key history + CLI flag support.\n"
      "  -sqrt, --square-root <value>  Calculate the square root of the given "
      "value.\n"
      "  -d, --divisors <number>       Calculate and display the divisors of "
      "the given number.\n"
      "  -c, --convert <from> <to> <value>  Convert value from one base to "
      "another (bases: 2, 10, 16).\n"
      "  --unit-convert <category> <from> <to> <value>  Convert measurement "
      "units (length, mass, volume, temperature).\n"
      "  -pf, --prime-factorization <value>  Factorize a number into primes.\n"
      "  --solve-linear <a> <b>        Solve a linear equation a*x + b = 0.\n"
      "  --solve-quadratic <a> <b> <c> Solve a quadratic equation "
      "a*x^2 + b*x + c = 0.\n"
      "  --matrix-add <A> <B>          Add matrices (rows ';', columns ',' or "
      "spaces).\n"
      "  --matrix-subtract <A> <B>     Subtract matrices (rows ';', columns "
      "',' or spaces).\n"
      "  --matrix-multiply <A> <B>     Multiply matrices (rows ';', columns "
      "',' or spaces).\n"
      "  --stats, --statistics <values...>  Compute summary statistics for a "
      "list.\n"
      "  --graph-values <output.png> <values...> [--height N]  Render values "
      "to a PNG graph.\n"
      "  --graph-csv <output.png> <csv> <column> [--height N] [--no-headers]  "
      "Render CSV column to a PNG graph.\n"
      "  -v, --version                 Print the application version.\n"
      "  --variables, --list-variables List persisted variables.\n"
      "  --set-variable <name> <value> Set or update a stored variable.\n"
      "  --unset-variable <name>       Remove a stored variable.\n"
      "  --report-bug                  Open the issue template in a browser.\n"
      "  -b, --batch <file.txt>        Execute CLI flag commands listed in a "
      "text file (supports @set/@input/@include/@if/@endif/@unset helpers).\n"
      "  --output <format>            Print CLI flag results as json, xml, or "
      "yaml.\n"
      "  -nc, --no-color               Disable colored output.\n"
      "  -h, --help                    Display this help message.\n";

  if (outputFormat == OutputFormat::Text) {
    std::cout << BOLD << BLUE << "CLI Calculator Help" << RESET << '\n';
    std::cout << "Usage: calculator [options]\n";
    std::cout << "Options:\n";
    std::cout << "  -e, --eval <expression>       Evaluate the given "
                 "mathematical expression.\n";
    std::cout << "  --repl                        Start the interactive REPL "
                 "with arrow-key history + CLI flag support.\n";
    std::cout << "  -sqrt, --square-root <value>  Calculate the square root of "
                 "the given value.\n";
    std::cout << "  -d, --divisors <number>       Calculate and display the "
                 "divisors of the given number.\n";
    std::cout << "  -c, --convert <from> <to> <value>  Convert value from one "
                 "base to another (bases: 2, 10, 16).\n";
    std::cout << "  --unit-convert <category> <from> <to> <value>  Convert "
                 "measurement units (length, mass, volume, temperature).\n";
    std::cout << "  -pf, --prime-factorization <value>  Factorize a number "
                 "into primes.\n";
    std::cout << "  --solve-linear <a> <b>        Solve a linear equation "
                 "a*x + b = 0.\n";
    std::cout << "  --solve-quadratic <a> <b> <c> Solve a quadratic equation "
                 "a*x^2 + b*x + c = 0.\n";
    std::cout << "  --matrix-add <A> <B>          Add matrices (rows ';', "
                 "columns ',' or spaces).\n";
    std::cout << "  --matrix-subtract <A> <B>     Subtract matrices (rows ';', "
                 "columns ',' or spaces).\n";
    std::cout << "  --matrix-multiply <A> <B>     Multiply matrices (rows ';', "
                 "columns ',' or spaces).\n";
    std::cout << "  --stats, --statistics <values...>  Compute summary "
                 "statistics for a list.\n";
    std::cout << "  --graph-values <output.png> <values...> [--height N]  "
                 "Render values to a PNG graph.\n";
    std::cout << "  --graph-csv <output.png> <csv> <column> [--height N] "
                 "[--no-headers]  Render CSV column to a PNG graph.\n";
    std::cout
        << "  -v, --version                 Print the application version.\n";
    std::cout << "  --variables, --list-variables List persisted variables.\n";
    std::cout << "  --set-variable <name> <value> Set or update a stored "
                 "variable.\n";
    std::cout << "  --unset-variable <name>       Remove a stored variable.\n";
    std::cout << "  --report-bug                  Open the issue template in a "
                 "browser.\n";
    std::cout << "  -b, --batch <file.txt>        Execute CLI flag commands "
                 "listed in a text file (supports "
                 "@set/@input/@include/@if/@endif/@unset helpers).\n";
    std::cout << "  --output <format>            Print CLI flag results as "
                 "json, xml, or yaml.\n";
    std::cout << "  -nc, --no-color               Disable colored output.\n";
    std::cout << "  -h, --help                    Display this help message.\n";
  } else {
    std::string jsonPayload = "\"text\":\"" + jsonEscape(helpText) + "\"";
    std::string xmlPayload = "<text>" + xmlEscape(helpText) + "</text>";
    std::string yamlPayload = std::string("text: ") + yamlEscape(helpText);
    printStructuredSuccess(std::cout, outputFormat, "help", jsonPayload,
                           xmlPayload, yamlPayload);
  }
  return 0;
}

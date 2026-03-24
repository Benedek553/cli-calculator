#include "menu_handlers.hpp"

#include "ansi_colors.hpp"
#include "cli_repl.hpp"
#include "core/graph_png.hpp"
#include "core/matrix.hpp"
#include "core/parse_utils.hpp"
#include "core/unit_conversion.hpp"
#include "core/variables.hpp"
#include "divisors.hpp"
#include "equations.hpp"
#include "expression.hpp"
#include "input.hpp"
#include "numeral_conversion.hpp"
#include "prime_factors.hpp"
#include "statistics.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
std::string trim(const std::string &text) {
  auto notSpace = [](unsigned char ch) { return !std::isspace(ch); };
  auto begin = std::find_if(text.begin(), text.end(), notSpace);
  if (begin == text.end()) {
    return "";
  }
  auto rbegin = std::find_if(text.rbegin(), text.rend(), notSpace);
  return {begin, rbegin.base()};
}

std::string toLowerCopy(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char ch) {
                   return static_cast<char>(std::tolower(ch));
                 });
  return value;
}

int chooseBase(const std::string &label) {
  while (true) {
    std::cout << label << '\n';
    std::cout << YELLOW << " 1) " << RESET << CYAN << "Decimal (10)" << RESET
              << '\n';
    std::cout << YELLOW << " 2) " << RESET << CYAN << "Binary (2)" << RESET
              << '\n';
    std::cout << YELLOW << " 3) " << RESET << CYAN << "Hexadecimal (16)"
              << RESET << '\n';
    std::cout << YELLOW << " 0) " << RESET << CYAN << "Back" << RESET << '\n';

    int choice = readMenuChoice(0, 3);
    switch (choice) {
    case 1:
      return 10;
    case 2:
      return 2;
    case 3:
      return 16;
    case 0:
      return 0;
    default:
      break;
    }
  }
}

void printVariables(const std::map<std::string, double> &vars) {
  if (vars.empty()) {
    std::cout << YELLOW << "No variables defined yet." << RESET << '\n';
    return;
  }
  std::cout << CYAN << "Stored variables:" << RESET << '\n';
  int precision = std::numeric_limits<double>::digits10 + 1;
  for (const auto &entry : vars) {
    std::ostringstream stream;
    stream << std::setprecision(precision) << entry.second;
    std::cout << "  " << entry.first << " = " << stream.str() << '\n';
  }
}

void handleSetVariable(VariableStore &store) {
  std::string name = trim(readLine("Variable name to create or update: "));
  if (!VariableStore::isValidName(name)) {
    std::cout << RED
              << "Names must start with a letter and may contain letters, "
                 "digits, or underscores."
              << RESET << '\n';
    return;
  }
  std::string prompt = "Enter value for '" + name + "': ";
  double value = readDouble(prompt);
  store.set(name, value);
  if (!store.save()) {
    std::cout << RED << "Unable to persist variables to vars.toml." << RESET
              << '\n';
    return;
  }
  std::cout << GREEN << "Saved '" << name << "' = " << value << RESET << '\n';
}

void handleDeleteVariable(VariableStore &store) {
  std::string name = trim(readLine("Variable name to delete: "));
  if (!VariableStore::isValidName(name)) {
    std::cout << RED
              << "Names must start with a letter and may contain letters, "
                 "digits, or underscores."
              << RESET << '\n';
    return;
  }
  if (!store.remove(name)) {
    std::cout << YELLOW << "Variable '" << name << "' does not exist." << RESET
              << '\n';
    return;
  }
  if (!store.save()) {
    std::cout << RED << "Unable to persist variables to vars.toml." << RESET
              << '\n';
    return;
  }
  std::cout << GREEN << "Removed variable '" << name << "'." << RESET << '\n';
}

std::size_t readPositiveDimension(const std::string &prompt) {
  while (true) {
    long long value = readInteger(prompt);
    if (value <= 0) {
      std::cout << YELLOW << "Dimensions must be positive integers." << RESET
                << '\n';
      continue;
    }
    return static_cast<std::size_t>(value);
  }
}

Matrix readMatrixValues(const std::string &name, std::size_t rows,
                        std::size_t columns) {
  Matrix matrix(rows, std::vector<double>(columns, 0.0));
  std::cout << CYAN << "Enter values for matrix " << name << ":" << RESET
            << '\n';
  for (std::size_t row = 0; row < rows; ++row) {
    for (std::size_t column = 0; column < columns; ++column) {
      std::ostringstream prompt;
      prompt << "  " << name << '[' << (row + 1) << ',' << (column + 1)
             << "] = ";
      matrix[row][column] = readDouble(prompt.str());
    }
  }
  return matrix;
}

void printMatrixResult(const Matrix &matrix) {
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

bool promptManualGraphValues(std::vector<double> &values) {
  while (true) {
    std::string input =
        readLine("Enter numbers separated by spaces (type 'back' to cancel): ");
    std::string lowered = toLowerCopy(trim(input));
    if (lowered == "back") {
      return false;
    }

    std::string error;
    if (!parseNumberList(input, values, error)) {
      std::cout << RED << "Error: " << RESET << error << '\n';
      continue;
    }
    return true;
  }
}

std::size_t promptAsciiGraphHeight() {
  while (true) {
    std::string heightInput = trim(readLine(
        "ASCII preview height 2-20 (press Enter for default 10): "));
    if (heightInput.empty()) {
      return 10;
    }
    try {
      long long parsed = std::stoll(heightInput);
      if (parsed < 2 || parsed > 20) {
        std::cout << YELLOW << "Please choose a value between 2 and 20."
                  << RESET << '\n';
        continue;
      }
      return static_cast<std::size_t>(parsed);
    } catch (const std::exception &) {
      std::cout << RED
                << "Invalid height. Please provide a whole number between 2 "
                   "and 20 or leave it empty."
                << RESET << '\n';
    }
  }
}

bool promptCsvGraphValues(std::vector<double> &values) {
  while (true) {
    std::string path =
        trim(readLine("Path to CSV file (type 'back' to cancel): "));
    if (path.empty()) {
      std::cout << YELLOW << "Please provide a path to a CSV file." << RESET
                << '\n';
      continue;
    }
    if (toLowerCopy(path) == "back") {
      return false;
    }

    std::ifstream input(path);
    if (!input) {
      std::cout << RED << "Unable to open '" << path << "'." << RESET << '\n';
      continue;
    }

    bool hasHeaders = true;
    while (true) {
      std::string response = trim(
          readLine("Does the first row contain headers? (y/n, default y): "));
      if (response.empty()) {
        break;
      }
      char answer =
          static_cast<char>(std::tolower(static_cast<unsigned char>(response[0])));
      if (answer == 'y') {
        hasHeaders = true;
        break;
      }
      if (answer == 'n') {
        hasHeaders = false;
        break;
      }
      std::cout << YELLOW << "Please answer with 'y' or 'n'." << RESET << '\n';
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
      std::cout << YELLOW << "The file appears to be empty." << RESET << '\n';
      continue;
    }

    std::vector<std::string> headers;
    std::size_t dataStartIndex = 0;
    if (hasHeaders) {
      headers = rows.front();
      dataStartIndex = 1;
      if (headers.empty()) {
        std::cout << YELLOW << "The header row does not contain any columns."
                  << RESET << '\n';
        continue;
      }
      for (std::size_t idx = 0; idx < headers.size(); ++idx) {
        if (trim(headers[idx]).empty()) {
          std::ostringstream fallback;
          fallback << "Column " << (idx + 1);
          headers[idx] = fallback.str();
        }
      }
    } else {
      if (rows.front().empty()) {
        std::cout << YELLOW
                  << "Unable to determine the number of columns from the first "
                     "row."
                  << RESET << '\n';
        continue;
      }
      headers.resize(rows.front().size());
      for (std::size_t idx = 0; idx < rows.front().size(); ++idx) {
        std::ostringstream fallback;
        fallback << "Column " << (idx + 1);
        headers[idx] = fallback.str();
      }
    }

    std::cout << GREEN << "Available columns:" << RESET << '\n';
    for (std::size_t idx = 0; idx < headers.size(); ++idx) {
      std::cout << "  " << (idx + 1) << ") " << headers[idx] << '\n';
    }

    std::size_t columnIndex = 0;
    while (true) {
      std::string selection = trim(readLine(
          "Select column by number or name (type 'back' to cancel): "));
      if (selection.empty()) {
        std::cout << YELLOW << "Please provide a column selection." << RESET
                  << '\n';
        continue;
      }
      std::string lowered = toLowerCopy(selection);
      if (lowered == "back") {
        return false;
      }

      bool matched = false;
      try {
        long long idx = std::stoll(selection);
        if (idx >= 1 && static_cast<std::size_t>(idx) <= headers.size()) {
          columnIndex = static_cast<std::size_t>(idx - 1);
          matched = true;
        }
      } catch (const std::exception &) {
      }
      if (!matched) {
        for (std::size_t idx = 0; idx < headers.size(); ++idx) {
          if (toLowerCopy(headers[idx]) == lowered) {
            columnIndex = idx;
            matched = true;
            break;
          }
        }
      }
      if (matched) {
        break;
      }
      std::cout << RED << "Unable to match that selection to a column." << RESET
                << '\n';
    }

    values.clear();
    std::size_t skippedMissing = 0;
    std::size_t skippedInvalid = 0;
    for (std::size_t idx = dataStartIndex; idx < rows.size(); ++idx) {
      if (columnIndex >= rows[idx].size()) {
        ++skippedMissing;
        continue;
      }
      std::string cell = trim(rows[idx][columnIndex]);
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
      std::cout << RED
                << "No numeric rows were found in the selected column."
                << RESET << '\n';
      continue;
    }

    if (skippedMissing > 0 || skippedInvalid > 0) {
      std::cout << YELLOW << "Skipped " << skippedMissing
                << " row(s) with missing values and " << skippedInvalid
                << " row(s) with invalid numbers." << RESET << '\n';
    }
    return true;
  }
}

std::string ensurePngExtension(std::string path) {
  std::string lowered = toLowerCopy(path);
  if (lowered.size() < 4 ||
      lowered.substr(lowered.size() - 4) != ".png") {
    path += ".png";
  }
  return path;
}

template <typename Unit>
int chooseUnit(const std::vector<Unit> &units, const std::string &prompt) {
  while (true) {
    std::cout << prompt << '\n';
    for (std::size_t idx = 0; idx < units.size(); ++idx) {
      std::cout << YELLOW << ' ' << (idx + 1) << ") " << RESET << CYAN
                << units[idx].name << " (" << units[idx].symbol << ")" << RESET
                << '\n';
    }
    std::cout << YELLOW << " 0) " << RESET << CYAN << "Back" << RESET << '\n';
    int selection = readMenuChoice(0, static_cast<int>(units.size()));
    if (selection == 0) {
      return -1;
    }
    return selection - 1;
  }
}

void runNumeralSystemConversion() {
  while (true) {
    std::cout << '\n'
              << UNDERLINE << BLUE << "--- Numeral System Conversion ---"
              << RESET << '\n';
    int fromBase =
        chooseBase(BOLD + YELLOW + std::string("Source base:") + RESET);
    if (fromBase == 0) {
      return;
    }
    int toBase = chooseBase(BOLD + BLUE + std::string("Target base:") + RESET);
    if (toBase == 0) {
      return;
    }
    if (fromBase == toBase) {
      std::cout << RED
                << "Source and target base are identical; nothing to convert."
                << RESET << '\n';
      continue;
    }

    std::cout << BOLD << BLUE << "Enter the integer to convert: " << RESET;
    std::string rawValue;
    std::cin >> rawValue;

    try {
      long long decimalValue = parseInteger(rawValue, fromBase);
      std::string converted = formatInteger(decimalValue, toBase);
      std::cout << GREEN << "Result: " << RESET << converted << '\n';
    } catch (const std::exception &ex) {
      std::cout << RED << "Error: " << RESET << ex.what() << '\n';
    }

    if (!askToContinue("Would you like to convert another number? (y/n): ")) {
      return;
    }
  }
}

void runLinearCategoryConversion(const LinearCategory &category) {
  while (true) {
    std::cout << '\n'
              << UNDERLINE << GREEN << "--- " << category.name
              << " Conversion ---" << RESET << '\n';
    int fromUnit = chooseUnit(
        category.units, BOLD + YELLOW + std::string("Source unit:") + RESET);
    if (fromUnit < 0) {
      return;
    }
    int toUnit = chooseUnit(category.units,
                            BOLD + BLUE + std::string("Target unit:") + RESET);
    if (toUnit < 0) {
      return;
    }
    if (fromUnit == toUnit) {
      std::cout << YELLOW
                << "Source and target units are identical; please select "
                   "different units."
                << RESET << '\n';
      continue;
    }

    const auto &from = category.units[fromUnit];
    const auto &to = category.units[toUnit];
    std::string prompt = "Enter value in " + from.name + ": ";
    double value = readDouble(prompt);
    double result = convertLinearValue(value, from, to);
    std::cout << GREEN << "Result: " << RESET << value << ' ' << from.symbol
              << " = " << result << ' ' << to.symbol << '\n';

    if (!askToContinue("Convert another value in this category? (y/n): ")) {
      return;
    }
  }
}

void runTemperatureConversion() {
  while (true) {
    std::cout << '\n'
              << UNDERLINE << GREEN << "--- Temperature Conversion ---" << RESET
              << '\n';
    int fromUnit =
        chooseUnit(temperatureUnits(),
                   BOLD + YELLOW + std::string("Source unit:") + RESET);
    if (fromUnit < 0) {
      return;
    }
    int toUnit =
        chooseUnit(temperatureUnits(),
                   BOLD + BLUE + std::string("Target unit:") + RESET);
    if (toUnit < 0) {
      return;
    }
    if (fromUnit == toUnit) {
      std::cout << YELLOW
                << "Source and target units are identical; please select "
                   "different units."
                << RESET << '\n';
      continue;
    }

    const auto &from = temperatureUnits()[fromUnit];
    const auto &to = temperatureUnits()[toUnit];
    std::string prompt = "Enter temperature in " + from.name + ": ";
    double value = readDouble(prompt);
    double result = convertTemperature(value, from, to);
    std::cout << GREEN << "Result: " << RESET << value << ' ' << from.symbol
              << " = " << result << ' ' << to.symbol << '\n';

    if (!askToContinue("Convert another temperature? (y/n): ")) {
      return;
    }
  }
}

void runUnitConversionMenu() {
  while (true) {
    std::cout << '\n'
              << UNDERLINE << GREEN << "--- Unit Conversion ---" << RESET
              << '\n';
    for (std::size_t idx = 0; idx < linearCategories().size(); ++idx) {
      std::cout << YELLOW << ' ' << (idx + 1) << ") " << RESET << CYAN
                << linearCategories()[idx].name << RESET << '\n';
    }
    std::cout << YELLOW << ' ' << (linearCategories().size() + 1) << ") "
              << RESET << CYAN << "Temperature" << RESET << '\n';
    std::cout << YELLOW << " 0) " << RESET << CYAN << "Back" << RESET << '\n';

    int maxOption = static_cast<int>(linearCategories().size() + 1);
    int choice = readMenuChoice(0, maxOption);
    if (choice == 0) {
      return;
    }
    if (choice <= static_cast<int>(linearCategories().size())) {
      runLinearCategoryConversion(linearCategories()[choice - 1]);
    } else {
      runTemperatureConversion();
    }
  }
}
} // namespace

void handleArithmetic() {
  std::cout << '\n'
            << UNDERLINE << MAGENTA << "--- Expression Evaluator ---" << RESET
            << '\n';
  std::cout << YELLOW << " 1) " << RESET << CYAN << "Standard (double)"
            << RESET << '\n';
  std::cout << YELLOW << " 2) " << RESET << CYAN << "Bigint (integers only)"
            << RESET << '\n';
  std::cout << YELLOW << " 3) " << RESET << CYAN
            << "Bigdouble (high-precision decimals)" << RESET << '\n';
  std::cout << YELLOW << " 0) " << RESET << CYAN << "Back" << RESET << '\n';

  int modeChoice = readMenuChoice(0, 3);
  if (modeChoice == 0) {
    return;
  }
  bool useBigInt = modeChoice == 2;
  bool useBigDouble = modeChoice == 3;

  while (true) {
    std::string prompt;
    if (useBigInt) {
      prompt = "Enter an integer expression (type 'back' to return): ";
    } else if (useBigDouble) {
      prompt = "Enter a decimal expression (type 'back' to return): ";
    } else {
      prompt = "Enter an expression (type 'back' to return): ";
    }
    std::string expression = readLine(prompt);
    std::string lowered = trim(expression);
    std::transform(
        lowered.begin(), lowered.end(), lowered.begin(),
        [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    if (lowered == "back") {
      return;
    }
    try {
      if (useBigInt) {
        std::string result = evaluateExpressionBigInt(
            expression, globalVariableStore().variables());
        std::cout << GREEN << "Result: " << RESET << result << '\n';
      } else if (useBigDouble) {
        std::string result = evaluateExpressionBigDouble(
            expression, globalVariableStore().variables());
        std::cout << GREEN << "Result: " << RESET << result << '\n';
      } else {
        double result =
            evaluateExpression(expression, globalVariableStore().variables());
        std::cout << GREEN << "Result: " << RESET << result << '\n';
      }
    } catch (const std::exception &ex) {
      std::cout << RED << "Error: " << RESET << ex.what() << '\n';
    }

    if (!askToContinue(
            "Would you like to evaluate another expression? (y/n): ")) {
      return;
    }
  }
}

void handleConversions() {
  while (true) {
    std::cout << '\n'
              << UNDERLINE << BLUE << "--- Conversion Tools ---" << RESET
              << '\n';
    std::cout << YELLOW << " 1) " << RESET << CYAN
              << "Numeral system conversion" << RESET << '\n';
    std::cout << YELLOW << " 2) " << RESET << CYAN
              << "Measurement unit conversion" << RESET << '\n';
    std::cout << YELLOW << " 0) " << RESET << CYAN << "Back" << RESET << '\n';

    int choice = readMenuChoice(0, 2);
    switch (choice) {
    case 1:
      runNumeralSystemConversion();
      break;
    case 2:
      runUnitConversionMenu();
      break;
    case 0:
      return;
    default:
      break;
    }
  }
}

void handleDivisors() {
  while (true) {
    std::cout << '\n'
              << UNDERLINE << MAGENTA << "--- Divisor Finder ---" << RESET
              << '\n';
    long long value = readInteger("Enter an integer (0 allowed): ");
    if (value == 0) {
      std::cout << RED << "Zero has infinitely many divisors." << RESET << '\n';
      continue;
    } else {
      try {
        std::vector<long long> divisors = calculateDivisors(value);
        std::cout << GREEN << "Divisors: " << RESET;
        for (std::size_t idx = 0; idx < divisors.size(); ++idx) {
          if (idx > 0) {
            std::cout << ", ";
          }
          std::cout << divisors[idx];
        }
        std::cout << '\n';
      } catch (const std::exception &ex) {
        std::cout << RED << "Error: " << RESET << ex.what() << '\n';
      }
    }

    if (!askToContinue("Would you like to check another number? (y/n): ")) {
      return;
    }
  }
}

void handlePrimeFactorization() {
  while (true) {
    std::cout << '\n'
              << UNDERLINE << MAGENTA << "--- Prime Factorization ---" << RESET
              << '\n';
    long long value = readInteger("Enter an integer: ");
    if (value == 0) {
      std::cout << RED
                << "Zero does not have a well-defined prime factorization."
                << RESET << '\n';
    } else {
      long long absValue = value < 0 ? -value : value;
      if (absValue == 1) {
        std::cout << YELLOW << value << " has no prime factors." << RESET
                  << '\n';
      } else {
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
          std::cout << GREEN << "Prime factorization: " << RESET;
          for (std::size_t idx = 0; idx < parts.size(); ++idx) {
            if (idx > 0) {
              std::cout << " * ";
            }
            std::cout << parts[idx];
          }
          std::cout << '\n';
        } catch (const std::exception &ex) {
          std::cout << RED << "Error: " << RESET << ex.what() << '\n';
        }
      }
    }

    if (!askToContinue("Would you like to factor another number? (y/n): ")) {
      return;
    }
  }
}

void handleEquations() {
  while (true) {
    std::cout << '\n'
              << UNDERLINE << MAGENTA << "--- Equation Solver ---" << RESET
              << '\n';
    std::cout << YELLOW << " 1) " << RESET << CYAN << "Linear (a * x + b = 0)"
              << RESET << '\n';
    std::cout << YELLOW << " 2) " << RESET << CYAN
              << "Quadratic (a * x^2 + b * x + c = 0)" << RESET << '\n';
    std::cout << YELLOW << " 3) " << RESET << CYAN
              << "Cubic (a * x^3 + b * x^2 + c * x + d = 0)" << RESET << '\n';
    std::cout << YELLOW << " 4) " << RESET << CYAN
              << "Linear system 2x2 (a1*x + b1*y = c1, a2*x + b2*y = c2)"
              << RESET << '\n';
    std::cout << YELLOW << " 0) " << RESET << CYAN << "Back" << RESET << '\n';

    int choice = readMenuChoice(0, 4);
    switch (choice) {
    case 0:
      return;
    case 1: {
      double a = readDouble("Enter coefficient a: ");
      double b = readDouble("Enter coefficient b: ");
      solveLinearEquation(a, b);
      break;
    }
    case 2: {
      double a = readDouble("Enter coefficient a: ");
      double b = readDouble("Enter coefficient b: ");
      double c = readDouble("Enter coefficient c: ");
      solveQuadraticEquation(a, b, c);
      break;
    }
    case 3: {
      double a = readDouble("Enter coefficient a: ");
      double b = readDouble("Enter coefficient b: ");
      double c = readDouble("Enter coefficient c: ");
      double d = readDouble("Enter coefficient d: ");
      solveCubicEquation(a, b, c, d);
      break;
    }
    case 4: {
      double a1 = readDouble("Enter coefficient a1: ");
      double b1 = readDouble("Enter coefficient b1: ");
      double c1 = readDouble("Enter coefficient c1: ");
      double a2 = readDouble("Enter coefficient a2: ");
      double b2 = readDouble("Enter coefficient b2: ");
      double c2 = readDouble("Enter coefficient c2: ");
      solveLinearSystem2x2(a1, b1, c1, a2, b2, c2);
      break;
    }
    default:
      break;
    }

    if (!askToContinue("Would you like to solve another equation? (y/n): ")) {
      return;
    }
  }
}

void handleMatrixOperations() {
  while (true) {
    std::cout << '\n'
              << UNDERLINE << BLUE << "--- Matrix Operations ---" << RESET
              << '\n';
    std::cout << YELLOW << " 1) " << RESET << CYAN << "Matrix addition" << RESET
              << '\n';
    std::cout << YELLOW << " 2) " << RESET << CYAN << "Matrix subtraction"
              << RESET << '\n';
    std::cout << YELLOW << " 3) " << RESET << CYAN << "Matrix multiplication"
              << RESET << '\n';
    std::cout << YELLOW << " 0) " << RESET << CYAN << "Back" << RESET << '\n';

    int choice = readMenuChoice(0, 3);
    if (choice == 0) {
      return;
    }

    try {
      Matrix result;
      switch (choice) {
      case 1:
      case 2: {
        std::size_t rows = readPositiveDimension("Enter the number of rows: ");
        std::size_t columns =
            readPositiveDimension("Enter the number of columns: ");
        Matrix first = readMatrixValues("A", rows, columns);
        Matrix second = readMatrixValues("B", rows, columns);
        result = (choice == 1) ? addMatrices(first, second)
                               : subtractMatrices(first, second);
        break;
      }
      case 3: {
        std::size_t rowsA =
            readPositiveDimension("Enter the number of rows for matrix A: ");
        std::size_t shared =
            readPositiveDimension("Enter the number of columns for matrix A: ");
        Matrix first = readMatrixValues("A", rowsA, shared);
        std::size_t columnsB =
            readPositiveDimension("Enter the number of columns for matrix B: ");
        std::cout << YELLOW << "Matrix B automatically uses " << shared
                  << " rows to match matrix A." << RESET << '\n';
        Matrix second = readMatrixValues("B", shared, columnsB);
        result = multiplyMatrices(first, second);
        break;
      }
      default:
        continue;
      }
      printMatrixResult(result);
    } catch (const std::exception &ex) {
      std::cout << RED << "Matrix error: " << RESET << ex.what() << '\n';
    }

    if (!askToContinue(
            "Would you like to perform another matrix operation? (y/n): ")) {
      return;
    }
  }
}

void handleSquareRoot() {
  while (true) {
    std::cout << UNDERLINE << MAGENTA << "\n--- Square Root Calculator ---\n"
              << RESET;
    double value = readDouble("Enter a number to find its square root: ");

    try {
      if (value < 0.0) {
        throw std::domain_error("Square root undefined for negative values.");
      }
      double result = std::sqrt(value);
      std::cout << "Square root of " << value << " = " << result << '\n';
    } catch (const std::exception &ex) {
      std::cout << "Error: " << ex.what() << '\n';
    }

    if (!askToContinue(
            "Would you like to calculate another square root? (y/n): ")) {
      return;
    }
  }
}

void handleVariables() {
  auto &store = globalVariableStore();
  while (true) {
    std::cout << '\n'
              << UNDERLINE << BLUE << "--- Variable Manager ---" << RESET
              << '\n';
    printVariables(store.variables());

    std::cout << YELLOW << " 1) " << RESET << CYAN << "Set or update a variable"
              << RESET << '\n';
    std::cout << YELLOW << " 2) " << RESET << CYAN << "Delete a variable"
              << RESET << '\n';
    std::cout << YELLOW << " 0) " << RESET << CYAN << "Back" << RESET << '\n';

    int choice = readMenuChoice(0, 2);
    switch (choice) {
    case 0:
      return;
    case 1:
      handleSetVariable(store);
      break;
    case 2:
      handleDeleteVariable(store);
      break;
    default:
      break;
    }
  }
}

void handleStatistics() {
  while (true) {
    std::cout << '\n'
              << UNDERLINE << GREEN << "--- Statistical Tools ---" << RESET
              << '\n';
    std::string input =
        readLine("Enter numbers separated by spaces (type 'back' to return): ");
    std::string lowered = trim(input);
    std::transform(
        lowered.begin(), lowered.end(), lowered.begin(),
        [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    if (lowered == "back") {
      return;
    }

    std::vector<double> values;
    std::string error;
    if (!parseNumberList(input, values, error)) {
      std::cout << RED << "Error: " << RESET << error << '\n';
      continue;
    }

    try {
      StatisticsSummary summary = calculateStatistics(values);
      double percentile25 = calculatePercentile(values, 25.0);
      double percentile75 = calculatePercentile(values, 75.0);
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
    } catch (const std::exception &ex) {
      std::cout << RED << "Failed to calculate statistics: " << RESET
                << ex.what() << '\n';
    }

    if (!askToContinue("Analyze another dataset? (y/n): ")) {
      return;
    }
  }
}

void handleGraphUtility() {
  while (true) {
    std::cout << '\n'
              << UNDERLINE << CYAN << "--- Graph Utility ---" << RESET << '\n';
    std::cout << YELLOW << " 1) " << RESET << CYAN << "Enter values manually"
              << RESET << '\n';
    std::cout << YELLOW << " 2) " << RESET << CYAN << "Load values from CSV"
              << RESET << '\n';
    std::cout << YELLOW << " 0) " << RESET << CYAN << "Back" << RESET << '\n';

    int choice = readMenuChoice(0, 2);
    if (choice == 0) {
      return;
    }

    std::vector<double> values;
    bool haveValues = false;
    if (choice == 1) {
      haveValues = promptManualGraphValues(values);
    } else if (choice == 2) {
      haveValues = promptCsvGraphValues(values);
    }
    if (!haveValues) {
      continue;
    }

    std::size_t height = promptAsciiGraphHeight();
    auto graphLines = buildAsciiGraph(values, height);
    std::cout << GREEN << "ASCII preview:" << RESET << '\n';
    for (const auto &line : graphLines) {
      std::cout << line << '\n';
    }

    std::string outputPath =
        trim(readLine("Enter output PNG filename (default graph.png): "));
    if (outputPath.empty()) {
      outputPath = "graph.png";
    }
    outputPath = ensurePngExtension(outputPath);

    std::string error;
    if (generateGraphPng(values, outputPath, error)) {
      std::cout << GREEN << "Saved graph to '" << outputPath << "'." << RESET
                << '\n';
    } else {
      std::cout << RED << "Failed to create PNG: " << RESET << error << '\n';
    }

    if (!askToContinue("Create another graph? (y/n): ")) {
      return;
    }
  }
}

void open_url(const std::string &url) {
  int ret = std::system("command -v snapctl >/dev/null 2>&1");
  if (ret == 0) {
    std::system(("snapctl user-open " + url).c_str());
  } else {
    std::system(("xdg-open " + url).c_str());
  }
}

void runInteractiveCalculator() {
  while (true) {
    std::cout << '\n'
              << UNDERLINE << RED << "=== Main Menu ===" << RESET << '\n';
    std::cout << YELLOW << " 1) " << RESET << CYAN << "Basic operations"
              << RESET << '\n';
    std::cout << YELLOW << " 2) " << RESET << CYAN << "Conversion tools"
              << RESET << '\n';
    std::cout << YELLOW << " 3) " << RESET << CYAN << "Divisor finder" << RESET
              << '\n';
    std::cout << YELLOW << " 4) " << RESET << CYAN << "Equation solver" << RESET
              << '\n';
    std::cout << YELLOW << " 5) " << RESET << CYAN << "Matrix operations"
              << RESET << '\n';
    std::cout << YELLOW << " 6) " << RESET << CYAN << "Square root calculator"
              << RESET << '\n';
    std::cout << YELLOW << " 7) " << RESET << CYAN << "Variable manager"
              << RESET << '\n';
    std::cout << YELLOW << " 8) " << RESET << CYAN << "Prime factorization"
              << RESET << '\n';
    std::cout << YELLOW << " 9) " << RESET << CYAN << "Statistical tools"
              << RESET << '\n';
    std::cout << YELLOW << "10) " << RESET << CYAN << "Graph utility" << RESET
              << '\n';
    std::cout << YELLOW << "11) " << RESET << CYAN << "Launch REPL mode"
              << RESET << '\n';
    std::cout << YELLOW << "12) " << RESET << CYAN << "Report a bug" << RESET
              << '\n';
    std::cout << YELLOW << " 0) " << RESET << CYAN << "Exit" << RESET << '\n';

    int choice = readMenuChoice(0, 12);
    switch (choice) {
    case 1:
      handleArithmetic();
      break;
    case 2:
      handleConversions();
      break;
    case 3:
      handleDivisors();
      break;
    case 4:
      handleEquations();
      break;
    case 5:
      handleMatrixOperations();
      break;
    case 6:
      handleSquareRoot();
      break;
    case 7:
      handleVariables();
      break;
    case 8:
      handlePrimeFactorization();
      break;
    case 9:
      handleStatistics();
      break;
    case 10:
      handleGraphUtility();
      break;
    case 11: {
      int replResult = runRepl(OutputFormat::Text);
      if (replResult != 0) {
        std::cerr << YELLOW << "Exited REPL with status " << replResult << RESET
                  << '\n';
      }
      break;
    }
    case 12:
      std::cout << BLUE << "Opened a browser to report a bug." << RESET << '\n';
      open_url("https://github.com/Benedek553/cli-calculator/issues/"
               "new?template=bug_report.yml");
      break;
    case 0:
      std::cout << BOLD << GREEN << "Goodbye!" << RESET << '\n';
      return;
    default:
      break;
    }
  }
}

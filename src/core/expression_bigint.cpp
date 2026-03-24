#include "expression.hpp"

#include "expression_internal.hpp"
#include "math_utils.hpp"

#include <boost/multiprecision/cpp_int.hpp>
#include <cctype>
#include <cmath>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using boost::multiprecision::cpp_int;

cpp_int parseBigInt(const std::string &text) {
  if (text.empty()) {
    throw std::invalid_argument("Empty integer literal.");
  }
  bool negative = false;
  std::size_t index = 0;
  if (text[0] == '-') {
    negative = true;
    index = 1;
  }
  if (index >= text.size()) {
    throw std::invalid_argument("Invalid integer literal.");
  }
  cpp_int value = 0;
  for (; index < text.size(); ++index) {
    char c = text[index];
    if (!std::isdigit(static_cast<unsigned char>(c))) {
      throw std::invalid_argument("Invalid integer literal.");
    }
    value *= 10;
    value += static_cast<unsigned int>(c - '0');
  }
  return negative ? -value : value;
}

cpp_int factorialOfBigInt(const cpp_int &operand) {
  if (operand < 0) {
    throw std::invalid_argument(
        "Factorial is not defined for negative numbers.");
  }
  if (operand > 10000) {
    throw std::overflow_error(
        "Factorial operand is too large for bigint mode.");
  }
  cpp_int result = 1;
  for (cpp_int i = 2; i <= operand; ++i) {
    result *= i;
  }
  return result;
}

cpp_int resolveBigIntVariable(const std::string &name, double value) {
  double rounded = std::round(value);
  if (!isApproximatelyZero(value - rounded)) {
    throw std::invalid_argument("Variable '" + name +
                                "' must be an integer in bigint mode.");
  }
  if (rounded < static_cast<double>(std::numeric_limits<long long>::min()) ||
      rounded > static_cast<double>(std::numeric_limits<long long>::max())) {
    throw std::overflow_error("Variable '" + name +
                              "' is out of range for bigint mode.");
  }
  return cpp_int(static_cast<long long>(rounded));
}
} // namespace

std::string evaluateExpressionBigInt(
    const std::string &expression,
    const std::map<std::string, double> &variables) {
  using boost::multiprecision::cpp_int;

  std::vector<expression_detail::BigToken> tokens =
      expression_detail::tokenizeExpressionBigInt(expression);
  std::vector<expression_detail::BigToken> rpn =
      expression_detail::toRpnBigInt(tokens);
  std::vector<cpp_int> stack;

  for (const expression_detail::BigToken &token : rpn) {
    switch (token.type) {
    case expression_detail::BigToken::Type::Number:
      stack.push_back(parseBigInt(token.number));
      break;
    case expression_detail::BigToken::Type::Operator:
      if (token.op == '!') {
        if (stack.empty()) {
          throw std::invalid_argument("Factorial operator missing operand.");
        }
        cpp_int value = stack.back();
        stack.back() = factorialOfBigInt(value);
        break;
      }
      if (stack.size() < 2) {
        throw std::invalid_argument(
            "Invalid expression: insufficient operands.");
      }
      {
        cpp_int rhs = stack.back();
        stack.pop_back();
        cpp_int lhs = stack.back();
        stack.pop_back();
        cpp_int result = 0;
        switch (token.op) {
        case '+':
          result = lhs + rhs;
          break;
        case '-':
          result = lhs - rhs;
          break;
        case '*':
          result = lhs * rhs;
          break;
        case '/':
          if (rhs == 0) {
            throw std::runtime_error("Division by zero in expression.");
          }
          if (lhs % rhs != 0) {
            throw std::domain_error(
                "Division results in a non-integer value in bigint mode.");
          }
          result = lhs / rhs;
          break;
        default:
          throw std::invalid_argument("Unknown operator in expression.");
        }
        stack.push_back(result);
      }
      break;
    case expression_detail::BigToken::Type::Variable: {
      auto found = variables.find(token.text);
      if (found == variables.end()) {
        throw std::invalid_argument("Unknown variable: " + token.text);
      }
      stack.push_back(resolveBigIntVariable(token.text, found->second));
      break;
    }
    default:
      break;
    }
  }

  if (stack.size() != 1) {
    throw std::invalid_argument("Invalid expression: leftover operands.");
  }

  return stack.back().convert_to<std::string>();
}

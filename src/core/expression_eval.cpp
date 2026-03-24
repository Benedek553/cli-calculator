#include "expression.hpp"

#include "expression_internal.hpp"
#include "math_utils.hpp"

#include <cmath>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
double factorialOf(double operand) {
  double rounded = std::round(operand);
  if (!isApproximatelyZero(operand - rounded)) {
    throw std::invalid_argument("Factorial is only defined for integers.");
  }

  long long n = static_cast<long long>(rounded);
  if (n < 0) {
    throw std::invalid_argument(
        "Factorial is not defined for negative numbers.");
  }
  if (n > 170) {
    throw std::overflow_error(
        "Factorial result would overflow double precision.");
  }

  long double result = 1.0L;
  for (long long i = 2; i <= n; ++i) {
    result *= static_cast<long double>(i);
  }
  return static_cast<double>(result);
}

double applyFunction(const std::string &functionName, double value) {
  if (functionName == "sin") {
    return std::sin(value);
  }
  if (functionName == "cos") {
    return std::cos(value);
  }
  if (functionName == "log") {
    if (value <= 0.0) {
      throw std::domain_error("Logarithm undefined for non-positive values.");
    }
    return std::log(value);
  }
  if (functionName == "tan") {
    return std::tan(value);
  }
  if (functionName == "sqrt") {
    if (value < 0.0) {
      throw std::domain_error("Square root undefined for negative values.");
    }
    return std::sqrt(value);
  }
  if (functionName == "exp") {
    return std::exp(value);
  }
  if (functionName == "cot") {
    double tanValue = std::tan(value);
    if (isApproximatelyZero(tanValue)) {
      throw std::domain_error("Cotangent undefined for this value.");
    }
    return 1.0 / tanValue;
  }
  if (functionName == "asin") {
    if (value < -1.0 || value > 1.0) {
      throw std::domain_error("Arcsine undefined for this value.");
    }
    return std::asin(value);
  }
  if (functionName == "acos") {
    if (value < -1.0 || value > 1.0) {
      throw std::domain_error("Arccosine undefined for this value.");
    }
    return std::acos(value);
  }
  if (functionName == "atan") {
    return std::atan(value);
  }
  if (functionName == "sinh") {
    return std::sinh(value);
  }
  throw std::invalid_argument("Unknown function: " + functionName);
}
} // namespace

double evaluateExpression(const std::string &expression,
                          const std::map<std::string, double> &variables) {
  std::vector<expression_detail::Token> tokens =
      expression_detail::tokenizeExpression(expression);
  std::vector<expression_detail::Token> rpn = expression_detail::toRpn(tokens);
  std::vector<double> stack;

  for (const expression_detail::Token &token : rpn) {
    switch (token.type) {
    case expression_detail::Token::Type::Number:
      stack.push_back(token.value);
      break;
    case expression_detail::Token::Type::Operator:
      if (token.op == '!') {
        if (stack.empty()) {
          throw std::invalid_argument("Factorial operator missing operand.");
        }
        double value = stack.back();
        stack.back() = factorialOf(value);
        break;
      }
      if (stack.size() < 2) {
        throw std::invalid_argument(
            "Invalid expression: insufficient operands.");
      }
      {
        double rhs = stack.back();
        stack.pop_back();
        double lhs = stack.back();
        stack.pop_back();
        double result = 0.0;
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
          if (rhs == 0.0) {
            throw std::runtime_error("Division by zero in expression.");
          }
          result = lhs / rhs;
          break;
        case '^':
          result = std::pow(lhs, rhs);
          break;
        default:
          throw std::invalid_argument("Unknown operator in expression.");
        }
        stack.push_back(result);
      }
      break;
    case expression_detail::Token::Type::Function:
      if (stack.empty()) {
        throw std::invalid_argument("Function missing operand.");
      }
      {
        double argument = stack.back();
        stack.back() = applyFunction(token.text, argument);
      }
      break;
    case expression_detail::Token::Type::Variable: {
      auto found = variables.find(token.text);
      if (found == variables.end()) {
        throw std::invalid_argument("Unknown variable: " + token.text);
      }
      stack.push_back(found->second);
      break;
    }
    default:
      break;
    }
  }

  if (stack.size() != 1) {
    throw std::invalid_argument("Invalid expression: leftover operands.");
  }

  return stack.back();
}

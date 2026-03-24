#include "expression.hpp"

#include "expression_internal.hpp"

#include <boost/multiprecision/cpp_dec_float.hpp>
#include <cmath>
#include <iomanip>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using BigFloat = boost::multiprecision::cpp_dec_float_50;

BigFloat parseBigDouble(const std::string &text) {
  if (text.empty()) {
    throw std::invalid_argument("Empty decimal literal.");
  }
  std::string normalized = text;
  if (normalized[0] == '.') {
    normalized.insert(normalized.begin(), '0');
  } else if (normalized.size() > 1 && normalized[0] == '-' &&
             normalized[1] == '.') {
    normalized.insert(normalized.begin() + 1, '0');
  }
  try {
    return BigFloat(normalized);
  } catch (const std::exception &) {
    throw std::invalid_argument("Invalid decimal literal.");
  }
}

BigFloat absValue(const BigFloat &value) {
  return value < 0 ? -value : value;
}

bool isApproximatelyZero(const BigFloat &value,
                          const BigFloat &epsilon = BigFloat("1e-40")) {
  return absValue(value) <= epsilon;
}

BigFloat factorialOfBigDouble(const BigFloat &operand) {
  BigFloat rounded = boost::multiprecision::floor(operand + BigFloat("0.5"));
  if (!isApproximatelyZero(operand - rounded)) {
    throw std::invalid_argument("Factorial is only defined for integers.");
  }
  if (rounded < 0) {
    throw std::invalid_argument(
        "Factorial is not defined for negative numbers.");
  }
  if (rounded > BigFloat(10000)) {
    throw std::overflow_error(
        "Factorial operand is too large for bigdouble mode.");
  }
  unsigned int n = rounded.convert_to<unsigned int>();
  BigFloat result = 1;
  for (unsigned int i = 2; i <= n; ++i) {
    result *= i;
  }
  return result;
}

BigFloat applyFunction(const std::string &functionName,
                       const BigFloat &value) {
  using boost::multiprecision::acos;
  using boost::multiprecision::asin;
  using boost::multiprecision::atan;
  using boost::multiprecision::cos;
  using boost::multiprecision::exp;
  using boost::multiprecision::log;
  using boost::multiprecision::sin;
  using boost::multiprecision::sinh;
  using boost::multiprecision::sqrt;
  using boost::multiprecision::tan;

  if (functionName == "sin") {
    return sin(value);
  }
  if (functionName == "cos") {
    return cos(value);
  }
  if (functionName == "log") {
    if (value <= 0) {
      throw std::domain_error("Logarithm undefined for non-positive values.");
    }
    return log(value);
  }
  if (functionName == "tan") {
    return tan(value);
  }
  if (functionName == "sqrt") {
    if (value < 0) {
      throw std::domain_error("Square root undefined for negative values.");
    }
    return sqrt(value);
  }
  if (functionName == "exp") {
    return exp(value);
  }
  if (functionName == "cot") {
    BigFloat tanValue = tan(value);
    if (isApproximatelyZero(tanValue)) {
      throw std::domain_error("Cotangent undefined for this value.");
    }
    return BigFloat(1) / tanValue;
  }
  if (functionName == "asin") {
    if (value < -1 || value > 1) {
      throw std::domain_error("Arcsine undefined for this value.");
    }
    return asin(value);
  }
  if (functionName == "acos") {
    if (value < -1 || value > 1) {
      throw std::domain_error("Arccosine undefined for this value.");
    }
    return acos(value);
  }
  if (functionName == "atan") {
    return atan(value);
  }
  if (functionName == "sinh") {
    return sinh(value);
  }
  throw std::invalid_argument("Unknown function: " + functionName);
}

std::string formatBigFloat(const BigFloat &value) {
  std::ostringstream out;
  out << std::setprecision(std::numeric_limits<BigFloat>::digits10) << value;
  return out.str();
}
} // namespace

std::string evaluateExpressionBigDouble(
    const std::string &expression,
    const std::map<std::string, double> &variables) {
  std::vector<expression_detail::BigFloatToken> tokens =
      expression_detail::tokenizeExpressionBigDouble(expression);
  std::vector<expression_detail::BigFloatToken> rpn =
      expression_detail::toRpnBigDouble(tokens);
  std::vector<BigFloat> stack;

  for (const expression_detail::BigFloatToken &token : rpn) {
    switch (token.type) {
    case expression_detail::BigFloatToken::Type::Number:
      stack.push_back(parseBigDouble(token.number));
      break;
    case expression_detail::BigFloatToken::Type::Operator:
      if (token.op == '!') {
        if (stack.empty()) {
          throw std::invalid_argument("Factorial operator missing operand.");
        }
        BigFloat value = stack.back();
        stack.back() = factorialOfBigDouble(value);
        break;
      }
      if (stack.size() < 2) {
        throw std::invalid_argument(
            "Invalid expression: insufficient operands.");
      }
      {
        BigFloat rhs = stack.back();
        stack.pop_back();
        BigFloat lhs = stack.back();
        stack.pop_back();
        BigFloat result = 0;
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
          if (isApproximatelyZero(rhs)) {
            throw std::runtime_error("Division by zero in expression.");
          }
          result = lhs / rhs;
          break;
        case '^':
          result = boost::multiprecision::pow(lhs, rhs);
          break;
        default:
          throw std::invalid_argument("Unknown operator in expression.");
        }
        stack.push_back(result);
      }
      break;
    case expression_detail::BigFloatToken::Type::Function:
      if (stack.empty()) {
        throw std::invalid_argument("Function missing operand.");
      }
      {
        BigFloat argument = stack.back();
        stack.back() = applyFunction(token.text, argument);
      }
      break;
    case expression_detail::BigFloatToken::Type::Variable: {
      auto found = variables.find(token.text);
      if (found == variables.end()) {
        throw std::invalid_argument("Unknown variable: " + token.text);
      }
      stack.push_back(BigFloat(found->second));
      break;
    }
    default:
      break;
    }
  }

  if (stack.size() != 1) {
    throw std::invalid_argument("Invalid expression: leftover operands.");
  }

  return formatBigFloat(stack.back());
}

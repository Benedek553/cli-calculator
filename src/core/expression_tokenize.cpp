#include "expression_internal.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
bool isOperatorChar(char ch) {
  ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
  return ch == '+' || ch == '-' || ch == '*' || ch == 'x' || ch == ':' ||
         ch == '/' || ch == '^';
}

char normalizeOperator(char ch) {
  ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
  if (ch == 'x') {
    return '*';
  }
  if (ch == ':') {
    return '/';
  }
  return ch;
}

double parseNumberToken(const std::string &expr, std::size_t &index) {
  std::size_t start = index;
  bool hasDigit = false;
  bool hasDot = false;
  while (index < expr.size()) {
    char c = expr[index];
    if (std::isdigit(static_cast<unsigned char>(c))) {
      hasDigit = true;
      ++index;
    } else if (c == '.') {
      if (hasDot) {
        throw std::invalid_argument(
            "Multiple decimal separators found in number.");
      }
      hasDot = true;
      ++index;
    } else {
      break;
    }
  }
  if (!hasDigit) {
    throw std::invalid_argument("Expected a digit in the number.");
  }
  return std::stod(expr.substr(start, index - start));
}

std::string parseDecimalToken(const std::string &expr, std::size_t &index) {
  std::size_t start = index;
  bool hasDigit = false;
  bool hasDot = false;
  while (index < expr.size()) {
    char c = expr[index];
    if (std::isdigit(static_cast<unsigned char>(c))) {
      hasDigit = true;
      ++index;
    } else if (c == '.') {
      if (hasDot) {
        throw std::invalid_argument(
            "Multiple decimal separators found in number.");
      }
      hasDot = true;
      ++index;
    } else {
      break;
    }
  }
  if (!hasDigit) {
    throw std::invalid_argument("Expected a digit in the number.");
  }
  return expr.substr(start, index - start);
}

std::string parseIntegerToken(const std::string &expr, std::size_t &index) {
  std::size_t start = index;
  bool hasDigit = false;
  while (index < expr.size()) {
    char c = expr[index];
    if (std::isdigit(static_cast<unsigned char>(c))) {
      hasDigit = true;
      ++index;
    } else {
      break;
    }
  }
  if (!hasDigit) {
    throw std::invalid_argument("Expected a digit in the integer.");
  }
  return expr.substr(start, index - start);
}
} // namespace

namespace expression_detail {
std::vector<Token> tokenizeExpression(const std::string &expression) {
  std::vector<Token> tokens;
  std::size_t i = 0;
  bool expectValue = true;

  while (i < expression.size()) {
    char c = expression[i];
    if (std::isspace(static_cast<unsigned char>(c))) {
      ++i;
      continue;
    }

    if (expectValue) {
      if (c == '(') {
        tokens.push_back({Token::Type::LeftParen, 0.0, 0, ""});
        ++i;
        continue;
      }

      int sign = 1;
      bool sawUnarySign = false;
      if (c == '+' || c == '-') {
        sawUnarySign = true;
        sign = (c == '-') ? -1 : 1;
        ++i;
        while (i < expression.size() &&
               std::isspace(static_cast<unsigned char>(expression[i]))) {
          ++i;
        }
        if (i >= expression.size()) {
          throw std::invalid_argument(
              "Expression cannot end with a unary operator.");
        }
        c = expression[i];
        if (c == '(') {
          if (sign == -1) {
            tokens.push_back({Token::Type::Number, 0.0, 0, ""});
            tokens.push_back({Token::Type::Operator, 0.0, '-', ""});
          }
          expectValue = true;
          continue;
        }
      } else {
        sign = 1;
      }

      if (std::isdigit(static_cast<unsigned char>(c)) || c == '.') {
        double number = parseNumberToken(expression, i);
        tokens.push_back({Token::Type::Number, sign * number, 0, ""});
        expectValue = false;
        continue;
      }

      if (std::isalpha(static_cast<unsigned char>(c))) {
        std::size_t start = i;
        ++i;
        while (i < expression.size() &&
               (std::isalnum(static_cast<unsigned char>(expression[i])) ||
                expression[i] == '_')) {
          ++i;
        }
        std::string identifier = expression.substr(start, i - start);
        std::string lowered = identifier;
        std::transform(lowered.begin(), lowered.end(), lowered.begin(),
                       [](unsigned char ch) {
                         return static_cast<char>(std::tolower(ch));
                       });

        if (lowered == "sin" || lowered == "cos" || lowered == "log" ||
            lowered == "tan" || lowered == "sqrt" || lowered == "exp" ||
            lowered == "cot" || lowered == "asin" || lowered == "acos" ||
            lowered == "atan" || lowered == "sinh") {
          if (sawUnarySign && sign == -1) {
            tokens.push_back({Token::Type::Number, 0.0, 0, ""});
            tokens.push_back({Token::Type::Operator, 0.0, '-', ""});
          }
          tokens.push_back({Token::Type::Function, 0.0, 0, lowered});

          std::size_t lookahead = i;
          while (
              lookahead < expression.size() &&
              std::isspace(static_cast<unsigned char>(expression[lookahead]))) {
            ++lookahead;
          }
          if (lookahead >= expression.size() || expression[lookahead] != '(') {
            throw std::invalid_argument("Function '" + identifier +
                                        "' must be followed by parentheses.");
          }
          i = lookahead;
          continue;
        }

        if (sawUnarySign && sign == -1) {
          tokens.push_back({Token::Type::Number, 0.0, 0, ""});
          tokens.push_back({Token::Type::Operator, 0.0, '-', ""});
        }
        tokens.push_back({Token::Type::Variable, 0.0, 0, lowered});
        expectValue = false;
        continue;
      }

      throw std::invalid_argument(
          "Expected a number or '(' in the expression.");
    } else {
      if (c == ')') {
        tokens.push_back({Token::Type::RightParen, 0.0, 0, ""});
        ++i;
        continue;
      }

      if (c == '!') {
        tokens.push_back({Token::Type::Operator, 0.0, '!', ""});
        ++i;
        continue;
      }

      if (isOperatorChar(c)) {
        tokens.push_back(
            {Token::Type::Operator, 0.0, normalizeOperator(c), ""});
        ++i;
        expectValue = true;
        continue;
      }

      throw std::invalid_argument(
          "Expected an operator or ')' in the expression.");
    }
  }

  if (expectValue) {
    throw std::invalid_argument(
        "Expression ended unexpectedly. Operand missing.");
  }

  return tokens;
}

std::vector<BigToken> tokenizeExpressionBigInt(const std::string &expression) {
  std::vector<BigToken> tokens;
  std::size_t i = 0;
  bool expectValue = true;

  while (i < expression.size()) {
    char c = expression[i];
    if (std::isspace(static_cast<unsigned char>(c))) {
      ++i;
      continue;
    }

    if (expectValue) {
      if (c == '(') {
        tokens.push_back({BigToken::Type::LeftParen, "", 0, ""});
        ++i;
        continue;
      }

      int sign = 1;
      bool sawUnarySign = false;
      if (c == '+' || c == '-') {
        sawUnarySign = true;
        sign = (c == '-') ? -1 : 1;
        ++i;
        while (i < expression.size() &&
               std::isspace(static_cast<unsigned char>(expression[i]))) {
          ++i;
        }
        if (i >= expression.size()) {
          throw std::invalid_argument(
              "Expression cannot end with a unary operator.");
        }
        c = expression[i];
        if (c == '(') {
          if (sign == -1) {
            tokens.push_back({BigToken::Type::Number, "0", 0, ""});
            tokens.push_back({BigToken::Type::Operator, "", '-', ""});
          }
          expectValue = true;
          continue;
        }
      } else {
        sign = 1;
      }

      if (std::isdigit(static_cast<unsigned char>(c))) {
        std::string number = parseIntegerToken(expression, i);
        if (sign == -1) {
          number.insert(number.begin(), '-');
        }
        tokens.push_back({BigToken::Type::Number, number, 0, ""});
        expectValue = false;
        continue;
      }

      if (c == '.') {
        throw std::invalid_argument(
            "Bigint mode does not support decimal numbers.");
      }

      if (std::isalpha(static_cast<unsigned char>(c))) {
        std::size_t start = i;
        ++i;
        while (i < expression.size() &&
               (std::isalnum(static_cast<unsigned char>(expression[i])) ||
                expression[i] == '_')) {
          ++i;
        }
        std::string identifier = expression.substr(start, i - start);
        std::string lowered = identifier;
        std::transform(lowered.begin(), lowered.end(), lowered.begin(),
                       [](unsigned char ch) {
                         return static_cast<char>(std::tolower(ch));
                       });

        if (lowered == "sin" || lowered == "cos" || lowered == "log" ||
            lowered == "tan" || lowered == "sqrt" || lowered == "exp" ||
            lowered == "cot" || lowered == "asin" || lowered == "acos" ||
            lowered == "atan" || lowered == "sinh") {
          throw std::invalid_argument("Functions are not supported in bigint "
                                      "mode: " +
                                      identifier);
        }

        if (sawUnarySign && sign == -1) {
          tokens.push_back({BigToken::Type::Number, "0", 0, ""});
          tokens.push_back({BigToken::Type::Operator, "", '-', ""});
        }
        tokens.push_back({BigToken::Type::Variable, "", 0, lowered});
        expectValue = false;
        continue;
      }

      throw std::invalid_argument(
          "Expected an integer or '(' in the expression.");
    } else {
      if (c == ')') {
        tokens.push_back({BigToken::Type::RightParen, "", 0, ""});
        ++i;
        continue;
      }

      if (c == '!') {
        tokens.push_back({BigToken::Type::Operator, "", '!', ""});
        ++i;
        continue;
      }

      if (isOperatorChar(c)) {
        tokens.push_back(
            {BigToken::Type::Operator, "", normalizeOperator(c), ""});
        ++i;
        expectValue = true;
        continue;
      }

      throw std::invalid_argument(
          "Expected an operator or ')' in the expression.");
    }
  }

  if (expectValue) {
    throw std::invalid_argument(
        "Expression ended unexpectedly. Operand missing.");
  }

  return tokens;
}

std::vector<BigFloatToken>
tokenizeExpressionBigDouble(const std::string &expression) {
  std::vector<BigFloatToken> tokens;
  std::size_t i = 0;
  bool expectValue = true;

  while (i < expression.size()) {
    char c = expression[i];
    if (std::isspace(static_cast<unsigned char>(c))) {
      ++i;
      continue;
    }

    if (expectValue) {
      if (c == '(') {
        tokens.push_back({BigFloatToken::Type::LeftParen, "", 0, ""});
        ++i;
        continue;
      }

      int sign = 1;
      bool sawUnarySign = false;
      if (c == '+' || c == '-') {
        sawUnarySign = true;
        sign = (c == '-') ? -1 : 1;
        ++i;
        while (i < expression.size() &&
               std::isspace(static_cast<unsigned char>(expression[i]))) {
          ++i;
        }
        if (i >= expression.size()) {
          throw std::invalid_argument(
              "Expression cannot end with a unary operator.");
        }
        c = expression[i];
        if (c == '(') {
          if (sign == -1) {
            tokens.push_back({BigFloatToken::Type::Number, "0", 0, ""});
            tokens.push_back({BigFloatToken::Type::Operator, "", '-', ""});
          }
          expectValue = true;
          continue;
        }
      } else {
        sign = 1;
      }

      if (std::isdigit(static_cast<unsigned char>(c)) || c == '.') {
        std::string number = parseDecimalToken(expression, i);
        if (sign == -1) {
          number.insert(number.begin(), '-');
        }
        tokens.push_back({BigFloatToken::Type::Number, number, 0, ""});
        expectValue = false;
        continue;
      }

      if (std::isalpha(static_cast<unsigned char>(c))) {
        std::size_t start = i;
        ++i;
        while (i < expression.size() &&
               (std::isalnum(static_cast<unsigned char>(expression[i])) ||
                expression[i] == '_')) {
          ++i;
        }
        std::string identifier = expression.substr(start, i - start);
        std::string lowered = identifier;
        std::transform(lowered.begin(), lowered.end(), lowered.begin(),
                       [](unsigned char ch) {
                         return static_cast<char>(std::tolower(ch));
                       });

        if (lowered == "sin" || lowered == "cos" || lowered == "log" ||
            lowered == "tan" || lowered == "sqrt" || lowered == "exp" ||
            lowered == "cot" || lowered == "asin" || lowered == "acos" ||
            lowered == "atan" || lowered == "sinh") {
          if (sawUnarySign && sign == -1) {
            tokens.push_back({BigFloatToken::Type::Number, "0", 0, ""});
            tokens.push_back({BigFloatToken::Type::Operator, "", '-', ""});
          }
          tokens.push_back(
              {BigFloatToken::Type::Function, "", 0, lowered});

          std::size_t lookahead = i;
          while (
              lookahead < expression.size() &&
              std::isspace(static_cast<unsigned char>(expression[lookahead]))) {
            ++lookahead;
          }
          if (lookahead >= expression.size() || expression[lookahead] != '(') {
            throw std::invalid_argument("Function '" + identifier +
                                        "' must be followed by parentheses.");
          }
          i = lookahead;
          continue;
        }

        if (sawUnarySign && sign == -1) {
          tokens.push_back({BigFloatToken::Type::Number, "0", 0, ""});
          tokens.push_back({BigFloatToken::Type::Operator, "", '-', ""});
        }
        tokens.push_back({BigFloatToken::Type::Variable, "", 0, lowered});
        expectValue = false;
        continue;
      }

      throw std::invalid_argument(
          "Expected a number or '(' in the expression.");
    } else {
      if (c == ')') {
        tokens.push_back({BigFloatToken::Type::RightParen, "", 0, ""});
        ++i;
        continue;
      }

      if (c == '!') {
        tokens.push_back({BigFloatToken::Type::Operator, "", '!', ""});
        ++i;
        continue;
      }

      if (isOperatorChar(c)) {
        tokens.push_back(
            {BigFloatToken::Type::Operator, "", normalizeOperator(c), ""});
        ++i;
        expectValue = true;
        continue;
      }

      throw std::invalid_argument(
          "Expected an operator or ')' in the expression.");
    }
  }

  if (expectValue) {
    throw std::invalid_argument(
        "Expression ended unexpectedly. Operand missing.");
  }

  return tokens;
}
} // namespace expression_detail

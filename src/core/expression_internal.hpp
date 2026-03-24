#pragma once

#include <string>
#include <vector>

namespace expression_detail {
struct Token {
  enum class Type {
    Number,
    Operator,
    Function,
    Variable,
    LeftParen,
    RightParen
  } type;
  double value{};
  char op{};
  std::string text;
};

struct BigToken {
  enum class Type { Number, Operator, Variable, LeftParen, RightParen } type;
  std::string number;
  char op{};
  std::string text;
};

struct BigFloatToken {
  enum class Type {
    Number,
    Operator,
    Function,
    Variable,
    LeftParen,
    RightParen
  } type;
  std::string number;
  char op{};
  std::string text;
};

std::vector<Token> tokenizeExpression(const std::string &expression);
std::vector<Token> toRpn(const std::vector<Token> &tokens);
std::vector<BigToken> tokenizeExpressionBigInt(const std::string &expression);
std::vector<BigToken> toRpnBigInt(const std::vector<BigToken> &tokens);
std::vector<BigFloatToken>
tokenizeExpressionBigDouble(const std::string &expression);
std::vector<BigFloatToken> toRpnBigDouble(
    const std::vector<BigFloatToken> &tokens);
} // namespace expression_detail

#include "expression_internal.hpp"

#include <stdexcept>
#include <vector>

namespace {
int precedence(char op) {
  if (op == '+' || op == '-') {
    return 1;
  }
  if (op == '*' || op == '/') {
    return 2;
  }
  if (op == '^') {
    return 3;
  }
  if (op == '!') {
    return 4;
  }
  throw std::invalid_argument("Unknown operator encountered.");
}

bool isRightAssociative(char op) {
  return op == '^';
}
} // namespace

namespace expression_detail {
std::vector<Token> toRpn(const std::vector<Token> &tokens) {
  std::vector<Token> output;
  std::vector<Token> stack;

  for (const Token &token : tokens) {
    switch (token.type) {
    case Token::Type::Number:
      output.push_back(token);
      break;
    case Token::Type::Variable:
      output.push_back(token);
      break;
    case Token::Type::Function:
      stack.push_back(token);
      break;
    case Token::Type::Operator:
      while (!stack.empty() && stack.back().type == Token::Type::Operator) {
        int stackPrecedence = precedence(stack.back().op);
        int tokenPrecedence = precedence(token.op);
        bool shouldPop = isRightAssociative(token.op)
                             ? stackPrecedence > tokenPrecedence
                             : stackPrecedence >= tokenPrecedence;
        if (!shouldPop) {
          break;
        }
        output.push_back(stack.back());
        stack.pop_back();
      }
      stack.push_back(token);
      break;
    case Token::Type::LeftParen:
      stack.push_back(token);
      break;
    case Token::Type::RightParen:
      while (!stack.empty() && stack.back().type != Token::Type::LeftParen) {
        output.push_back(stack.back());
        stack.pop_back();
      }
      if (stack.empty() || stack.back().type != Token::Type::LeftParen) {
        throw std::invalid_argument("Mismatched parentheses in expression.");
      }
      stack.pop_back();
      if (!stack.empty() && stack.back().type == Token::Type::Function) {
        output.push_back(stack.back());
        stack.pop_back();
      }
      break;
    }
  }

  while (!stack.empty()) {
    if (stack.back().type == Token::Type::LeftParen ||
        stack.back().type == Token::Type::RightParen) {
      throw std::invalid_argument("Mismatched parentheses in expression.");
    }
    output.push_back(stack.back());
    stack.pop_back();
  }

  return output;
}

std::vector<BigToken> toRpnBigInt(const std::vector<BigToken> &tokens) {
  std::vector<BigToken> output;
  std::vector<BigToken> stack;

  for (const BigToken &token : tokens) {
    switch (token.type) {
    case BigToken::Type::Number:
      output.push_back(token);
      break;
    case BigToken::Type::Variable:
      output.push_back(token);
      break;
    case BigToken::Type::Operator:
      while (!stack.empty() && stack.back().type == BigToken::Type::Operator &&
             precedence(stack.back().op) >= precedence(token.op)) {
        output.push_back(stack.back());
        stack.pop_back();
      }
      stack.push_back(token);
      break;
    case BigToken::Type::LeftParen:
      stack.push_back(token);
      break;
    case BigToken::Type::RightParen:
      while (!stack.empty() &&
             stack.back().type != BigToken::Type::LeftParen) {
        output.push_back(stack.back());
        stack.pop_back();
      }
      if (stack.empty() || stack.back().type != BigToken::Type::LeftParen) {
        throw std::invalid_argument("Mismatched parentheses in expression.");
      }
      stack.pop_back();
      break;
    }
  }

  while (!stack.empty()) {
    if (stack.back().type == BigToken::Type::LeftParen ||
        stack.back().type == BigToken::Type::RightParen) {
      throw std::invalid_argument("Mismatched parentheses in expression.");
    }
    output.push_back(stack.back());
    stack.pop_back();
  }

  return output;
}

std::vector<BigFloatToken>
toRpnBigDouble(const std::vector<BigFloatToken> &tokens) {
  std::vector<BigFloatToken> output;
  std::vector<BigFloatToken> stack;

  for (const BigFloatToken &token : tokens) {
    switch (token.type) {
    case BigFloatToken::Type::Number:
      output.push_back(token);
      break;
    case BigFloatToken::Type::Variable:
      output.push_back(token);
      break;
    case BigFloatToken::Type::Function:
      stack.push_back(token);
      break;
    case BigFloatToken::Type::Operator:
      while (!stack.empty() &&
             stack.back().type == BigFloatToken::Type::Operator) {
        int stackPrecedence = precedence(stack.back().op);
        int tokenPrecedence = precedence(token.op);
        bool shouldPop = isRightAssociative(token.op)
                             ? stackPrecedence > tokenPrecedence
                             : stackPrecedence >= tokenPrecedence;
        if (!shouldPop) {
          break;
        }
        output.push_back(stack.back());
        stack.pop_back();
      }
      stack.push_back(token);
      break;
    case BigFloatToken::Type::LeftParen:
      stack.push_back(token);
      break;
    case BigFloatToken::Type::RightParen:
      while (!stack.empty() &&
             stack.back().type != BigFloatToken::Type::LeftParen) {
        output.push_back(stack.back());
        stack.pop_back();
      }
      if (stack.empty() ||
          stack.back().type != BigFloatToken::Type::LeftParen) {
        throw std::invalid_argument("Mismatched parentheses in expression.");
      }
      stack.pop_back();
      if (!stack.empty() && stack.back().type == BigFloatToken::Type::Function) {
        output.push_back(stack.back());
        stack.pop_back();
      }
      break;
    }
  }

  while (!stack.empty()) {
    if (stack.back().type == BigFloatToken::Type::LeftParen ||
        stack.back().type == BigFloatToken::Type::RightParen) {
      throw std::invalid_argument("Mismatched parentheses in expression.");
    }
    output.push_back(stack.back());
    stack.pop_back();
  }

  return output;
}
} // namespace expression_detail

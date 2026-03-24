#include <gtest/gtest.h>
#include <map>

#include "core/expression.hpp"

TEST(ExpressionTest, SimpleArithmetic)
{
    EXPECT_DOUBLE_EQ(evaluateExpression("1+1"), 2);
    EXPECT_DOUBLE_EQ(evaluateExpression("10 - 3"), 7);
    EXPECT_DOUBLE_EQ(evaluateExpression("2 * 5"), 10);
    EXPECT_DOUBLE_EQ(evaluateExpression("20 / 4"), 5);
}

TEST(ExpressionTest, ComplexArithmetic)
{
    EXPECT_DOUBLE_EQ(evaluateExpression("2+3*4"), 14);
    EXPECT_DOUBLE_EQ(evaluateExpression("(2+3)*4"), 20);
    EXPECT_NEAR(evaluateExpression("3.5 * 2.2"), 7.7, 1e-9);
}

TEST(ExpressionTest, TrigonometryAndFunctions)
{
    EXPECT_NEAR(evaluateExpression("sin(0)"), 0.0, 1e-9);
    EXPECT_NEAR(evaluateExpression("cos(0)"), 1.0, 1e-9);
    EXPECT_NEAR(evaluateExpression("tan(0.78539816339)"), 1.0, 1e-6);
    EXPECT_NEAR(evaluateExpression("log(exp(1))"), 1.0, 1e-9);
}

TEST(ExpressionTest, Factorial)
{
    EXPECT_DOUBLE_EQ(evaluateExpression("5!"), 120);
}

TEST(ExpressionTest, Variables)
{
    std::map<std::string, double> vars{
        {"x", 2.5},
        {"pi", 3.141592653589793}
    };
    EXPECT_DOUBLE_EQ(evaluateExpression("x + 1", vars), 3.5);
    EXPECT_NEAR(evaluateExpression("pi * 2", vars), 6.283185307179586, 1e-12);
    EXPECT_NEAR(evaluateExpression("pi + x", vars), 5.641592653589793, 1e-12);
}

TEST(ExpressionTest, BigDoubleArithmetic)
{
    EXPECT_EQ(evaluateExpressionBigDouble("0.1 + 0.2"), "0.3");
    EXPECT_EQ(evaluateExpressionBigDouble("2^10"), "1024");
}

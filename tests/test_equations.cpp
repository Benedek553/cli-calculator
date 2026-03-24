#include <functional>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <string>

#include "ansi_colors.hpp"
#include "core/equations.hpp"

namespace
{
    std::string captureOutput(std::function<void()> fn)
    {
        std::ostringstream oss;
        std::streambuf *oldBuf = std::cout.rdbuf(oss.rdbuf());
        fn();
        std::cout.rdbuf(oldBuf);
        return oss.str();
    }
}

TEST(EquationTest, LinearEquation)
{
    setColorsEnabled(false);
    std::string output = captureOutput([]()
                                       { solveLinearEquation(2.0, -4.0); });
    EXPECT_NE(output.find("x = 2"), std::string::npos);
}

TEST(EquationTest, LinearNoSolution)
{
    setColorsEnabled(false);
    std::string output = captureOutput([]()
                                       { solveLinearEquation(0.0, 5.0); });
    EXPECT_NE(output.find("No solution"), std::string::npos);
}

TEST(EquationTest, QuadraticEquation)
{
    setColorsEnabled(false);
    std::string output = captureOutput([]()
                                       { solveQuadraticEquation(1.0, -5.0, 6.0); });
    EXPECT_NE(output.find("x1 = 3"), std::string::npos);
    EXPECT_NE(output.find("x2 = 2"), std::string::npos);
}

TEST(EquationTest, CubicEquation)
{
    setColorsEnabled(false);
    std::string output = captureOutput([]()
                                       { solveCubicEquation(1.0, 0.0, 0.0, -1.0); });
    EXPECT_NE(output.find("One real solution"), std::string::npos);
}

TEST(EquationTest, LinearSystem2x2)
{
    setColorsEnabled(false);
    std::string output =
        captureOutput([]() { solveLinearSystem2x2(1.0, 1.0, 3.0, 2.0, -1.0, 0.0); });
    EXPECT_NE(output.find("x = 1"), std::string::npos);
    EXPECT_NE(output.find("y = 2"), std::string::npos);
}

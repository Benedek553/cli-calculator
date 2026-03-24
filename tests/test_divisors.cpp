#include <gtest/gtest.h>
#include <stdexcept>

#include "core/divisors.hpp"


TEST(DivisorTest, DivisorsOfZeroThrowsException) {
    EXPECT_THROW(calculateDivisors(0), std::invalid_argument);
}
TEST(DivisorTest, CalculatesDivisorsOfTen) {
    EXPECT_EQ(calculateDivisors(10), std::vector<long long>({1, 2, 5, 10}));
}
TEST(DivisorTest, TestNegativeDivisors) {
    EXPECT_EQ(calculateDivisors(-10), std::vector<long long>({1, 2, 5, 10}));
}
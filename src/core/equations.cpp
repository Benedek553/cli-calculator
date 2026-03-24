#include "equations.hpp"
#include "ansi_colors.hpp"
#include "math_utils.hpp"
#include <complex>
#include <iostream>

void solveLinearEquation(double a, double b) {
  if (isApproximatelyZero(a)) {
    if (isApproximatelyZero(b)) {
      std::cout << CYAN << "Every real number is a solution." << RESET << '\n';
    } else {
      std::cout << RED << "No solution exists for this equation." << RESET
                << '\n';
    }
    return;
  }

  double result = -b / a;
  std::cout << GREEN << "Solution: x = " << RESET << result << '\n';
}

void solveQuadraticEquation(double a, double b, double c) {
  if (isApproximatelyZero(a)) {
    std::cout << YELLOW
              << "Coefficient 'a' is zero; falling back to a linear equation."
              << RESET << '\n';
    solveLinearEquation(b, c);
    return;
  }

  double discriminant = b * b - 4.0 * a * c;
  constexpr double epsilon = 1e-9;

  if (discriminant > epsilon) {
    double sqrtDisc = std::sqrt(discriminant);
    double denom = 2.0 * a;
    std::cout << CYAN << "Two real solutions:" << RESET << '\n';
    std::cout << GREEN << " x1 = " << RESET << (-b + sqrtDisc) / denom << '\n';
    std::cout << GREEN << " x2 = " << RESET << (-b - sqrtDisc) / denom << '\n';
  } else if (isApproximatelyZero(discriminant, epsilon)) {
    double root = -b / (2.0 * a);
    std::cout << GREEN << "One real solution (double root): x = " << RESET
              << root << '\n';
  } else {
    std::complex<double> sqrtDisc =
        std::sqrt(std::complex<double>(discriminant, 0.0));
    std::complex<double> denom(2.0 * a, 0.0);
    std::complex<double> x1 = (-b + sqrtDisc) / denom;
    std::complex<double> x2 = (-b - sqrtDisc) / denom;

    auto printComplex = [](const std::complex<double> &value) {
      double realPart = value.real();
      double imagPart = value.imag();
      std::cout << GREEN << realPart << RESET;
      if (!isApproximatelyZero(imagPart)) {
        if (imagPart >= 0) {
          std::cout << " + " << GREEN << imagPart << RESET << "i";
        } else {
          std::cout << " - " << GREEN << std::abs(imagPart) << RESET << "i";
        }
      }
    };

    std::cout << "Two complex solutions:\n x1 = ";
    printComplex(x1);
    std::cout << "\n x2 = ";
    printComplex(x2);
    std::cout << '\n';
  }
}

namespace {
void printComplexValue(const std::complex<double> &value,
                       double epsilon = 1e-9) {
  double realPart = value.real();
  double imagPart = value.imag();
  std::cout << GREEN << realPart << RESET;
  if (!isApproximatelyZero(imagPart, epsilon)) {
    if (imagPart >= 0) {
      std::cout << " + " << GREEN << imagPart << RESET << "i";
    } else {
      std::cout << " - " << GREEN << std::abs(imagPart) << RESET << "i";
    }
  }
}

bool isNearlyReal(const std::complex<double> &value, double epsilon = 1e-9) {
  return isApproximatelyZero(value.imag(), epsilon);
}
} // namespace

void solveCubicEquation(double a, double b, double c, double d) {
  if (isApproximatelyZero(a)) {
    std::cout << YELLOW
              << "Coefficient 'a' is zero; falling back to a quadratic equation."
              << RESET << '\n';
    solveQuadraticEquation(b, c, d);
    return;
  }

  double p = (3.0 * a * c - b * b) / (3.0 * a * a);
  double q =
      (2.0 * b * b * b - 9.0 * a * b * c + 27.0 * a * a * d) /
      (27.0 * a * a * a);
  double discriminant = (q * q) / 4.0 + (p * p * p) / 27.0;

  std::complex<double> sqrtDisc =
      std::sqrt(std::complex<double>(discriminant, 0.0));
  std::complex<double> u = std::pow(-q / 2.0 + sqrtDisc, 1.0 / 3.0);
  std::complex<double> v = std::pow(-q / 2.0 - sqrtDisc, 1.0 / 3.0);
  std::complex<double> omega(-0.5, std::sqrt(3.0) / 2.0);
  std::complex<double> omegaConj = std::conj(omega);
  double shift = -b / (3.0 * a);

  std::complex<double> x1 = u + v + shift;
  std::complex<double> x2 = u * omega + v * omegaConj + shift;
  std::complex<double> x3 = u * omegaConj + v * omega + shift;

  constexpr double epsilon = 1e-8;
  int realCount = 0;
  realCount += isNearlyReal(x1, epsilon) ? 1 : 0;
  realCount += isNearlyReal(x2, epsilon) ? 1 : 0;
  realCount += isNearlyReal(x3, epsilon) ? 1 : 0;

  if (realCount == 3) {
    std::cout << CYAN << "Three real solutions:" << RESET << '\n';
    std::cout << GREEN << " x1 = " << RESET << x1.real() << '\n';
    std::cout << GREEN << " x2 = " << RESET << x2.real() << '\n';
    std::cout << GREEN << " x3 = " << RESET << x3.real() << '\n';
    return;
  }

  if (realCount == 1) {
    std::cout << CYAN << "One real solution and two complex solutions:"
              << RESET << '\n';
  } else {
    std::cout << CYAN << "Solutions:" << RESET << '\n';
  }
  std::cout << " x1 = ";
  printComplexValue(x1, epsilon);
  std::cout << "\n x2 = ";
  printComplexValue(x2, epsilon);
  std::cout << "\n x3 = ";
  printComplexValue(x3, epsilon);
  std::cout << '\n';
}

void solveLinearSystem2x2(double a1, double b1, double c1, double a2,
                          double b2, double c2) {
  double determinant = a1 * b2 - a2 * b1;
  constexpr double epsilon = 1e-9;
  if (isApproximatelyZero(determinant, epsilon)) {
    bool consistent =
        isApproximatelyZero(a1 * c2 - a2 * c1, epsilon) &&
        isApproximatelyZero(b1 * c2 - b2 * c1, epsilon);
    if (consistent) {
      std::cout << CYAN << "Infinitely many solutions (dependent equations)."
                << RESET << '\n';
    } else {
      std::cout << RED << "No solution exists for this system." << RESET
                << '\n';
    }
    return;
  }

  double x = (c1 * b2 - c2 * b1) / determinant;
  double y = (a1 * c2 - a2 * c1) / determinant;
  std::cout << GREEN << "Solution: x = " << RESET << x << '\n';
  std::cout << GREEN << "Solution: y = " << RESET << y << '\n';
}

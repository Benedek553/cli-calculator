#pragma once

#include "cli_output.hpp"

#include <optional>
#include <string>
#include <vector>

int runEval(const std::string &expression, OutputFormat outputFormat,
            std::optional<double> *lastResult = nullptr,
            bool useBigInt = false, bool useBigDouble = false);
int runSquareRoot(const std::string &number, OutputFormat outputFormat,
                  std::optional<double> *lastResult = nullptr);
int runDivisors(const std::string &input, OutputFormat outputFormat);
int runConvert(const std::string &fromBaseStr, const std::string &toBaseStr,
               const std::string &valueStr, OutputFormat outputFormat);
int runUnitConvert(const std::string &category, const std::string &fromUnit,
                   const std::string &toUnit, const std::string &valueStr,
                   OutputFormat outputFormat);
int runPrimeFactorization(const std::string &input, OutputFormat outputFormat);
int runSolveLinear(const std::string &aStr, const std::string &bStr,
                   OutputFormat outputFormat);
int runSolveQuadratic(const std::string &aStr, const std::string &bStr,
                      const std::string &cStr, OutputFormat outputFormat);
int runMatrixAdd(const std::string &lhsStr, const std::string &rhsStr,
                 OutputFormat outputFormat);
int runMatrixSubtract(const std::string &lhsStr, const std::string &rhsStr,
                      OutputFormat outputFormat);
int runMatrixMultiply(const std::string &lhsStr, const std::string &rhsStr,
                      OutputFormat outputFormat);
int runStatistics(const std::vector<std::string> &tokens,
                  OutputFormat outputFormat);
int runGraphValues(const std::vector<std::string> &tokens,
                   OutputFormat outputFormat);
int runGraphCsv(const std::vector<std::string> &tokens,
                OutputFormat outputFormat);
int runVersion(OutputFormat outputFormat);
int runListVariables(OutputFormat outputFormat);
int runSetVariable(const std::string &name, const std::string &valueStr,
                   OutputFormat outputFormat);
int runUnsetVariable(const std::string &name, OutputFormat outputFormat);
int runReportBug(OutputFormat outputFormat);
int runHelp(OutputFormat outputFormat);

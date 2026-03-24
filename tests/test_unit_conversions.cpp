#include <gtest/gtest.h>
#include <stdexcept>

#include "core/unit_conversion.hpp"

const TemperatureUnit fahrenheit = {"Fahrenheit", "F", TemperatureScale::Fahrenheit};
const TemperatureUnit celsius = {"Celsius", "C", TemperatureScale::Celsius};
const TemperatureUnit kelvin = {"Kelvin", "K", TemperatureScale::Kelvin};
const LinearUnit meter = {"Meter", "m", 1.0};
const LinearUnit kilometer = {"Kilometer", "km", 1000.0};
const LinearUnit centimeter = {"Centimeter", "cm", 0.01};
const LinearUnit millimeter = {"Millimeter", "mm", 0.001};
const LinearUnit mile = {"Mile", "mi", 1609.344};
const LinearUnit yard = {"Yard", "yd", 0.9144};
const LinearUnit foot = {"Foot", "ft", 0.3048};
const LinearUnit inch = {"Inch", "in", 0.0254};
const LinearUnit kilogram = {"Kilogram", "kg", 1.0};
const LinearUnit gram = {"Gram", "g", 0.001};
const LinearUnit milligram = {"Milligram", "mg", 0.000001};
const LinearUnit metric_ton = {"Metric ton", "t", 1000.0};
const LinearUnit pound = {"Pound", "lb", 0.45359237};
const LinearUnit ounce = {"Ounce", "oz", 0.028349523125};
const LinearUnit liter = {"Liter", "L", 1.0};
const LinearUnit milliliter = {"Milliliter", "mL", 0.001};
const LinearUnit cubic_meter = {"Cubic meter", "m^3", 1000.0};
const LinearUnit gallon_us = {"Gallon (US)", "gal", 3.78541};
const LinearUnit pint_us = {"Pint (US)", "pt", 0.473176};

TEST(UnitConversionTest, FahrenheitToCelsius) {
    EXPECT_NEAR(convertTemperature(32.0, fahrenheit, celsius), 0.0, 1e-6);
}
TEST(UnitConversionTest, FahrenheitToKelvin) {
    EXPECT_NEAR(convertTemperature(32.0, fahrenheit, kelvin), 273.15, 1e-6);
}
TEST(UnitConversionTest, CelsiusToFahrenheit) {
    EXPECT_NEAR(convertTemperature(100.0, celsius, fahrenheit), 212.0, 1e-6);
}
TEST(UnitConversionTest, CelsiusToKelvin) {
    EXPECT_NEAR(convertTemperature(0.0, celsius, kelvin), 273.15, 1e-6);
}
TEST(UnitConversionTest, KelvinToCelsius) {
    EXPECT_NEAR(convertTemperature(373.15, kelvin, celsius), 100.0, 1e-6);
}
TEST(UnitConversionTest, KelvinToFahrenheit) {
    EXPECT_NEAR(convertTemperature(273.15, kelvin, fahrenheit), 32.0, 1e-6);
}
TEST(UnitConversionTest, SameUnitConversion) {
    EXPECT_NEAR(convertTemperature(25.0, celsius, celsius), 25.0, 1e-6);
    EXPECT_NEAR(convertTemperature(77.0, fahrenheit, fahrenheit), 77.0, 1e-6);
    EXPECT_NEAR(convertTemperature(300.0, kelvin, kelvin), 300.0, 1e-6);
}
TEST(UnitConversionTest, MeterToKilometer) {
    EXPECT_NEAR(convertLinearValue(1000.0, meter, kilometer), 1.0, 1e-6);
}
TEST(UnitConversionTest, KilometerToMeter) {
    EXPECT_NEAR(convertLinearValue(1.0, kilometer, meter), 1000.0, 1e-6);
}
TEST(UnitConversionTest, CentimeterToMeter) {
    EXPECT_NEAR(convertLinearValue(100.0, centimeter, meter), 1.0, 1e-6);
}
TEST(UnitConversionTest, MeterToMillimeter) {
    EXPECT_NEAR(convertLinearValue(1.0, meter, millimeter), 1000.0, 1e-6);
}
TEST(UnitConversionTest, MileToKilometer) {
    EXPECT_NEAR(convertLinearValue(1.0, mile, kilometer), 1.609344, 1e-6);
}
TEST(UnitConversionTest, YardToMeter) {
    EXPECT_NEAR(convertLinearValue(1.0, yard, meter), 0.9144, 1e-6);
}
TEST(UnitConversionTest, FootToMeter) {
    EXPECT_NEAR(convertLinearValue(1.0, foot, meter), 0.3048, 1e-6);
}
TEST(UnitConversionTest, InchToCentimeter) {
    EXPECT_NEAR(convertLinearValue(1.0, inch, centimeter), 2.54, 1e-6);
}
TEST(UnitConversionTest, KilogramToGram) {
    EXPECT_NEAR(convertLinearValue(1.0, kilogram, gram), 1000.0, 1e-6);
}
TEST(UnitConversionTest, GramToMilligram) {
    EXPECT_NEAR(convertLinearValue(1.0, gram, milligram), 1000.0, 1e-6);
}
TEST(UnitConversionTest, KilogramToPound) {
    EXPECT_NEAR(convertLinearValue(1.0, kilogram, pound), 2.20462, 1e-5);
}
TEST(UnitConversionTest, PoundToOunce) {
    EXPECT_NEAR(convertLinearValue(1.0, pound, ounce), 16.0, 1e-6);
}
TEST(UnitConversionTest, LiterToMilliliter) {
    EXPECT_NEAR(convertLinearValue(1.0, liter, milliliter), 1000.0, 1e-6);
}
TEST(UnitConversionTest, CubicMeterToLiter) {
    EXPECT_NEAR(convertLinearValue(1.0, cubic_meter, liter), 1000.0, 1e-6);
}
TEST(UnitConversionTest, GallonToLiter) {
    EXPECT_NEAR(convertLinearValue(1.0, gallon_us, liter), 3.78541, 1e-5);
}
TEST(UnitConversionTest, PintToLiter) {
    EXPECT_NEAR(convertLinearValue(1.0, pint_us, liter), 0.473176, 1e-6);
}
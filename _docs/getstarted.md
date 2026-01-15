---
layout: page
title: Get Started with CLI Calculator
---

A fast, script-friendly command-line calculator with a REPL, rich expression support, and power features for everyday math and developer workflows.

## 1) First run

### Interactive Menu

Start the menu:

```bash
calculator
```

Then select modes:

```bash
Welcome to the CLI Calculator!

=== Main Menu ===
 1) Basic operations
 2) Conversion tools
 3) Divisor finder
 4) Equation solver
 5) Matrix operations
 6) Square root calculator
 7) Variable manager
 8) Prime factorization
 9) Statistical tools
10) Graph utility
11) Launch REPL mode
12) Report a bug
 0) Exit
Select option: 1

--- Expression Evaluator ---
 1) Standard (double)
 2) Bigint (integers only)
 3) Bigdouble (high-precision decimals)
 0) Back
Select option: 1
Enter an expression (type 'back' to return): 2*2
Result: 4
```

### Interactive REPL

Start the REPL:

```bash
cli-calculator --repl
```

Then type expressions:

```
> 2 + 2
4
> (3 + 5) * 2
16
```

### One-shot evaluation

If you prefer non-interactive usage (great for scripts), run a single expression and exit:

```bash
cli-calculator --eval "2 + 2"
```

Tip: quote expressions to avoid shell interpretation.

## 3) Core syntax

### Numbers & operators

- Supports standard arithmetic: `+ - * /` and parentheses `(...)`
- Use decimals normally: `3.14`, `0.001`

Examples:

```
> 10 / 4
2.5
> 2*(7-3)
8

```

### Variables

Assign and reuse values:

```
> x = 42
> x * 2
84

```

### Functions

Typical scientific-style functions are available (names may vary by build). Try:

```
> sin(0)
0
> sqrt(81)
9

```

If a function name fails, use `help` (see below) to list what your build supports.

## 4) Useful commands

Inside the REPL, these are commonly available:

- `help` — show commands and features
- `exit` / `quit` — leave the REPL
- `clear` — clear the screen (if supported)

Example:

```
> help
... lists commands, operators, and modules ...

```

## 5) Big numbers (BigInt / BigDouble)

CLI Calculator can handle larger-than-standard values depending on your build configuration.

Try:

```
> 999999999999999999999999 + 1
1000000000000000000000000

```

If you see a limitation message, your build may be using a standard numeric backend.

## 6) Matrices (if enabled)

Some builds include matrix operations.

Example idea (syntax may vary):

```
> A = [[1,2],[3,4]]
> det(A)
-2

```

If this doesn’t parse, use `help` to see the exact matrix syntax in your version.

## 7) Tips for scripting

### Exit codes

Use one-shot evaluation for scripts and CI:

```bash
cli-calculator "3*3"

```

### Avoid shell pitfalls

- Quote expressions containing , `(`, `)`, `^`, or spaces.
- On PowerShell, prefer double quotes and escape where needed.

## 8) Troubleshooting

### “Command not found”

- Snap: make sure `~/snap/bin` is in your `PATH` (or log out/in).
- Docker: confirm you’re running the image command correctly.

### Unexpected parsing errors

- Wrap the expression in quotes.
- Use `help` to check supported operators/functions.

### Build issues

- Ensure your compiler supports modern C++ (C++17/20).
- Reconfigure from a clean build folder:

```bash
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

```

## 9) Next steps

- Explore `help` to discover modules and advanced features (graphing, extra functions, etc.).
- Check the repository README for the full feature list, releases, and platform-specific notes.

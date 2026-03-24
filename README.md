# cli-calculator
---

## Overview

**cli-calculator** is a C++ project providing two command-line tools:

### calculator

A full-featured CLI calculator supporting:

* Arbitrary-length expression evaluation
* Number-base conversion (bin / dec / hex)
* Unit conversion (length, mass, volume, temperature)
* Matrix operations (add / subtract / multiply)
* Divisor search
* Prime factorization with variable persistence
* CLI-only actions without launching the menu

### calculator_gui

Qt-based GUI that wraps the same calculation engine with tabs for expressions,
numbers, conversions, equations, matrices, graphs, statistics, variables, notes,
and an in-app terminal. Supports split-view tab layouts and Markdown notes with
LaTeX-style symbols.

---

## Quickstart

### Install via Snap (recommended)

```bash
sudo snap install cli-calculator
cli-calculator
```

### Manual install (prebuilt binary)

1. Download from **Releases**
2. Make executable:

```bash
chmod +x calculator
sudo mv calculator /usr/local/bin/
```

3. Verify:

```bash
calculator --help
```

---

## Requirements

* CMake >= 3.10
* C++17-compatible compiler (GCC / Clang / MSVC)
* zlib (PNG graph export)
* Qt5 or Qt6 Widgets (optional GUI)

---

## Build (from source)

```bash
cmake -S . -B build
cmake --build build --config Release
```

### Build with GUI

```bash
cmake -S . -B build -DBUILD_GUI=ON
cmake --build build --config Release
```

---

## Run

```bash
./build/src/calculator
./build/src/divisors
./build/src/calculator_gui
```

### Snap GUI

```bash
cli-calculator.gui
```

---

## Features (Summary)

| Area        | Capabilities                                 |
| ----------- | -------------------------------------------- |
| Expressions | Operators, parentheses, functions, variables |
| Conversions | Bases (2/10/16), measurement units           |
| Equations   | Linear & quadratic (complex roots)           |
| Matrices    | Add / subtract / multiply                    |
| Statistics  | Mean, median, variance, stddev, percentiles  |
| Graphs      | ASCII preview (CLI) + PNG export             |
| Variables   | Persisted via `vars.toml`                    |

---

## GUI Highlights

* Split view: open any two tabs side-by-side.
* Notes: Markdown editor with live preview, code block copy buttons, and file
  operations (new/open/save/save as).
* Graphs: generate PNGs from values or CSV and save to any location.
* Terminal: run CLI calculator commands or shell commands from the workspace
  directory.

## CLI Flags (Grouped)
[Detailed Documentation](https://github.com/Benedek553/cli-calculator/wiki/CLI-Flags-Documentation)
### Core execution

* `--eval <expr>`
* `--repl`
* `--version`
* `--no-color`

### Math & algebra

* `--square-root <v>`
* `--solve-linear a b`
* `--solve-quadratic a b c`

### Conversions

* `--convert <from> <to> <value>`
* `--unit-convert <category> <from> <to> <value>`

### Matrices

* `--matrix-add <A> <B>`
* `--matrix-subtract <A> <B>`
* `--matrix-multiply <A> <B>`

### Analysis

* `--stats <values...>`
* `--graph-values <out.png> <values...>`
* `--graph-csv <out.png> <csv> <column>`

### Variables

* `--variables`
* `--set-variable <name> <value>`
* `--unset-variable <name>`

### Automation

* `--batch <file>`
* `--output json|xml|yaml`

---

## Variables

* Stored in `vars.toml`
* Loaded on startup
* Managed via menu or CLI flags
* Names: `[A-Za-z][A-Za-z0-9_]*`

---

## Expression Functions

`sin cos tan cot asin acos atan log exp sqrt sinh !`

Multiplication and division also accept `x` and `:`.

---

## Graph Utility (Workflow)

1. Select data source (manual / CSV)
2. Preview via ASCII renderer
3. Export labeled PNG

Requires zlib.

---

## Code Structure

### Core (`src/core/`)

* expression
* numeral_conversion
* equations
* input
* math_utils
* divisors_lib

### App (`src/app/`)

* calculator_app
* cli_parser

### Tools

* `src/tools/divisors.cpp`
* `src/main.cpp`
* `src/ansi_colors.hpp`

---

## Contribution

Issues and pull requests are welcome.

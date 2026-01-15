---

layout: page

title: CLI Flags Reference

---



This page documents all command-line flags supported by CLI Calculator



\## Overview



CLI Calculator supports two execution models:



\* \*\*Interactive mode\*\* — default when no flags are provided

\* \*\*One-shot CLI mode\*\* — a single action is executed via flags, then the program exits



> Flags do \*\*not\*\* launch the interactive menu unless explicitly stated.



---



\## Global Flags



\### `--no-color`, `-nc`



Disable ANSI color output globally.



\*\*Recommended for:\*\* logs, CI, or terminals without ANSI support.



---



\### `--version`, `-v`



Print the application version and exit.



---



\### `--output <format>`



Emit structured output for supported one-shot actions.



\*\*Formats:\*\* `json`, `xml`, `yaml`



\*\*Rules:\*\*



\* Must be combined with \*\*exactly one\*\* action flag

\* Using `--output` alone is rejected



---



\## Expression Evaluation



\### `--eval <expression>`, `-e <expression>`



Evaluate a full mathematical expression and print the result.



```bash

calculator --eval "2 \* (3 + 4)"

```



---



\## REPL Mode



\### `--repl`



Launch a lightweight read–eval–print loop.



\*\*Features:\*\*



\* Command history (`Up` / `Down`)

\* `:history` and `!<n>` recall

\* All CLI flags available via `:command` or `--command`

\* Can also be started from the interactive menu (\*\*11) Launch REPL mode\*\*)



---



\## Numeric Operations



\### Square Root



\#### `--square-root <value>`, `-sqrt <value>`



Compute a single square root.



\* Negative inputs are rejected



```bash

calculator --square-root 144

```



---



\### Prime Factorization



\#### `--prime-factorization <value>`, `-pf <value>`



Display prime factors of an integer.



\* Negative inputs fall back to `-1`



```bash

calculator --prime-factorization 144

```



---



\### Divisors



\#### `--divisors <value>`



Compute all positive divisors of the given value.



```bash

calculator --divisors 144

```



---



\## Base Conversion



\### `--convert <from> <to> <value>`, `-c <from> <to> <value>`



Convert an integer between numeral systems.



\*\*Accepted bases:\*\* `2`, `10`, `16`



```bash

calculator --convert 10 16 78

calculator -c 2 10 0b1011

```



---



\## Unit Conversion



\### `--unit-convert <category> <from> <to> <value>`



Convert measurement units.



\*\*Categories:\*\* `length`, `mass`, `volume`, `temperature`



\* Units may be specified using names or symbols



```bash

calculator --unit-convert length m ft 144

```



---



\## Equation Solvers



\### Linear Equation



\#### `--solve-linear <a> <b>`



Solve:



```

a \* x + b = 0

```



---



\### Quadratic Equation



\#### `--solve-quadratic <a> <b> <c>`



Solve:



```

a \* x² + b \* x + c = 0

```



---



\## Matrix Operations



\### Matrix syntax



\* Rows separated by `;`

\* Columns separated by `,` or spaces



\### Addition



\#### `--matrix-add <A> <B>`



```bash

calculator --matrix-add "23;46;53;54;45" "64;56;23;234;54"

```



---



\### Subtraction



\#### `--matrix-subtract <A> <B>`



```bash

calculator --matrix-subtract "23;46;53;54;45" "64;56;23;234;54"

```



---



\### Multiplication



\#### `--matrix-multiply <A> <B>`



```bash

calculator --matrix-multiply "23;46;53;54;45" "64;56;23;234;54"

```



---



\## Statistics



\### `--stats <values...>`, `--statistics <values...>`



Compute summary statistics over a list of numeric values.



---



\## Graph Rendering



\### Graph from Values



\#### `--graph-values <output.png> <values...> \[--height N]`



\* Renders a PNG graph

\* Prints an ASCII preview to stdout



```bash

calculator --graph-values output1.png 12 24 34 45 53 --height 12

```



---



\### Graph from CSV



\#### `--graph-csv <output.png> <csv-path> <column> \[--height N] \[--no-headers]`



\* Renders a graph from a CSV column

\* Optional header skipping via `--no-headers`



```bash

calculator --graph-csv output1.png ./data.csv 1 --height 12

```



---



\## Variable Management



\### List Variables



\#### `--variables`, `--list-variables`



Print all persisted variables and their values.



---



\### Set Variable



\#### `--set-variable <name> <value>`



Create or update a variable in `vars.toml`.



```bash

calculator --set-variable x 144

```



---



\### Unset Variable



\#### `--unset-variable <name>`



Remove a persisted variable.



```bash

calculator --unset-variable x

```



---



\## Batch Execution



\### `--batch <file.txt>`, `-b <file.txt>`



Execute CLI commands from a text file.



\*\*Rules:\*\*



\* One CLI invocation per line

\* Lines starting with `#` are ignored



\*\*Directives:\*\*



\* `@set <variable>` — store previous numeric result

\* `@input <variable>` — prompt for input and store result

\* `@include <file>` — include another batch file

\* `@if <expression>` / `@endif` — conditional execution (non-zero = true)

\* `@unset <variable>` — remove a stored variable



---



\## Miscellaneous



\### `--report-bug`



Open the GitHub issue template in the default browser.



---



\## Variable Resolution Rules



Numeric CLI arguments may reference variables defined via:



\* Interactive menu

\* Batch mode (`@set`, `@input`)

\* Included batch files



Variables are resolved before execution.



---



\## Exit Codes



\* `0` — success

\* `non-zero` — invalid input, malformed arguments, or computation error



---



\## Notes \& Limitations



\* Only \*\*one\*\* action flag may be used at a time

\* Flags never trigger the interactive menu implicitly

\* Structured output requires a valid one-shot action flag




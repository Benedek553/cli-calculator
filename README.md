# CLI Calculator

A simple command-line calculator and prime number utility.

## Features

- Basic arithmetic operations (addition, subtraction, multiplication, division)
- Prime number checking and generation

## Installation

### Prerequisites

- C++ compiler (e.g., GCC, Clang)
- Bash (for running the setup script)

### Build

You can build the project using the provided setup script:

```sh
./quick-setup.sh
```

Alternatively, compile manually:

```sh
g++ -o calc calc.cpp
g++ -o prime prime.cpp
```

### Add to PATH

To use the calculator as a CLI command from anywhere, add the build directory to your `PATH`. For example, if your executables are in `/home/farsang/Dokumentumok/cli-calculator/cli-calculator`, run:

```sh
export PATH=$PATH:/home/farsang/Dokumentumok/cli-calculator/cli-calculator
```

To make this change permanent, add the above line to your `~/.bashrc` or `~/.zshrc` file.

## Usage

### Calculator

Run the calculator from anywhere:

```sh
calc
```

Follow the on-screen instructions to perform calculations.

### Prime Utility

Run the prime number utility:

```sh
prime
```

You will be prompted to enter a number to check if it is prime or generate primes.

## License

This project is licensed under the [Apache License 2.0](LICENSE).

## Contributing

Contributions are welcome! Please open issues or submit pull requests.

## Author

Farsang Benedek
@Benedek553
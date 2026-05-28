# Contributing to ChaiScript ☕

Thanks for your interest in contributing! Here's how to get started.

## Development Setup

1. Clone the repo and build:
   ```bash
   git clone https://github.com/Pratham-ji/chaiscript-lang.git
   cd chaiscript-lang
   mkdir build && cd build
   cmake .. && cmake --build .
   ```

2. Run the test suite:
   ```bash
   python3 tests/run_tests.py
   ```

## How to Contribute

1. **Fork** the repository
2. Create a **feature branch** (`git checkout -b feature/my-change`)
3. Make your changes and add tests if applicable
4. Run the full test suite to ensure nothing breaks
5. **Commit** with a clear message (`git commit -m "feat: add modulo operator"`)
6. **Push** and open a Pull Request

## Adding a New Language Feature

The compiler has a clean 4-phase pipeline. To add a new feature:

1. **Token.hpp** — Add new `TokenType` entries
2. **Lexer.cpp** — Add keyword recognition in `identifierType()`
3. **Parser.cpp** — Add parsing rules (statement or expression)
4. **Sema.cpp** — Add type-checking logic
5. **CodeGen.cpp** — Add C code emission
6. **tests/** — Add a test case with expected output

## Code Style

- Modern C++20 idioms (prefer `std::variant`, `std::string_view`, smart pointers)
- No raw `new`/`delete`
- All warnings enabled (`-Wall -Wextra -Wpedantic`)
- Comments should be clear and occasionally fun ☕

## Reporting Issues

Open an issue with:
- Your OS and compiler version
- The `.chai` source file that triggers the issue
- Expected vs actual output

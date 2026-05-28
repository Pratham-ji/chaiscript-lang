# ☕ ChaiScript

**A statically-typed, compiled programming language with Indian tea-themed syntax — hand-built in modern C++20.**

[![C++ Standard](https://img.shields.io/badge/C++-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Build System](https://img.shields.io/badge/Build-CMake-green.svg)](https://cmake.org/)
[![Backend](https://img.shields.io/badge/Backend-C%20Transpilation-orange.svg)](src/CodeGen.cpp)
[![Platform](https://img.shields.io/badge/Platform-macOS%20%7C%20Linux%20%7C%20Windows-lightgrey.svg)](#-quick-start)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Tests](https://img.shields.io/badge/Tests-Automated-brightgreen.svg)](tests/run_tests.py)

ChaiScript takes a culturally flavored syntax inspired by Indian chai culture and compiles it into optimized C code, which is then natively assembled via GCC or Clang. Under the hood, it features a **zero-copy lexer**, a **Pratt parser** for expression precedence, **scoped semantic analysis** with static type checking, and a clean **C code generator** — all engineered from scratch without any parser generators or third-party compiler libraries.

> **Sole Architect:** Prathamraj Giri

---

## 🍵 Syntax at a Glance

```javascript
// A fresh cup of ChaiScript
garam_kar {
    chai chini cups = 3;
    chai adrak greeting = "Namaste, duniya!";

    serve(greeting);

    agar (cups > 2) {
        serve("Bahut chai pi li!");
    } warna {
        serve("Aur chai lao!");
    }

    ubalo_jab_tak (cups > 0) {
        serve(cups);
        cups = cups - 1;
    }

    peelo 0;
}
```

**Output:**
```
Namaste, duniya!
Bahut chai pi li!
3
2
1
```

---

## 📖 Language Reference

| ChaiScript            | Meaning          | C Equivalent         |
|------------------------|------------------|----------------------|
| `garam_kar { }`        | Program entry    | `int main() { }`    |
| `peelo <expr>;`        | Return           | `return <expr>;`     |
| `chai`                 | Declare variable | *(declaration)*      |
| `chini`                | Integer type     | `int`                |
| `adrak`                | String type      | `const char*`        |
| `elaichi`              | Boolean type     | `bool`               |
| `kadak`                | True             | `true`               |
| `fika`                 | False            | `false`              |
| `serve(x);`            | Print to stdout  | `printf(..., x);`    |
| `agar (...) { }`       | If               | `if (...) { }`       |
| `warna { }`            | Else             | `else { }`           |
| `ubalo_jab_tak (...)` | While loop       | `while (...)`        |
| `// ek cup chai`       | Comment          | `// comment`         |

### Type System

ChaiScript is **statically typed**. Every variable must declare its type at the point of declaration:

```javascript
chai chini   age = 25;                // integer
chai adrak   name = "Pratham";        // string
chai elaichi is_ready = kadak;        // boolean (true)
```

The compiler enforces type safety at compile time. Attempting to mix incompatible types (e.g., adding `adrak` to `chini`) produces a clear diagnostic:

```
Type Error: Cannot mix adrak and chini here.
```

### Operators

| Operator | Meaning              | Precedence |
|----------|----------------------|------------|
| `*` `/`  | Multiply / Divide    | Highest    |
| `+` `-`  | Add / Subtract       | ↓          |
| `<` `>` `<=` `>=` | Comparison  | ↓          |
| `==` `!=`| Equality             | ↓          |
| `=`      | Assignment           | Lowest     |

Expression precedence is handled by **Pratt Parsing** (precedence climbing), resolving `a + b * c` correctly as `a + (b * c)` without deep recursive call stacks.

---

## 🚀 Quick Start

### Prerequisites

<details>
<summary><strong>macOS</strong></summary>

```bash
# Install with Homebrew
brew install cmake ninja gcc
# Python 3 is pre-installed on macOS
```
</details>

<details>
<summary><strong>Linux (Ubuntu/Debian)</strong></summary>

```bash
sudo apt update
sudo apt install cmake ninja-build g++ python3
```
</details>

<details>
<summary><strong>Linux (Fedora/RHEL)</strong></summary>

```bash
sudo dnf install cmake ninja-build gcc-c++ python3
```
</details>

<details>
<summary><strong>Windows</strong></summary>

**Option A: MSYS2 (Recommended)**
```powershell
# Install MSYS2 from https://www.msys2.org/
# Then inside MSYS2 terminal:
pacman -S cmake ninja gcc python3
```

**Option B: Visual Studio**
```powershell
# Install Visual Studio with C++ workload
# Install CMake from https://cmake.org/download/
# Install Python from https://python.org
```
</details>

### Build & Run

```bash
git clone https://github.com/Pratham-ji/chaiscript-lang.git
cd chaiscript-lang
python3 tools/brew_chai.py examples/hello_chai.chai
```

The orchestrator handles everything automatically:

```
☕ [1/4] Brewing the ChaiScript Compiler (C++20)...
🍵 [2/4] Steeping hello_chai.chai → C...
⚙️  [3/4] Compiling generated C with gcc...
🚀 [4/4] Serving fresh chai:
──────────────────────────────────────────────────
Namaste, duniya!
5
Bahut chai pi li!
3
2
1
──────────────────────────────────────────────────
✅ Chai served successfully (exit code 0). Aur chahiye?
```

### Manual Build (Without the Orchestrator)

```bash
# Step 1: Build the compiler
mkdir build && cd build
cmake ..
cmake --build .

# Step 2: Compile a .chai file to C
./chaiscript ../examples/hello_chai.chai -o hello.c

# Step 3: Compile the C output to a binary
gcc -O3 hello.c -o hello

# Step 4: Run it
./hello
```

### Run Tests

```bash
python3 tests/run_tests.py
```

---

## 🧠 Architecture

```
.chai source
    │
    ▼
┌─────────────────────────┐
│  Lexer (src/Lexer.cpp)  │  Zero-copy std::string_view tokenization
│  line:col tracking      │  with keyword resolution
└────────────┬────────────┘
             │ Token stream
             ▼
┌─────────────────────────┐
│  Parser (src/Parser.cpp)│  Recursive descent + Pratt precedence
│  Panic-mode recovery    │  climbing for binary expressions
└────────────┬────────────┘
             │ Abstract Syntax Tree (std::variant nodes)
             ▼
┌─────────────────────────┐
│  Sema (src/Sema.cpp)    │  Scoped hash-table symbol resolution
│  Static type checking   │  with O(1) lookups and shadowing
└────────────┬────────────┘
             │ Verified AST
             ▼
┌─────────────────────────┐
│  CodeGen (CodeGen.cpp)  │  AST → readable C transpilation
│  printf format inference│  with proper type mapping
└────────────┬────────────┘
             │ .c file
             ▼
┌─────────────────────────┐
│  GCC / Clang / MSVC     │  Native machine code generation
└─────────────────────────┘
```

### Design Highlights

- **Zero-Copy Lexing:** Tokens are `std::string_view` slices into the source buffer — zero heap allocations during the entire scanning phase.
- **Variant-Based AST:** All AST nodes use `std::variant` instead of inheritance hierarchies. Pattern matching via `std::visit` is type-safe and keeps nodes on the stack.
- **Pratt Parser:** Operator precedence is resolved via a flat precedence-climbing loop instead of deeply nested recursive functions, yielding O(N) expression parsing.
- **Scoped Symbol Table:** Semantic analysis uses a stack of `std::unordered_map` for O(1) variable lookups with proper lexical scoping and variable shadowing.
- **Panic Mode Recovery:** Syntax errors don't crash the compiler. The parser synchronizes to the next statement boundary and continues, reporting multiple errors in a single pass.

### Error Diagnostics

```
Syntax Error [5:1]: Cup is leaking! Missing semicolon at line 5.
Semantic Error: Ingredient 'x' not found in the kitchen.
Type Error: Cannot mix adrak and chini here.
```

---

## 📚 Compiler Design Concepts Mapped

This project implements the core phases of compiler construction as taught in standard Compiler Design courses (Aho, Ullman — "The Dragon Book"). Here's how each textbook concept maps to the codebase:

| Textbook Concept              | ChaiScript Implementation                | File                  |
|-------------------------------|-------------------------------------------|-----------------------|
| **Lexical Analysis**          | Zero-copy scanner with `string_view`      | `src/Lexer.cpp`       |
| **Token Specification**       | Enum-based token types, keyword table     | `include/chai/Token.hpp` |
| **Finite Automata**           | Character-by-character state transitions  | `src/Lexer.cpp`       |
| **Context-Free Grammar**      | Recursive descent parsing rules           | `src/Parser.cpp`      |
| **Operator Precedence**       | Pratt parsing / precedence climbing       | `src/Parser.cpp`      |
| **Abstract Syntax Trees**     | `std::variant`-based tagged union nodes   | `include/chai/AST.hpp`|
| **Syntax-Directed Translation**| Visitor pattern over AST variants        | `src/CodeGen.cpp`     |
| **Symbol Tables**             | Stack of hash maps for scoped resolution  | `src/Sema.cpp`        |
| **Type Checking**             | Static analysis pass over the AST         | `src/Sema.cpp`        |
| **Error Recovery**            | Panic-mode with statement synchronization | `src/Parser.cpp`      |
| **Code Generation**           | AST → C transpilation with type mapping   | `src/CodeGen.cpp`     |
| **Intermediate Representation**| Generated C code acts as IR              | Output `.c` files     |

---

## ✍️ Writing Your First ChaiScript Program

Create a file called `my_program.chai`:

```javascript
garam_kar {
    // Declare variables with chai + type + name
    chai adrak name = "World";
    chai chini year = 2025;

    // Print values
    serve("Hello from ChaiScript!");
    serve(name);
    serve(year);

    // Conditionals
    agar (year > 2024) {
        serve("Welcome to the future!");
    }

    // Loops
    chai chini i = 3;
    ubalo_jab_tak (i > 0) {
        serve(i);
        i = i - 1;
    }

    serve("Chai time is over!");
    peelo 0;
}
```

Compile and run:
```bash
python3 tools/brew_chai.py my_program.chai
```

---

## 📁 Repository Structure

```
chaiscript-lang/
├── CMakeLists.txt              # Cross-platform CMake build (macOS/Linux/Windows)
├── README.md
├── LICENSE                     # MIT License
├── CONTRIBUTING.md             # Contribution guidelines
├── include/chai/
│   ├── Token.hpp               # Token types and debug names
│   ├── AST.hpp                 # std::variant AST with Visitor pattern
│   ├── ErrorReporter.hpp       # Panic-mode error collection
│   ├── Lexer.hpp               # Zero-copy scanner interface
│   ├── Parser.hpp              # Recursive descent + Pratt parser
│   ├── Sema.hpp                # Scoped semantic analyzer
│   └── CodeGen.hpp             # C transpilation backend
├── src/
│   ├── Lexer.cpp               # Character-by-character tokenization
│   ├── Parser.cpp              # Full Pratt parsing implementation
│   ├── Sema.cpp                # Type checking + scope resolution
│   ├── CodeGen.cpp             # AST → C code emission
│   └── main.cpp                # CLI driver (4-pass pipeline)
├── examples/
│   ├── hello_chai.chai         # Hello world
│   ├── fibonacci.chai          # Iterative Fibonacci
│   ├── calculator.chai         # Operator precedence demo
│   ├── loops_demo.chai         # Loops and factorial
│   ├── scope_demo.chai         # Lexical scoping and shadowing
│   └── error_demo.chai         # Error diagnostics demo
├── tests/
│   ├── run_tests.py            # Automated test suite
│   └── cases/                  # Test files with expected outputs
└── tools/
    └── brew_chai.py            # Cross-platform build/run orchestrator
```

---

*Hand-crafted by **Prathamraj Giri**. No parser generators were harmed in the making of this compiler.*

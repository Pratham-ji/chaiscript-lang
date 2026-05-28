# ☕ ChaiScript

**A statically-typed, compiled programming language with Indian tea-themed syntax — hand-built in modern C++20.**

[![C++ Standard](https://img.shields.io/badge/C++-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Build System](https://img.shields.io/badge/Build-CMake%20+%20Ninja-green.svg)](https://cmake.org/)
[![Architecture](https://img.shields.io/badge/Backend-C%20Transpilation-orange.svg)](src/CodeGen.cpp)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

ChaiScript takes a culturally flavored syntax inspired by Indian chai culture and compiles it into optimized C code, which is then natively assembled via GCC. Under the hood, it features a **zero-copy lexer**, a **Pratt parser** for expression precedence, **scoped semantic analysis** with static type checking, and a clean **C code generator** — all engineered from scratch without any parser generators or third-party compiler libraries.

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
5
Bahut chai pi li!
3
2
1
```

---

## 📖 Language Reference

| ChaiScript         | Meaning          | C Equivalent         |
|---------------------|------------------|----------------------|
| `garam_kar { }`     | Program entry    | `int main() { }`    |
| `peelo <expr>;`     | Return           | `return <expr>;`     |
| `chai`              | Declare variable | *(declaration)*      |
| `chini`             | Integer type     | `int`                |
| `adrak`             | String type      | `const char*`        |
| `elaichi`           | Boolean type     | `bool`               |
| `kadak`             | True             | `true`               |
| `fika`              | False            | `false`              |
| `serve(x);`         | Print to stdout  | `printf(..., x);`    |
| `agar (...) { }`    | If               | `if (...) { }`       |
| `warna { }`         | Else             | `else { }`           |
| `ubalo_jab_tak (...)` | While loop     | `while (...)`        |
| `// ek cup chai`    | Comment          | `// comment`         |

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

| Tool    | Minimum Version | Install (macOS)          |
|---------|----------------|--------------------------|
| CMake   | 3.20           | `brew install cmake`     |
| Ninja   | 1.10           | `brew install ninja`     |
| GCC     | 11+            | `brew install gcc`       |
| Python  | 3.8+           | *(pre-installed)*        |

### Build & Run

```bash
git clone https://github.com/your-username/chaiscript-lang.git
cd chaiscript-lang
chmod +x tools/brew_chai.py
python3 tools/brew_chai.py examples/hello_chai.chai
```

The orchestrator handles everything automatically:

```
☕ [1/4] Brewing the ChaiScript Compiler (C++20)...
🍵 [2/4] Steeping hello_chai.chai → C...
⚙️  [3/4] Compiling generated C with gcc -O3...
🚀 [4/4] Serving fresh chai:
────────────────────────────────────────
Namaste, duniya!
5
Bahut chai pi li!
3
2
1
────────────────────────────────────────
✅ Chai served successfully. Aur chahiye?
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
│  GCC -O3                │  Native machine code generation
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

## 📁 Repository Structure

```
chaiscript-lang/
├── CMakeLists.txt              # Modern CMake with strict warnings
├── README.md
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
│   └── hello_chai.chai         # Example program
└── tools/
    └── brew_chai.py            # Build/run orchestrator
```

---

*Hand-crafted by **Prathamraj Giri**. No parser generators were harmed in the making of this compiler.*

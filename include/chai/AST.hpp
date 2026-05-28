#pragma once

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace chai {
namespace ast {

// ── Expression Leaf Nodes ──────────────────────────────────────────────────────
struct NumberLiteral { int value; };
struct StringLiteral { std::string value; };
struct BoolLiteral   { bool value; };
struct VariableExpr  { std::string name; };

// Forward-declare recursive expression wrappers.
// Using shared_ptr breaks the infinite-size variant cycle.
struct BinaryExpr;
struct UnaryExpr;
struct AssignExpr;

using Expr = std::variant<
    NumberLiteral,
    StringLiteral,
    BoolLiteral,
    VariableExpr,
    std::shared_ptr<BinaryExpr>,
    std::shared_ptr<UnaryExpr>,
    std::shared_ptr<AssignExpr>
>;

struct BinaryExpr {
    Expr left;
    std::string op;
    Expr right;
};

struct UnaryExpr {
    std::string op;
    Expr operand;
};

struct AssignExpr {
    std::string name;
    Expr value;
};

// ── Statement Leaf Nodes ──────────────────────────────────────────────────────
struct ExprStmt    { Expr expression; };
struct ServeStmt   { Expr expression; };  // maps to printf
struct ReturnStmt  { std::optional<Expr> value; };

struct VarDeclStmt {
    std::string type;    // "chini", "adrak", "elaichi"
    std::string name;
    std::optional<Expr> initializer;
};

// Forward-declare recursive statement wrappers
struct BlockStmt;
struct IfStmt;
struct WhileStmt;

using Stmt = std::variant<
    ExprStmt,
    VarDeclStmt,
    ServeStmt,
    ReturnStmt,
    std::shared_ptr<IfStmt>,
    std::shared_ptr<WhileStmt>,
    std::shared_ptr<BlockStmt>
>;

struct BlockStmt {
    std::vector<Stmt> statements;
};

struct IfStmt {
    Expr condition;
    std::shared_ptr<BlockStmt> thenBranch;
    std::shared_ptr<BlockStmt> elseBranch;  // nullptr when no warna
};

struct WhileStmt {
    Expr condition;
    std::shared_ptr<BlockStmt> body;
};

// Top-level translation unit
struct Program {
    std::vector<Stmt> statements;
};

} // namespace ast
} // namespace chai

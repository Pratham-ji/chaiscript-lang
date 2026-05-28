#pragma once

#include "chai/AST.hpp"
#include "chai/ErrorReporter.hpp"
#include "chai/Token.hpp"
#include <vector>

namespace chai {

// Recursive-descent parser with Pratt precedence climbing for expressions.
class Parser {
public:
    Parser(std::vector<Token> tokens, ErrorReporter& reporter);
    ast::Program parse();

private:
    std::vector<Token> tokens_;
    ErrorReporter& reporter_;
    size_t current_ = 0;

    // Pratt precedence levels (higher = tighter binding)
    enum Precedence {
        NONE       = 0,
        ASSIGNMENT = 1,   // =
        EQUALITY   = 2,   // == !=
        COMPARISON = 3,   // < > <= >=
        TERM       = 4,   // + -
        FACTOR     = 5,   // * /
        UNARY      = 6,   // ! -
        PRIMARY    = 7
    };

    // ── Token navigation ──
    const Token& peek() const;
    const Token& previous() const;
    const Token& advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    Token consume(TokenType type, const std::string& message);
    bool isAtEnd() const;

    // ── Panic-mode recovery ──
    void synchronize();

    // ── Statement rules ──
    ast::Stmt declaration();
    ast::Stmt statement();
    ast::VarDeclStmt varDeclaration();
    ast::ServeStmt serveStatement();
    ast::ReturnStmt returnStatement();
    std::shared_ptr<ast::IfStmt> ifStatement();
    std::shared_ptr<ast::WhileStmt> whileStatement();
    std::shared_ptr<ast::BlockStmt> block();

    // ── Expression rules (Pratt) ──
    ast::Expr expression();
    ast::Expr parsePrecedence(Precedence minPrec);
    ast::Expr prefixRule(TokenType type);
    ast::Expr infixRule(TokenType type, ast::Expr left);
    Precedence getPrecedence(TokenType type) const;
};

} // namespace chai

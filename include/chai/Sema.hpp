#pragma once

#include "chai/AST.hpp"
#include "chai/ErrorReporter.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace chai {

// Semantic analysis pass: scope resolution + static type checking.
// Uses a stack of hash maps for O(1) scoped lookups with proper shadowing.
class Sema {
public:
    explicit Sema(ErrorReporter& reporter);
    bool analyze(const ast::Program& program);

private:
    ErrorReporter& reporter_;

    // Scope stack: each frame maps variable name -> chai type string
    std::vector<std::unordered_map<std::string, std::string>> scopes_;

    void pushScope();
    void popScope();
    void declare(const std::string& name, const std::string& type);
    std::string lookup(const std::string& name) const;

    void visit(const ast::Stmt& stmt);
    std::string visitExpr(const ast::Expr& expr);

    void visitExprStmt(const ast::ExprStmt& s);
    void visitVarDecl(const ast::VarDeclStmt& s);
    void visitServe(const ast::ServeStmt& s);
    void visitReturn(const ast::ReturnStmt& s);
    void visitIf(const ast::IfStmt& s);
    void visitWhile(const ast::WhileStmt& s);
    void visitBlock(const ast::BlockStmt& s);
};

} // namespace chai

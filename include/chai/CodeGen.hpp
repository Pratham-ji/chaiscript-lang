#pragma once

#include "chai/AST.hpp"
#include <sstream>
#include <string>
#include <unordered_map>

namespace chai {

// Transpilation backend: walks the verified AST and emits clean, readable C.
class CodeGen {
public:
    std::string generate(const ast::Program& program);

private:
    std::stringstream out_;
    int indent_ = 1;   // inside main() by default
    std::unordered_map<std::string, std::string> varTypes_;

    void writeIndent();

    void emit(const ast::Stmt& stmt);
    std::string emitExpr(const ast::Expr& expr);

    void emitExprStmt(const ast::ExprStmt& s);
    void emitVarDecl(const ast::VarDeclStmt& s);
    void emitServe(const ast::ServeStmt& s);
    void emitReturn(const ast::ReturnStmt& s);
    void emitIf(const ast::IfStmt& s);
    void emitWhile(const ast::WhileStmt& s);
    void emitBlock(const ast::BlockStmt& s);

    std::string mapType(const std::string& chaiType) const;
    std::string formatSpecifier(const std::string& chaiType) const;
};

} // namespace chai

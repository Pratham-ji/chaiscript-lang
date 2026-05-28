#include "chai/Sema.hpp"

namespace chai {

Sema::Sema(ErrorReporter& reporter) : reporter_(reporter) {}

bool Sema::analyze(const ast::Program& program) {
    pushScope();  // top-level garam_kar scope
    for (const auto& stmt : program.statements) {
        visit(stmt);
    }
    popScope();
    return !reporter_.hadError();
}

void Sema::pushScope() { scopes_.emplace_back(); }
void Sema::popScope()  { scopes_.pop_back(); }

void Sema::declare(const std::string& name, const std::string& type) {
    if (scopes_.empty()) return;
    auto& current = scopes_.back();
    if (current.find(name) != current.end()) {
        reporter_.semanticError(0, 0, "Ingredient '" + name + "' is already in this batch.");
    }
    current[name] = type;
}

std::string Sema::lookup(const std::string& name) const {
    // Walk from innermost scope outward — handles shadowing naturally
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) return found->second;
    }
    return "";  // not found
}

// ── Statement Dispatch ──────────────────────────────────────────────────────

void Sema::visit(const ast::Stmt& stmt) {
    std::visit([this](const auto& s) {
        using T = std::decay_t<decltype(s)>;
        if constexpr (std::is_same_v<T, ast::ExprStmt>)                        visitExprStmt(s);
        else if constexpr (std::is_same_v<T, ast::VarDeclStmt>)                visitVarDecl(s);
        else if constexpr (std::is_same_v<T, ast::ServeStmt>)                  visitServe(s);
        else if constexpr (std::is_same_v<T, ast::ReturnStmt>)                 visitReturn(s);
        else if constexpr (std::is_same_v<T, std::shared_ptr<ast::IfStmt>>)    visitIf(*s);
        else if constexpr (std::is_same_v<T, std::shared_ptr<ast::WhileStmt>>) visitWhile(*s);
        else if constexpr (std::is_same_v<T, std::shared_ptr<ast::BlockStmt>>) visitBlock(*s);
    }, stmt);
}

void Sema::visitExprStmt(const ast::ExprStmt& s) {
    visitExpr(s.expression);
}

void Sema::visitVarDecl(const ast::VarDeclStmt& s) {
    declare(s.name, s.type);
    if (s.initializer) {
        std::string rhs = visitExpr(s.initializer.value());
        if (!rhs.empty() && rhs != s.type) {
            reporter_.typeError(0, 0, "Cannot mix " + s.type + " and " + rhs + " here.");
        }
    }
}

void Sema::visitServe(const ast::ServeStmt& s) {
    visitExpr(s.expression);
}

void Sema::visitReturn(const ast::ReturnStmt& s) {
    if (s.value) visitExpr(s.value.value());
}

void Sema::visitIf(const ast::IfStmt& s) {
    visitExpr(s.condition);
    pushScope();
    for (const auto& st : s.thenBranch->statements) visit(st);
    popScope();
    if (s.elseBranch) {
        pushScope();
        for (const auto& st : s.elseBranch->statements) visit(st);
        popScope();
    }
}

void Sema::visitWhile(const ast::WhileStmt& s) {
    visitExpr(s.condition);
    pushScope();
    for (const auto& st : s.body->statements) visit(st);
    popScope();
}

void Sema::visitBlock(const ast::BlockStmt& s) {
    pushScope();
    for (const auto& st : s.statements) visit(st);
    popScope();
}

// ── Expression Type Inference ───────────────────────────────────────────────

std::string Sema::visitExpr(const ast::Expr& expr) {
    return std::visit([this](const auto& e) -> std::string {
        using T = std::decay_t<decltype(e)>;

        if constexpr (std::is_same_v<T, ast::NumberLiteral>)  return "chini";
        else if constexpr (std::is_same_v<T, ast::StringLiteral>) return "adrak";
        else if constexpr (std::is_same_v<T, ast::BoolLiteral>)  return "elaichi";

        else if constexpr (std::is_same_v<T, ast::VariableExpr>) {
            std::string type = lookup(e.name);
            if (type.empty()) {
                reporter_.semanticError(0, 0,
                    "Ingredient '" + e.name + "' not found in the kitchen.");
            }
            return type;
        }

        else if constexpr (std::is_same_v<T, std::shared_ptr<ast::BinaryExpr>>) {
            std::string lhs = visitExpr(e->left);
            std::string rhs = visitExpr(e->right);
            if (lhs.empty() || rhs.empty()) return "";

            if (e->op == "+" || e->op == "-" || e->op == "*" || e->op == "/") {
                if (lhs != "chini" || rhs != "chini") {
                    reporter_.typeError(0, 0,
                        "Cannot mix " + lhs + " and " + rhs + " here.");
                    return "";
                }
                return "chini";
            }
            if (e->op == "==" || e->op == "!=" || e->op == "<" || e->op == ">" ||
                e->op == "<=" || e->op == ">=") {
                if (lhs != rhs) {
                    reporter_.typeError(0, 0,
                        "Cannot compare " + lhs + " with " + rhs + ".");
                }
                return "elaichi";
            }
            return lhs;
        }

        else if constexpr (std::is_same_v<T, std::shared_ptr<ast::UnaryExpr>>) {
            return visitExpr(e->operand);
        }

        else if constexpr (std::is_same_v<T, std::shared_ptr<ast::AssignExpr>>) {
            std::string varType = lookup(e->name);
            if (varType.empty()) {
                reporter_.semanticError(0, 0,
                    "Ingredient '" + e->name + "' not found in the kitchen.");
                return "";
            }
            std::string rhsType = visitExpr(e->value);
            if (!rhsType.empty() && rhsType != varType) {
                reporter_.typeError(0, 0,
                    "Cannot mix " + varType + " and " + rhsType + " here.");
            }
            return varType;
        }

        else return "";
    }, expr);
}

} // namespace chai

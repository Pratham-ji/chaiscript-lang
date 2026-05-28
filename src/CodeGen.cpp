#include "chai/CodeGen.hpp"

namespace chai {

std::string CodeGen::generate(const ast::Program& program) {
    out_ << "#include <stdio.h>\n";
    out_ << "#include <stdbool.h>\n\n";
    out_ << "int main() {\n";

    for (const auto& stmt : program.statements) {
        emit(stmt);
    }

    out_ << "}\n";
    return out_.str();
}

void CodeGen::writeIndent() {
    for (int i = 0; i < indent_; ++i) out_ << "    ";
}

std::string CodeGen::mapType(const std::string& chaiType) const {
    if (chaiType == "chini")   return "int";
    if (chaiType == "adrak")   return "const char*";
    if (chaiType == "elaichi") return "bool";
    if (chaiType == "paani")   return "void";
    return "int";
}

std::string CodeGen::formatSpecifier(const std::string& chaiType) const {
    if (chaiType == "chini")   return "%d";
    if (chaiType == "adrak")   return "%s";
    if (chaiType == "elaichi") return "%d";
    return "%d";
}

// ── Statement Dispatch ──────────────────────────────────────────────────────

void CodeGen::emit(const ast::Stmt& stmt) {
    std::visit([this](const auto& s) {
        using T = std::decay_t<decltype(s)>;
        if constexpr (std::is_same_v<T, ast::ExprStmt>)                        emitExprStmt(s);
        else if constexpr (std::is_same_v<T, ast::VarDeclStmt>)                emitVarDecl(s);
        else if constexpr (std::is_same_v<T, ast::ServeStmt>)                  emitServe(s);
        else if constexpr (std::is_same_v<T, ast::ReturnStmt>)                 emitReturn(s);
        else if constexpr (std::is_same_v<T, std::shared_ptr<ast::IfStmt>>)    emitIf(*s);
        else if constexpr (std::is_same_v<T, std::shared_ptr<ast::WhileStmt>>) emitWhile(*s);
        else if constexpr (std::is_same_v<T, std::shared_ptr<ast::BlockStmt>>) emitBlock(*s);
    }, stmt);
}

void CodeGen::emitExprStmt(const ast::ExprStmt& s) {
    writeIndent();
    out_ << emitExpr(s.expression) << ";\n";
}

void CodeGen::emitVarDecl(const ast::VarDeclStmt& s) {
    varTypes_[s.name] = s.type;
    writeIndent();
    out_ << mapType(s.type) << " " << s.name;
    if (s.initializer) {
        out_ << " = " << emitExpr(s.initializer.value());
    }
    out_ << ";\n";
}

void CodeGen::emitServe(const ast::ServeStmt& s) {
    writeIndent();
    std::string exprStr = emitExpr(s.expression);

    // Infer the printf format specifier from the expression's compile-time type
    std::string fmt = "%d";
    std::visit([&](const auto& e) {
        using T = std::decay_t<decltype(e)>;
        if constexpr (std::is_same_v<T, ast::StringLiteral>) {
            fmt = "%s";
        } else if constexpr (std::is_same_v<T, ast::VariableExpr>) {
            auto it = varTypes_.find(e.name);
            if (it != varTypes_.end()) fmt = formatSpecifier(it->second);
        }
    }, s.expression);

    out_ << "printf(\"" << fmt << "\\n\", " << exprStr << ");\n";
}

void CodeGen::emitReturn(const ast::ReturnStmt& s) {
    writeIndent();
    out_ << "return";
    if (s.value) {
        out_ << " " << emitExpr(s.value.value());
    }
    out_ << ";\n";
}

void CodeGen::emitIf(const ast::IfStmt& s) {
    writeIndent();
    out_ << "if (" << emitExpr(s.condition) << ") {\n";
    indent_++;
    for (const auto& st : s.thenBranch->statements) emit(st);
    indent_--;
    writeIndent();
    out_ << "}";
    if (s.elseBranch) {
        out_ << " else {\n";
        indent_++;
        for (const auto& st : s.elseBranch->statements) emit(st);
        indent_--;
        writeIndent();
        out_ << "}";
    }
    out_ << "\n";
}

void CodeGen::emitWhile(const ast::WhileStmt& s) {
    writeIndent();
    out_ << "while (" << emitExpr(s.condition) << ") {\n";
    indent_++;
    for (const auto& st : s.body->statements) emit(st);
    indent_--;
    writeIndent();
    out_ << "}\n";
}

void CodeGen::emitBlock(const ast::BlockStmt& s) {
    writeIndent();
    out_ << "{\n";
    indent_++;
    for (const auto& st : s.statements) emit(st);
    indent_--;
    writeIndent();
    out_ << "}\n";
}

// ── Expression Emitter ──────────────────────────────────────────────────────

std::string CodeGen::emitExpr(const ast::Expr& expr) {
    return std::visit([this](const auto& e) -> std::string {
        using T = std::decay_t<decltype(e)>;

        if constexpr (std::is_same_v<T, ast::NumberLiteral>) {
            return std::to_string(e.value);
        }
        else if constexpr (std::is_same_v<T, ast::StringLiteral>) {
            return "\"" + e.value + "\"";
        }
        else if constexpr (std::is_same_v<T, ast::BoolLiteral>) {
            return e.value ? "true" : "false";
        }
        else if constexpr (std::is_same_v<T, ast::VariableExpr>) {
            return e.name;
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<ast::BinaryExpr>>) {
            return "(" + emitExpr(e->left) + " " + e->op + " " + emitExpr(e->right) + ")";
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<ast::UnaryExpr>>) {
            return "(" + e->op + emitExpr(e->operand) + ")";
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<ast::AssignExpr>>) {
            return e->name + " = " + emitExpr(e->value);
        }
        else return "";
    }, expr);
}

} // namespace chai

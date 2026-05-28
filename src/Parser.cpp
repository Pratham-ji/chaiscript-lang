#include "chai/Parser.hpp"
#include <stdexcept>

namespace chai {

Parser::Parser(std::vector<Token> tokens, ErrorReporter& reporter)
    : tokens_(std::move(tokens)), reporter_(reporter) {}

// ── Token Navigation ─────────────────────────────────────────────────────────

const Token& Parser::peek() const     { return tokens_[current_]; }
const Token& Parser::previous() const { return tokens_[current_ - 1]; }
bool Parser::isAtEnd() const          { return peek().type == TokenType::Eof; }
bool Parser::check(TokenType type) const { return !isAtEnd() && peek().type == type; }

const Token& Parser::advance() {
    if (!isAtEnd()) current_++;
    return previous();
}

bool Parser::match(TokenType type) {
    if (check(type)) { advance(); return true; }
    return false;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    reporter_.syntaxError(peek().line, peek().col, message);
    throw std::runtime_error(message);
}

// ── Panic-Mode Error Recovery ────────────────────────────────────────────────
// Skips tokens until we land on a likely statement boundary, allowing the
// compiler to report multiple errors in a single pass instead of bailing out.

void Parser::synchronize() {
    advance();
    while (!isAtEnd()) {
        if (previous().type == TokenType::Semicolon) return;
        switch (peek().type) {
            case TokenType::Chai:
            case TokenType::Serve:
            case TokenType::Agar:
            case TokenType::UbaloJabTak:
            case TokenType::Peelo:
            case TokenType::GaramKar:
                return;
            default:
                advance();
        }
    }
}

// ── Program Entry ────────────────────────────────────────────────────────────

ast::Program Parser::parse() {
    ast::Program program;

    consume(TokenType::GaramKar, "Every chai program must begin with 'garam_kar'.");
    consume(TokenType::LeftBrace, "Expected '{' after garam_kar.");

    while (!check(TokenType::RightBrace) && !isAtEnd()) {
        try {
            program.statements.push_back(declaration());
        } catch (const std::runtime_error&) {
            synchronize();
        }
    }

    consume(TokenType::RightBrace, "Expected '}' to close garam_kar block.");
    return program;
}

// ── Statement Parsing ────────────────────────────────────────────────────────

ast::Stmt Parser::declaration() {
    if (check(TokenType::Chai)) return varDeclaration();
    return statement();
}

ast::Stmt Parser::statement() {
    if (match(TokenType::Serve))        return serveStatement();
    if (match(TokenType::Peelo))        return returnStatement();
    if (check(TokenType::Agar))         { advance(); return ifStatement(); }
    if (check(TokenType::UbaloJabTak))  { advance(); return whileStatement(); }
    if (match(TokenType::LeftBrace))    return block();

    // Fall through: expression statement
    ast::Expr expr = expression();
    consume(TokenType::Semicolon,
            "Cup is leaking! Missing semicolon at line " + std::to_string(previous().line) + ".");
    return ast::ExprStmt{expr};
}

ast::VarDeclStmt Parser::varDeclaration() {
    advance(); // consume 'chai'

    std::string type;
    if (match(TokenType::Chini))        type = "chini";
    else if (match(TokenType::Adrak))   type = "adrak";
    else if (match(TokenType::Elaichi)) type = "elaichi";
    else {
        reporter_.syntaxError(peek().line, peek().col,
                              "Expected type (chini/adrak/elaichi) after 'chai'.");
        throw std::runtime_error("Bad type");
    }

    Token name = consume(TokenType::Identifier, "Expected variable name after type.");
    std::string varName(name.lexeme);

    std::optional<ast::Expr> init;
    if (match(TokenType::Equal)) {
        init = expression();
    }

    consume(TokenType::Semicolon,
            "Cup is leaking! Missing semicolon at line " + std::to_string(previous().line) + ".");
    return ast::VarDeclStmt{type, varName, init};
}

ast::ServeStmt Parser::serveStatement() {
    consume(TokenType::LeftParen, "Expected '(' after serve.");
    ast::Expr expr = expression();
    consume(TokenType::RightParen, "Expected ')' after serve expression.");
    consume(TokenType::Semicolon,
            "Cup is leaking! Missing semicolon at line " + std::to_string(previous().line) + ".");
    return ast::ServeStmt{expr};
}

ast::ReturnStmt Parser::returnStatement() {
    std::optional<ast::Expr> value;
    if (!check(TokenType::Semicolon)) {
        value = expression();
    }
    consume(TokenType::Semicolon,
            "Cup is leaking! Missing semicolon at line " + std::to_string(previous().line) + ".");
    return ast::ReturnStmt{value};
}

std::shared_ptr<ast::IfStmt> Parser::ifStatement() {
    consume(TokenType::LeftParen, "Expected '(' after agar.");
    ast::Expr condition = expression();
    consume(TokenType::RightParen, "Expected ')' after agar condition.");
    consume(TokenType::LeftBrace, "Expected '{' for agar body.");

    auto thenBranch = block();

    std::shared_ptr<ast::BlockStmt> elseBranch = nullptr;
    if (match(TokenType::Warna)) {
        consume(TokenType::LeftBrace, "Expected '{' after warna.");
        elseBranch = block();
    }

    return std::make_shared<ast::IfStmt>(ast::IfStmt{condition, thenBranch, elseBranch});
}

std::shared_ptr<ast::WhileStmt> Parser::whileStatement() {
    consume(TokenType::LeftParen, "Expected '(' after ubalo_jab_tak.");
    ast::Expr condition = expression();
    consume(TokenType::RightParen, "Expected ')' after loop condition.");
    consume(TokenType::LeftBrace, "Expected '{' for loop body.");

    auto body = block();
    return std::make_shared<ast::WhileStmt>(ast::WhileStmt{condition, body});
}

std::shared_ptr<ast::BlockStmt> Parser::block() {
    // Opening '{' was already consumed by the caller
    auto blk = std::make_shared<ast::BlockStmt>();
    while (!check(TokenType::RightBrace) && !isAtEnd()) {
        try {
            blk->statements.push_back(declaration());
        } catch (const std::runtime_error&) {
            synchronize();
        }
    }
    consume(TokenType::RightBrace, "Expected '}' to close block.");
    return blk;
}

// ── Expression Parsing: Pratt / Precedence Climbing ──────────────────────────
//
// Instead of writing deeply nested recursive functions (expr -> term -> factor),
// Pratt parsing assigns an integer "binding power" to each operator and uses a
// single loop that climbs from low to high precedence. This resolves * before +
// in O(N) flat iteration.

ast::Expr Parser::expression() {
    return parsePrecedence(ASSIGNMENT);
}

ast::Expr Parser::parsePrecedence(Precedence minPrec) {
    advance();
    ast::Expr left = prefixRule(previous().type);

    while (getPrecedence(peek().type) >= minPrec) {
        advance();
        left = infixRule(previous().type, std::move(left));
    }

    return left;
}

ast::Expr Parser::prefixRule(TokenType type) {
    switch (type) {
        case TokenType::Number: {
            int val = std::stoi(std::string(previous().lexeme));
            return ast::NumberLiteral{val};
        }
        case TokenType::String: {
            auto raw = previous().lexeme;
            std::string val(raw.substr(1, raw.size() - 2));  // strip quotes
            return ast::StringLiteral{val};
        }
        case TokenType::Kadak:
            return ast::BoolLiteral{true};
        case TokenType::Fika:
            return ast::BoolLiteral{false};
        case TokenType::Identifier: {
            std::string name(previous().lexeme);
            if (match(TokenType::Equal)) {
                ast::Expr value = expression();
                return std::make_shared<ast::AssignExpr>(ast::AssignExpr{name, value});
            }
            return ast::VariableExpr{name};
        }
        case TokenType::LeftParen: {
            ast::Expr expr = expression();
            consume(TokenType::RightParen, "Expected ')' after grouped expression.");
            return expr;
        }
        case TokenType::Minus:
        case TokenType::Bang: {
            std::string op(previous().lexeme);
            ast::Expr operand = parsePrecedence(UNARY);
            return std::make_shared<ast::UnaryExpr>(ast::UnaryExpr{op, operand});
        }
        default:
            reporter_.syntaxError(previous().line, previous().col,
                "Unexpected token '" + std::string(previous().lexeme) + "'. Expected an expression.");
            throw std::runtime_error("Bad prefix");
    }
}

ast::Expr Parser::infixRule(TokenType /*type*/, ast::Expr left) {
    std::string op(previous().lexeme);
    Precedence nextPrec = static_cast<Precedence>(getPrecedence(previous().type) + 1);
    ast::Expr right = parsePrecedence(nextPrec);
    return std::make_shared<ast::BinaryExpr>(ast::BinaryExpr{std::move(left), op, std::move(right)});
}

Parser::Precedence Parser::getPrecedence(TokenType type) const {
    switch (type) {
        case TokenType::EqualEqual:
        case TokenType::BangEqual:    return EQUALITY;
        case TokenType::Less:
        case TokenType::Greater:
        case TokenType::LessEqual:
        case TokenType::GreaterEqual: return COMPARISON;
        case TokenType::Plus:
        case TokenType::Minus:        return TERM;
        case TokenType::Star:
        case TokenType::Slash:        return FACTOR;
        default:                      return NONE;
    }
}

} // namespace chai

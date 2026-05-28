#pragma once

#include "chai/Token.hpp"
#include <string>
#include <string_view>
#include <vector>

namespace chai {

// Zero-copy lexer that produces string_view tokens into the source buffer.
class Lexer {
public:
    explicit Lexer(std::string source);
    std::vector<Token> tokenize();

private:
    std::string source_;       // Owns the source text
    std::string_view view_;    // Non-owning window for zero-copy slicing
    size_t start_    = 0;
    size_t current_  = 0;
    int line_        = 1;
    int col_         = 1;
    int startCol_    = 1;

    bool isAtEnd() const;
    char advance();
    char peek() const;
    char peekNext() const;
    bool match(char expected);

    Token makeToken(TokenType type) const;
    Token errorToken(const std::string& message) const;

    void skipWhitespace();
    Token scanToken();
    Token number();
    Token string();
    Token identifier();
    TokenType identifierType(std::string_view text) const;
};

} // namespace chai

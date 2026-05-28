#include "chai/Lexer.hpp"
#include <cctype>

namespace chai {

Lexer::Lexer(std::string source)
    : source_(std::move(source)), view_(source_) {}

bool Lexer::isAtEnd() const { return current_ >= view_.size(); }

char Lexer::advance() {
    char c = view_[current_++];
    col_++;
    return c;
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return view_[current_];
}

char Lexer::peekNext() const {
    if (current_ + 1 >= view_.size()) return '\0';
    return view_[current_ + 1];
}

bool Lexer::match(char expected) {
    if (isAtEnd() || view_[current_] != expected) return false;
    current_++;
    col_++;
    return true;
}

Token Lexer::makeToken(TokenType type) const {
    return Token{type, view_.substr(start_, current_ - start_), line_, startCol_};
}

Token Lexer::errorToken(const std::string& message) const {
    // The string_view here points to the message, which must outlive the token.
    // In practice, we immediately report the error so this is safe.
    return Token{TokenType::Error, std::string_view(message), line_, startCol_};
}

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                line_++;
                col_ = 0;   // advance() will bump to 1
                advance();
                break;
            case '/':
                if (peekNext() == '/') {
                    // ek cup chai — single-line comment
                    while (!isAtEnd() && peek() != '\n') advance();
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

Token Lexer::number() {
    while (!isAtEnd() && std::isdigit(peek())) advance();
    return makeToken(TokenType::Number);
}

Token Lexer::string() {
    while (!isAtEnd() && peek() != '"') {
        if (peek() == '\n') { line_++; col_ = 0; }
        advance();
    }
    if (isAtEnd()) return errorToken("Unterminated string — cup overflowed!");
    advance(); // closing quote
    return makeToken(TokenType::String);
}

TokenType Lexer::identifierType(std::string_view text) const {
    // Keyword table — hand-rolled for zero allocation overhead
    if (text == "garam_kar")      return TokenType::GaramKar;
    if (text == "peelo")          return TokenType::Peelo;
    if (text == "chai")           return TokenType::Chai;
    if (text == "paani")          return TokenType::Paani;
    if (text == "chini")          return TokenType::Chini;
    if (text == "adrak")          return TokenType::Adrak;
    if (text == "elaichi")        return TokenType::Elaichi;
    if (text == "kadak")          return TokenType::Kadak;
    if (text == "fika")           return TokenType::Fika;
    if (text == "serve")          return TokenType::Serve;
    if (text == "agar")           return TokenType::Agar;
    if (text == "warna")          return TokenType::Warna;
    if (text == "ubalo_jab_tak")  return TokenType::UbaloJabTak;
    return TokenType::Identifier;
}

Token Lexer::identifier() {
    while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_')) advance();
    auto text = view_.substr(start_, current_ - start_);
    return makeToken(identifierType(text));
}

Token Lexer::scanToken() {
    skipWhitespace();
    start_ = current_;
    startCol_ = col_;

    if (isAtEnd()) return makeToken(TokenType::Eof);

    char c = advance();

    if (std::isdigit(c)) return number();
    if (std::isalpha(c) || c == '_') return identifier();

    switch (c) {
        case '(': return makeToken(TokenType::LeftParen);
        case ')': return makeToken(TokenType::RightParen);
        case '{': return makeToken(TokenType::LeftBrace);
        case '}': return makeToken(TokenType::RightBrace);
        case ';': return makeToken(TokenType::Semicolon);
        case ',': return makeToken(TokenType::Comma);
        case '+': return makeToken(TokenType::Plus);
        case '-': return makeToken(TokenType::Minus);
        case '*': return makeToken(TokenType::Star);
        case '/': return makeToken(TokenType::Slash);
        case '=': return makeToken(match('=') ? TokenType::EqualEqual : TokenType::Equal);
        case '!': return makeToken(match('=') ? TokenType::BangEqual  : TokenType::Bang);
        case '<': return makeToken(match('=') ? TokenType::LessEqual  : TokenType::Less);
        case '>': return makeToken(match('=') ? TokenType::GreaterEqual : TokenType::Greater);
        case '"': return string();
        default:  return errorToken("Unexpected character in the chai");
    }
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (true) {
        Token tok = scanToken();
        tokens.push_back(tok);
        if (tok.type == TokenType::Eof) break;
    }
    return tokens;
}

} // namespace chai

#pragma once

#include <string>
#include <string_view>

namespace chai {

enum class TokenType {
    // ── Literals ──
    Number, String, Identifier,

    // ── Program Structure ──
    GaramKar,       // main entry
    Peelo,          // return

    // ── Declaration ──
    Chai,           // variable declaration keyword

    // ── Types ──
    Paani,          // void
    Chini,          // int
    Adrak,          // string
    Elaichi,        // bool

    // ── Boolean Values ──
    Kadak,          // true
    Fika,           // false

    // ── IO ──
    Serve,          // print

    // ── Control Flow ──
    Agar,           // if
    Warna,          // else
    UbaloJabTak,    // while

    // ── Operators ──
    Plus, Minus, Star, Slash,
    Equal, EqualEqual,
    Bang, BangEqual,
    Less, Greater, LessEqual, GreaterEqual,

    // ── Delimiters ──
    LeftParen, RightParen,
    LeftBrace, RightBrace,
    Semicolon, Comma,

    // ── Special ──
    Eof, Error
};

struct Token {
    TokenType type;
    std::string_view lexeme;
    int line;
    int col;
};

// Diagnostic utility for pretty-printing token names
inline std::string_view tokenName(TokenType t) {
    switch (t) {
        case TokenType::Number:       return "Number";
        case TokenType::String:       return "String";
        case TokenType::Identifier:   return "Identifier";
        case TokenType::GaramKar:     return "garam_kar";
        case TokenType::Peelo:        return "peelo";
        case TokenType::Chai:         return "chai";
        case TokenType::Paani:        return "paani";
        case TokenType::Chini:        return "chini";
        case TokenType::Adrak:        return "adrak";
        case TokenType::Elaichi:      return "elaichi";
        case TokenType::Kadak:        return "kadak";
        case TokenType::Fika:         return "fika";
        case TokenType::Serve:        return "serve";
        case TokenType::Agar:         return "agar";
        case TokenType::Warna:        return "warna";
        case TokenType::UbaloJabTak:  return "ubalo_jab_tak";
        case TokenType::Plus:         return "+";
        case TokenType::Minus:        return "-";
        case TokenType::Star:         return "*";
        case TokenType::Slash:        return "/";
        case TokenType::Equal:        return "=";
        case TokenType::EqualEqual:   return "==";
        case TokenType::Bang:         return "!";
        case TokenType::BangEqual:    return "!=";
        case TokenType::Less:         return "<";
        case TokenType::Greater:      return ">";
        case TokenType::LessEqual:    return "<=";
        case TokenType::GreaterEqual: return ">=";
        case TokenType::LeftParen:    return "(";
        case TokenType::RightParen:   return ")";
        case TokenType::LeftBrace:    return "{";
        case TokenType::RightBrace:   return "}";
        case TokenType::Semicolon:    return ";";
        case TokenType::Comma:        return ",";
        case TokenType::Eof:          return "EOF";
        case TokenType::Error:        return "ERROR";
    }
    return "UNKNOWN";
}

} // namespace chai

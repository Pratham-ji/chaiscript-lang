#pragma once

#include <string>
#include <vector>
#include <iostream>

namespace chai {

// Centralized diagnostic engine with panic-mode support.
// Collects all errors without crashing the compiler, then reports them at the end.
class ErrorReporter {
public:
    void report(int line, int col, const std::string& phase, const std::string& message) {
        std::string loc = (line > 0) ? " [" + std::to_string(line) + ":" + std::to_string(col) + "]" : "";
        std::string formatted = "\033[1;31m" + phase + " Error\033[0m" + loc + ": " + message;
        errors_.push_back(formatted);
        hadError_ = true;
    }

    void syntaxError(int line, int col, const std::string& message) {
        report(line, col, "Syntax", message);
    }

    void semanticError(int line, int col, const std::string& message) {
        report(line, col, "Semantic", message);
    }

    void typeError(int line, int col, const std::string& message) {
        report(line, col, "Type", message);
    }

    [[nodiscard]] bool hadError() const { return hadError_; }

    void printAll() const {
        for (const auto& err : errors_) {
            std::cerr << err << "\n";
        }
    }

    void clear() {
        errors_.clear();
        hadError_ = false;
    }

private:
    std::vector<std::string> errors_;
    bool hadError_ = false;
};

} // namespace chai

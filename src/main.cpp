#include "chai/Lexer.hpp"
#include "chai/Parser.hpp"
#include "chai/Sema.hpp"
#include "chai/CodeGen.hpp"
#include "chai/ErrorReporter.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

int main(int argc, char* argv[]) {
    std::string inputFile;
    std::string outputFile = "out.c";

    // Minimal CLI argument handling
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-o" && i + 1 < argc) {
            outputFile = argv[++i];
        } else {
            inputFile = argv[i];
        }
    }

    if (inputFile.empty()) {
        std::cerr << "Usage: chaiscript <source.chai> [-o output.c]\n";
        return 1;
    }

    std::ifstream file(inputFile);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << inputFile << "\n";
        return 1;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    chai::ErrorReporter reporter;

    try {
        // Pass 1: Lexical Analysis (zero-copy tokenization)
        chai::Lexer lexer(source);
        auto tokens = lexer.tokenize();

        // Pass 2: Syntax Analysis (recursive descent + Pratt parsing)
        chai::Parser parser(std::move(tokens), reporter);
        auto ast = parser.parse();

        if (reporter.hadError()) {
            reporter.printAll();
            return 1;
        }

        // Pass 3: Semantic Analysis (scoped type checking)
        chai::Sema sema(reporter);
        if (!sema.analyze(ast)) {
            reporter.printAll();
            return 1;
        }

        // Pass 4: C Code Generation
        chai::CodeGen codegen;
        std::string cCode = codegen.generate(ast);

        std::ofstream out(outputFile);
        out << cCode;
        out.close();

        std::cerr << "Compiled successfully: " << outputFile << "\n";

    } catch (const std::exception& e) {
        reporter.printAll();
        std::cerr << "Compilation failed.\n";
        return 1;
    }

    return 0;
}

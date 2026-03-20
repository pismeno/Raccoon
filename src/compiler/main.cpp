#include <iostream>
#include <memory>
#include "include/Lexer.hpp"
#include "include/Parser.hpp"
#include "include/ast/Printer.hpp"
#include "include/ast/SemanticAnalyzer.hpp"
#include "include/ast/IRGenerator.hpp"
#include <fstream>
#include <llvm/Support/raw_os_ostream.h>
#include "include/CLI11.hpp"

std::string get_file_contents(const std::string& sourceFileName) {
    std::ifstream file(sourceFileName);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + sourceFileName);
    }

    return std::string((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
}

int main(int argc, char** argv) {
    using namespace raccoon::compiler;

    CLI::App raccoonCli{"Raccoon Compiler"};

    CLI::App* print = raccoonCli.add_subcommand("print", "Compile a source file and output the IR in the console.");
    CLI::App* compile = raccoonCli.add_subcommand("compile", "Compile a source file and write it inside build/");

    std::string sourceFileName;
    std::string outputFileName = "output.ll";

    print->add_option("source", sourceFileName, "Source file to compile")->required();
    compile->add_option("source", sourceFileName, "Source file to compile")->required();
    compile->add_option("-o,--output", outputFileName, "Output binary name");

    CLI11_PARSE(raccoonCli, argc, argv);

    if (*print || *compile) {
        Lexer lexer(get_file_contents(sourceFileName));
        auto tokens = lexer.tokenize();

        Parser parser(tokens);

        try {
            std::unique_ptr<ast::BlockStmt> root = parser.parse();

            ast::SemanticAnalyzer semanticAnalyzer;
            ast::IRGenerator irGenerator;

            root->accept(semanticAnalyzer);
            root->accept(irGenerator);

            if (*print) {
                std::cout << "--- Raccoon Tokens ---" << std::endl;
                for (const auto& token : tokens) {
                    std::cout << "Token Type: " << static_cast<int>(token.type)
                              << " | Lexeme: [" << token.lexeme << "]" << std::endl;
                }

                std::cout << "--- Raccoon AST ---" << std::endl;
                ast::Printer printer;
                root->accept(printer);

                std::cout << "--- Raccoon IR ---" << std::endl;
                irGenerator.module->print(llvm::outs(), nullptr);
            }

            if (*compile) {
                std::filesystem::create_directories("build");
                std::ofstream file("build/" + outputFileName);
                if (!file.is_open()) {
                    std::cerr << "Could not open file '" << outputFileName << "' for writing." << std::endl;
                    return 1;
                }

                llvm::raw_os_ostream dest(file);

                irGenerator.module->print(dest, nullptr);
                dest.flush();
                file.close();

                std::cout << "Successfully generated output.ll!" << std::endl;
            }
        } catch (const ParseError& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    return 0;
}
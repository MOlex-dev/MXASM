/*-------------------------------*
 |        MOlex Assembler        |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#include <iostream>
#include <iomanip>

#include "../include/util.hpp"
#include "../include/lexer.hpp"

//#define DEBUG_INPUT
#define DEBUG_LEXER

using namespace mxasm;


int main(int argc, char **argv)
{
    try {
        // Check arguments
        if (argc == 1) throw std::invalid_argument("No input file!\n");

        std::string source_file_path {argv[1]};
        validate_source_file_path(source_file_path);

        const auto source_code = open_source_code(source_file_path);

#ifdef DEBUG_INPUT
        for (const auto &[line_num, line] : source_code) {
            std::cout << std::setw(5) << line_num << ':';
            std::cout << ' ' << line << '\n';
        }
#endif

        // Using lexer
        lexer tokenizer(source_code);
        const auto &tokens = tokenizer.tokens();

#ifdef DEBUG_LEXER
        for (const auto &token : tokens) {
            std::cout << std::setw(6) << token.row() << ',';
            std::cout << std::setw(4) << token.column() << ':';
            std::cout << std::setw(25) << token.kind_str() << ": ";
            std::cout << token.lexeme();
            std::cout << '\n';
        }
#endif






    } catch (const std::exception &ex) {
        std::cerr << "ERROR!" << std::endl;
        std::cerr << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

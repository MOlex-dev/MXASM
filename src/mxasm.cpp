/*-------------------------------*
 |        MOlex Assembler        |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#include <iostream>
#include <iomanip>

#include "../include/util.hpp"
#include "../include/lexer.hpp"

//#define DEBUG_INPUT

using namespace mxasm;


int main(int argc, char **argv)
{
    try {
        // Check arguments
        if (argc == 1) throw std::invalid_argument("No input file!\n");

        std::string source_file_path {argv[1]};
        validate_source_file_path(source_file_path);

        auto source_code = open_source_code(source_file_path);

#ifdef DEBUG_INPUT
        for (const auto &[line_num, line] : source_code) {
            std::cout << std::setw(5) << line_num << ':';
            std::cout << ' ' << line << '\n';
        }
#endif

        // Using lexer
        lexer tokenizer(source_code);







        // Using lexer
       // source_lexer.tokenize();

     //   auto lexer_tokens = source_lexer.tokens();





    } catch (const std::exception &ex) {
        std::cerr << "ERROR!" << std::endl;
        std::cerr << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/*-------------------------------*
 |        MOlex Assembler        |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

#include "../include/util.hpp"
#include "../include/lexer.hpp"
#include "../include/parser.hpp"
#include "../include/serializer.hpp"

//#define DEBUG_INPUT
//#define DEBUG_LEXER

using namespace mxasm;


int main(int argc, char **argv)
{
    try {
        std::vector<std::string> cmd_arguments(argc);
        for (int i = 0; i < argc; ++i) {
            cmd_arguments[i] = std::string(argv[i]);
        }

        auto source_file_path = get_source_file_path_from_cmd(cmd_arguments);
        auto program_listing = open_source_code(source_file_path);

#ifdef DEBUG_INPUT
        for (const auto &[num, str] : program_listing) {
            std::cout << std::setw(5) << num << ": ";
            std::cout << str << '\n';
        }
#endif

        lexer tokenizer(program_listing);
        auto lexed_tokens = tokenizer.tokens();

#ifdef DEBUG_LEXER
        for (const auto &token : lexed_tokens) {
            std::cout << "[" << std::setw(4) << token.row() << ", " << std::setw(4) << token.column() << "]: ";
            std::cout << std::setw(20) << token.kind()  << ": " << token.lexeme() << '\n';
        }
#endif

        parser lex_parser(lexed_tokens);
        auto parsed_tokens = lex_parser.tokens();

        serializer encoder(parsed_tokens);
        auto program = encoder.binary_program();

        std::string output_name = source_file_path.substr(0, source_file_path.length() - 4) + ".bin";
        write_program_to_file(program, output_name);

    } catch (const mxasm_exception &ex) {
        std::cerr << "ERROR!" << std::endl;
        std::cerr << ex.type() << ": " << ex.message() << std::endl;
        return EXIT_FAILURE;
    } catch (const std::exception &ex) {
        std::cerr << "ERROR!" << std::endl;
        std::cerr << ex.what() << std::endl;
        return EXIT_FAILURE;
    } catch (const exception_list &exs) {
        std::cerr << "ERROR!" << std::endl;
        for (const auto &ex : exs) {
            std::cerr << ex->type() << ": " << ex->message() << '\n';
        }
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

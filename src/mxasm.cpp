#include <iostream>
#include <fstream>
#include "../include/lexer.hpp"

std::string open_program_code(const std::string path);
int main(int argc, char **argv)
{
    try {
        std::string program_code;

        // Read source code from file
        if (argc == 1) throw std::invalid_argument("Wrong source code file name or extension. Should be [name].mxasm\n");
        program_code = open_program_code(argv[1]);


#ifdef DEBUG_SRC   // Print source code
        std::cout << program_code << std::endl;
#endif


        // Using lexer
        mxasm::lexer source_lexer(program_code.c_str());
        source_lexer.tokenize();

        auto lexer_tokens = source_lexer.tokens();


#ifdef DEBUG_LEX
        for (const auto &e : lexer_tokens) {
            std::cout << e.lexeme() << '\n';
        }
#endif





#ifdef DEBUG_PRS
        for (const auto &e : lexer_tokens) {
            std::cout << e.lexeme() << '\n';
        }
#endif






    } catch (const std::exception &e) {
        std::cout << "ERROR!" << '\n' << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}



std::string open_program_code(const std::string path)
{

    if (path.length() < 7 or path.substr(path.length() - 6) != ".mxasm")
        throw std::invalid_argument("Wrong source code file name or extension. Should be [name].mxasm\n");

    std::ifstream file_reader(path);
    if (not file_reader.is_open())
        throw std::ios_base::failure("Source code file opening error");

    std::string buffer, result_str;

    while (std::getline(file_reader, buffer)) {
        result_str.append(buffer + '\n');
    }

    return std::move(result_str);
}
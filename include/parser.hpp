/*-------------------------------*
 |        MOlex Assembler        |
 |             Parser            |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#pragma once

#include <list>
#include <map>

#include "../include/lexer_token.hpp"
#include "../include/parser_token.hpp"
#include "../include/serializable_token.hpp"
#include "../include/exceptions/parser_excpetion.hpp"
#include "../include/util.hpp"


namespace mxasm
{
    class parser
    {
    public:
        explicit parser(std::list<lexer_token> &lexed_tokens);

        std::list<serializable_token> tokens();

    private:
        std::list<std::list<lexer_token>>  m_lexer_tokens;
        std::list<std::list<parser_token>> m_parser_tokens;
        std::list<serializable_token>      m_tokens;
        exception_list                     m_exceptions;

        void tokenize();
        void validate_numbers_size();
        void validate_code_pos_directives();
        void find_byte_lines();
        void index_and_replace_constants();
        void validate_and_replace_labels();
        void parser_tokens_to_serializable();

        void organize_lexer_tokens(std::list<lexer_token> &lexed_tokens);
        void add_exception(const std::string &exception) noexcept;
    };
}

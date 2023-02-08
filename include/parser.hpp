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
        void index_and_replace_constants();

        void organize_lexer_tokens(std::list<lexer_token> &lexed_tokens);
        void add_exception(const std::string &exception) noexcept;
    };
}















//    private:
//        const std::list<lexer_token> &m_lex_tokens;
//        parser_tokens       m_input_tokens;
//        serializable_tokens m_out_tokens;
//
//        void                  tokenize();
//        void                  parse_tokens();
//        parser_tokens         lexer_tokens_to_parser(const std::list<lexer_token> &input);
//        std::string           parse_number_to_hex(const lexer_token &token) const noexcept;
//        parser_token::pt_kind check_directive_type(const std::string &lexeme) const;
//        parser_token::pt_kind check_identifier_type(const std::string &lexeme) const;
//        parser_token::pt_kind check_opcode_name(const std::string &lexeme) const noexcept;
//        parser_token::pt_kind check_register_name(const std::string &lexeme) const noexcept;
//
//        void find_and_replace_macros();
//        void validate_labels();
//
//        bool is_register_name(const std::string &str) const noexcept;
//        bool is_opcode(const std::string &str) const noexcept;
//        bool is_label_declaration(const std::string &str) const noexcept;
//
//        parser_token peek() const noexcept;
//        parser_token get() noexcept;
//
//        parser_tokens::const_iterator m_current     {};
//        parser_tokens::const_iterator m_current_end {};

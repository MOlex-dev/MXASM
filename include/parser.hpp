/*-------------------------------*
 |        MOlex Assembler        |
 |            Parser             |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#pragma once

#include "../include/parser_token.hpp"
#include "../include/serializable_token.hpp"
#include "../include/util.hpp"


namespace mxasm
{
    class parser
    {
    public:
        explicit parser(const lexer_tokens &input_tokens);

        parser_tokens       organized_input() const noexcept;
        serializable_tokens tokens();

    private:
        const lexer_tokens &m_lex_tokens;
        parser_tokens       m_input_tokens;
        serializable_tokens m_out_tokens;

        void                  tokenize();
        void                  parse_tokens();
        parser_tokens         lexer_tokens_to_parser(const lexer_tokens &input);
        std::string           parse_number_to_hex(const lexer_token &token) const noexcept;
        parser_token::pt_kind check_directive_type(const std::string &lexeme) const;
        parser_token::pt_kind check_identifier_type(const std::string &lexeme) const;
        parser_token::pt_kind check_opcode_name(const std::string &lexeme) const noexcept;
        parser_token::pt_kind check_register_name(const std::string &lexeme) const noexcept;

        void find_and_replace_macros();
        void validate_labels();

        bool is_register_name(const std::string &str) const noexcept;
        bool is_opcode(const std::string &str) const noexcept;
        bool is_label_declaration(const std::string &str) const noexcept;










        lexer_tokens::const_iterator m_current;
        lexer_tokens::const_iterator m_current_end;



    };
}
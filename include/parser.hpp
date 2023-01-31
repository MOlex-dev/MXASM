/*-------------------------------*
 |        MOlex Assembler        |
 |            Parser             |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#pragma once

#include "../include/parser_token.hpp"
#include "../include/util.hpp"


namespace mxasm
{
    class parser
    {
    public:
        explicit parser(const lexer_tokens &input_tokens);

        parser_tokens organized_input() const noexcept;
        // TODO: CREATE & REWRITE FOR NOT PARSER_TOKENS, BUT SERIALIZABLE TOKENS
    private:
        const lexer_tokens &m_lex_tokens;
        parser_tokens       m_input_tokens;

        parser_tokens         lexer_tokens_to_parser(const lexer_tokens &input);
        std::string           parse_number_to_hex(const lexer_token &token) const noexcept;
        parser_token::pt_kind check_directive_type(const std::string &lexeme) const;
        parser_token::pt_kind check_identifier_type(const std::string &lexeme) const;
        parser_token::pt_kind check_opcode_name(const std::string &lexeme) const noexcept;
        parser_token::pt_kind check_register_name(const std::string &lexeme) const noexcept;

        bool is_register_name(const std::string &str) const noexcept;
        bool is_opcode(const std::string &str) const noexcept;
        bool is_label_declaration(const std::string &str) const noexcept;










        lexer_tokens::const_iterator m_current;
        lexer_tokens::const_iterator m_current_end;

//        std::unordered_map<std::string, std::variant<byte, word>> m_constants;
//
//
//
//        lexed_oplist lexer_tokens_to_parser_list(const lexer_tokens &input_tokens);
        void         tokenize();
        void         find_constants();
        void         parse_constant();


        parser_token parse_line(const lexer_tokens &tokens);



        parser_token directive();
        parser_token directive_code_position();

        lexer_token peek() const noexcept;
        lexer_token get() noexcept;


        std::string unexpected_token_message(const lexer_token &current,
                                             const lexer_token::lt_kind expected ...) const noexcept;




    };
}
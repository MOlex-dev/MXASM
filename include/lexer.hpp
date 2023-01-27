/*-------------------------------*
 |         MX  Assembler         |
 |             Lexer             |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#pragma once

#include <list>
#include <stdexcept>

#include "../include/lexer_token.hpp"

namespace mxasm
{
    class lexer
    {
    public:
        explicit lexer(const char *file) noexcept;

        void                          tokenize();
        std::list<lexer_token>      &&tokens() noexcept;

    private:
        std::list<lexer_token> m_tokens {};
        const char            *m_file = nullptr;

        lexer_token next() noexcept;
        lexer_token decimal_constant() noexcept;
        lexer_token identifier() noexcept;
        lexer_token atom(const lexer_token::lt_kind kind) noexcept;
        lexer_token unexpected() noexcept;
        lexer_token unexpected(const std::string lexeme) noexcept;
        lexer_token comment() noexcept;
        lexer_token string() noexcept;
        lexer_token register_name() noexcept;
        lexer_token hex_constant() noexcept;
        lexer_token label() noexcept;

        bool is_identifier_char(const char c) noexcept;
        bool is_letter(const char c) noexcept;
        bool is_digit(const char c) noexcept;
        bool is_space(const char c) noexcept;
        bool is_space_or_file_end(const char c) noexcept;
        bool is_hex_digit(const char c) noexcept;

        char peek() const noexcept;
        char get() noexcept;
    };
}
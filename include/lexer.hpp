/*-------------------------------*
 |        MOlex Assembler        |
 |             Lexer             |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#pragma once

#include <list>

#include "../include/lexer_token.hpp"
#include "../include/util.hpp"


namespace mxasm
{
    typedef std::pair<std::size_t, std::string> source_line;
    typedef std::list<lexer_token>              lexer_tokens;

    class lexer
    {
    public:
        explicit lexer(const source_listing &source) noexcept;

        lexer_tokens tokens();
    private:
        const source_listing &m_source_listing;
        lexer_tokens          m_tokens {};

        std::string::const_iterator m_current_begin {};
        std::string::const_iterator m_current_iter  {};
        std::size_t                 m_current_row   {};


        void        tokenize();
        lexer_token next(const source_line &line) noexcept;
        lexer_token comment() noexcept;
        lexer_token atom(const lexer_token::lt_kind kind) noexcept;
        lexer_token identifier() noexcept;
        lexer_token unexpected(const std::string::const_iterator begin) noexcept;
        lexer_token unexpected(const std::string lexeme, std::size_t column) noexcept;


        char        peek() const noexcept;
        char        get() noexcept;
        std::size_t get_column(const std::string::const_iterator position) const noexcept;
        std::size_t get_current_column() const noexcept;


        bool is_space(const char c) const noexcept;
        bool is_end_of_line(const char c) const noexcept;
        bool is_digit(const char c) const noexcept;
        bool is_letter(const char c) const noexcept;
        bool is_identifier_char(const char c) const noexcept;


    private:
        const char            *m_file = nullptr;


        lexer_token decimal_constant() noexcept;
        lexer_token string() noexcept;
        lexer_token register_name() noexcept;
        lexer_token hex_constant() noexcept;

        bool is_space_or_file_end(const char c) noexcept;
        bool is_hex_digit(const char c) noexcept;



    };
}
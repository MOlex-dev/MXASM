/*-------------------------------*
 |        MOlex Assembler        |
 |             Lexer             |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#pragma once

#include <list>

#include "lexer_token.hpp"
#include "exceptions/lexer_exception.hpp"
#include "util.hpp"


namespace mxasm
{
    class lexer
    {
    public:
        explicit lexer(const source_listing &source) noexcept;

        std::list<lexer_token> tokens();

    private:
        const source_listing   &m_source_listing;
        std::list<lexer_token>  m_tokens;
        exception_list          m_exceptions;

        std::string::const_iterator m_current_begin {};
        std::string::const_iterator m_current_iter  {};
        std::size_t                 m_current_row   {};

        void tokenize();
        lexer_token next(const std::pair<std::size_t, std::string> &line) noexcept;
        lexer_token atom(const lexer_token::lt_kind token_kind) noexcept;
        lexer_token unexpected(const std::string::const_iterator begin) noexcept;
        lexer_token unexpected(std::string lexeme, const std::size_t column) noexcept;
        lexer_token comment() noexcept;
        lexer_token directive() noexcept;
        lexer_token string() noexcept;
        lexer_token hex_constant() noexcept;
        lexer_token bin_constant() noexcept;
        lexer_token octal_constant() noexcept;
        lexer_token decimal_constant() noexcept;
        lexer_token identifier_or_label_decl() noexcept;

        std::size_t get_column_number(const std::string::const_iterator current_position) const noexcept;
        void        add_exception(const std::string &exception) noexcept;
        char        peek() const noexcept;
        char        get() noexcept;

        static bool is_space(const char c) noexcept;
        static bool is_end_of_line(const char c) noexcept;
        static bool is_digit(const char c) noexcept;
        static bool is_letter(const char c) noexcept;
        static bool is_identifier_char(const char c) noexcept;
        static bool is_string_symbol(const char c) noexcept;
        static bool is_hex_digit(const char c) noexcept;
        static bool is_bin_digit(const char c) noexcept;
        static bool is_octal_digit(const char c) noexcept;
        static bool validate_identifier_name(const std::string &id) noexcept;
        static bool is_allowed_back_symbol(const char c) noexcept;
    };
}
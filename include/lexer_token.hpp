/*-------------------------------*
 |         MX  Assembler         |
 |          Lexer Token          |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#pragma once

#include <string>

namespace mxasm
{
    class lexer_token
    {



    public:
        enum class lt_kind
        {
            // lexemes
            IDENTIFIER,
            REGISTER,
            DECIMAL_CONSTANT,
            HEXADECIMAL_CONSTANT,
            STRING,
            LABEL,
            COMMENT,

            // atoms
            COMMA,
            LEFT_SQUARE,
            RIGHT_SQUARE,

            // control
            END_OF_FILE,
            UNEXPECTED
        };

        explicit lexer_token(const lt_kind kind) noexcept;
        lexer_token(const lt_kind kind, const std::string lexeme) noexcept;
        lexer_token(const lt_kind kind, const char *const begin, std::size_t len) noexcept;
        lexer_token(const lt_kind kind, const char *const begin, const char *const end) noexcept;

        lt_kind          kind() const noexcept;
        void             kind(const lt_kind kind) noexcept;
        bool             is(const lt_kind kind) const noexcept;
        bool             is_not(const lt_kind kind) const noexcept;
        std::string      lexeme() const noexcept;
        void             lexeme(const std::string lex) noexcept;

    private:
        lt_kind          m_kind{};
        std::string      m_lexeme{};



    };
}
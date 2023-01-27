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
        enum class token_kind
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




        explicit lexer_token(const token_kind kind) noexcept;
        lexer_token(const token_kind kind, const std::string lexeme) noexcept;
        lexer_token(const token_kind kind, const char *const begin, std::size_t len) noexcept;
        lexer_token(const token_kind kind, const char *const begin, const char *const end) noexcept;

        token_kind          kind() const noexcept;
        void             kind(const token_kind kind) noexcept;
        bool             is(const token_kind kind) const noexcept;
        bool             is_not(const token_kind kind) const noexcept;
        std::string      lexeme() const noexcept;
        void             lexeme(const std::string lex) noexcept;



    private:



        token_kind          m_kind{};
        std::string      m_lexeme{};



    };
}
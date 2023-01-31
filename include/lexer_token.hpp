/*-------------------------------*
 |        MOlex Assembler        |
 |          Lexer Token          |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#pragma once

#include <string>
#include <map>
#include <list>


namespace mxasm
{
    class lexer_token
    {
    public:
        enum class lt_kind
        {
            // multichar
            IDENTIFIER,
            COMMENT,
            DECIMAL_CONSTANT,
            BINARY_CONSTANT,
            OCTAL_CONSTANT,
            HEX_CONSTANT,
            DIRECTIVE,
            STRING,

            // atom
            COMMA,
            HASH,
            LEFT_PARENTHESIS,
            RIGHT_PARENTHESIS,
            LESS,
            GREATER,
            ASTERISK,
            EQUALS,

            // control
            END_OF_LINE,
            UNEXPECTED,
        };

        lexer_token(const lt_kind kind);
        lexer_token(const lt_kind kind, const std::string lexeme, const std::size_t row, const std::size_t column);



        std::size_t row() const noexcept;
        std::size_t column() const noexcept;
        std::string lexeme() const noexcept;
        lt_kind     kind() const noexcept;

        void row(const std::size_t row) noexcept;
        void column(const std::size_t column) noexcept;
        void lexeme(std::string lexeme) noexcept;
        void kind(const lt_kind kind) noexcept;

        bool is(const lt_kind kind) const noexcept;
        bool is_not(const lt_kind kind) const noexcept;
        bool is_number() const noexcept;

        static std::string lt_kind_to_string(const lt_kind &kind) noexcept;

    private:
        std::size_t m_row;
        std::size_t m_column;
        std::string m_lexeme;
        lt_kind     m_kind;

        const static std::map<lt_kind, std::string> kind_string;

        friend std::ostream & operator<<(std::ostream &os, const mxasm::lexer_token::lt_kind &kind);
    };


    typedef std::list<lexer_token>                         lexer_tokens;
    std::ostream &operator<<(std::ostream &os, const mxasm::lexer_token::lt_kind &kind);
}



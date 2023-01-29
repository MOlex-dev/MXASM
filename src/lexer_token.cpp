/*-------------------------------*
 |        MOlex Assembler        |
 |             Lexer             |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#include "../include/lexer_token.hpp"

using namespace mxasm;
using lt_kind = lexer_token::lt_kind;



lexer_token::
lexer_token(const lt_kind kind) : lexer_token(kind, "", 0, 0) {}

lexer_token::
lexer_token(const lt_kind kind, const std::string lexeme, const std::size_t row, const std::size_t column)
    : m_kind {kind}, m_lexeme {lexeme}, m_row {row}, m_column {column} {}


std::size_t        lexer_token::
row() const noexcept
{ return m_row; }

std::size_t        lexer_token::
column() const noexcept
{ return m_column; }

std::string        lexer_token::
lexeme() const noexcept
{ return m_lexeme; }

lt_kind            lexer_token::
kind() const noexcept
{ return m_kind; }


void               lexer_token::
row(const std::size_t row) noexcept
{ m_row = row; }

void               lexer_token::
column(const std::size_t column) noexcept
{ m_column = column; }

void               lexer_token::
lexeme(std::string lexeme) noexcept
{ m_lexeme = std::move(lexeme); }

void               lexer_token::
kind(const lt_kind kind) noexcept
{ m_kind = kind; }

std::string        lexer_token::
kind_str() const noexcept
{ return kind_string.at(m_kind); }


bool               lexer_token::
is(const lt_kind kind) const noexcept
{ return m_kind == kind; }

bool               lexer_token::
is_not(const lt_kind kind) const noexcept
{ return m_kind != kind; }


const std::map<lt_kind, std::string> lexer_token::
kind_string
{
    { lt_kind::IDENTIFIER,       "IDENTIFIER"          },
    { lt_kind::COMMENT,          "COMMENT"             },
    { lt_kind::DECIMAL_CONSTANT, "DECIMAL_CONSTANT"    },
    { lt_kind::OCTAL_CONSTANT,   "OCTAL_CONSTANT"      },
    { lt_kind::BINARY_CONSTANT,  "BINARY_CONSTANT"     },
    { lt_kind::HEX_CONSTANT,     "HEXADECIMAL CONSTANT"},
    { lt_kind::DIRECTIVE,        "DIRECTIVE"           },
    { lt_kind::STRING,           "STRING"              },

    { lt_kind::COMMA,             "COMMA"             },
    { lt_kind::HASH,              "HASH"              },
    { lt_kind::LEFT_PARENTHESIS,  "LEFT_PARENTHESIS"  },
    { lt_kind::RIGHT_PARENTHESIS, "RIGHT_PARENTHESIS" },
    { lt_kind::LESS,              "LESS"              },
    { lt_kind::GREATER,           "GREATER"           },
    { lt_kind::ASTERISK,          "ASTERISK"          },
    { lt_kind::EQUALS,            "EQUALS"            },

    { lt_kind::END_OF_LINE, "END OF LINE" },
    { lt_kind::UNEXPECTED,  "UNEXPECTED"  }
};

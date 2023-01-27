/*-------------------------------*
 |         MX  Assembler         |
 |          Lexer Token          |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#include "../include/lexer_token.hpp"

using namespace mxasm;
using lt_kind = lexer_token::lt_kind;



lexer_token::
lexer_token(const lt_kind kind) noexcept
        : m_kind{kind} {}

lexer_token::
lexer_token(const lt_kind kind, const std::string lexeme) noexcept
        : m_kind{kind}, m_lexeme{std::move(lexeme)}{ }

lexer_token::
lexer_token(const lt_kind kind, const char *const begin, std::size_t len) noexcept
        : m_kind{kind}, m_lexeme{begin, len} {}

lexer_token::
lexer_token(const lt_kind kind, const char *const begin, const char *const end) noexcept
        : m_kind{kind}, m_lexeme(begin, std::distance(begin, end)) {}



lt_kind            lexer_token::
kind() const noexcept
{ return m_kind; }

void               lexer_token::
kind(const lt_kind kind) noexcept
{ m_kind = kind; }

bool               lexer_token::
is(const lt_kind kind) const noexcept
{ return kind == m_kind; }

bool               lexer_token::
is_not(const lt_kind kind) const noexcept
{ return not is(kind); }

std::string   lexer_token::
lexeme() const noexcept
{ return m_lexeme; }

void               lexer_token::
lexeme(const std::string lex) noexcept
{ m_lexeme = std::move(lex); }
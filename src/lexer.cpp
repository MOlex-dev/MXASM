/*-------------------------------*
 |        MOlex Assembler        |
 |             Lexer             |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#include <stdexcept>

#include "../include/lexer.hpp"

using namespace mxasm;
using lt_kind = lexer_token::lt_kind;

lexer::
lexer(const source_listing &source) noexcept
    : m_source_listing {source} {}


lexer_tokens       lexer::
tokens()
{
    if (m_tokens.empty()) tokenize();
    return m_tokens;
}


void                              lexer::
tokenize()
{
    std::string exception_msg;
    for (const auto &line: m_source_listing) {
        auto token = next(line);
        while (token.is_not(lt_kind::END_OF_LINE)) {
            if (token.is(lt_kind::UNEXPECTED)) {
                exception_msg.append(std::string("Unexpected token at [")
                                     + std::to_string(token.row()) + ','
                                     + std::to_string(token.column()) + "]:\n"
                                     + token.lexeme() + '\n');
            } else {
                m_tokens.push_back(token);
            }
            token = next(line);
        }
    }
    if (not exception_msg.empty()) {
        throw std::domain_error(exception_msg);
    }
}

lexer_token        lexer::
next(const source_line &line) noexcept
{
    if (m_current_begin != line.second.cbegin()) {
        m_current_begin = m_current_iter = line.second.cbegin();
        m_current_row = line.first;
    }
    while (is_space(peek())) get();

    if (is_end_of_line(peek())) return lexer_token(lt_kind::END_OF_LINE);
    if (is_letter(peek()) or peek() == '_') return identifier();

    if (is_digit(peek())) {
        if (peek() == '0') {
            return octal_constant();
        }
        return decimal_constant();
    }

    switch (peek()) {
        case ';' : return comment();
        case ',' : return atom(lt_kind::COMMA);
        case '#' : return atom(lt_kind::HASH);
        case '(' : return atom(lt_kind::LEFT_PARENTHESIS);
        case ')' : return atom(lt_kind::RIGHT_PARENTHESIS);
        case '$' : return hex_constant();
        case '%' : return bin_constant();
        case '.' : return directive();
        case '\"': return string();
        case '<' : return atom(lt_kind::LESS);
        case '>' : return atom(lt_kind::GREATER);
        case '*' : return atom(lt_kind::ASTERISK);
        case '=' : return atom(lt_kind::EQUALS);
    }
    return unexpected(m_current_iter);
}

lexer_token        lexer::
comment() noexcept
{
    const std::string::const_iterator begin = m_current_iter;
    get();
    while (not is_end_of_line(peek())) get();
    return lexer_token(lt_kind::COMMENT, std::string(begin, m_current_iter), m_current_row, get_column(begin));
}

lexer_token        lexer::
atom(const lt_kind kind) noexcept
{
    const std::string::const_iterator begin = m_current_iter;
    if (kind == lt_kind::RIGHT_PARENTHESIS) {
        auto next_symbol = *(m_current_iter + 1);
        if (next_symbol != ',') {
            if (not (is_space(next_symbol) or is_end_of_line(next_symbol))) {
                return unexpected(begin);
            }
        }
    }
    return lexer_token(kind, std::string(1, get()), m_current_row, get_column(begin));
}

lexer_token        lexer::
identifier() noexcept
{
    const std::string::const_iterator begin = m_current_iter;
    get();
    while (is_identifier_char(peek())) get();

    if (peek() != ',' and peek() != ')') {
        if (peek() == ':') {
            get();
        } else if (not (is_space(peek()) or is_end_of_line(peek()))) {
            return unexpected(begin);
        }
    }

    const std::string result_lexeme(begin, m_current_iter);

    // Check for underscores
    std::size_t underscores = std::count(result_lexeme.cbegin(), result_lexeme.cend(), '_');
    std::size_t name_length = result_lexeme.length();
    if (result_lexeme.at(name_length - 1) == ':') --name_length;

    if (underscores == name_length) {
        return unexpected(result_lexeme, get_column(begin));
    }

    return lexer_token(lt_kind::IDENTIFIER, result_lexeme, m_current_row, get_column(begin));
}

lexer_token        lexer::
unexpected(const std::string::const_iterator begin) noexcept
{
    std::string::const_iterator iter = begin;
    ++iter;
    while (not (is_space(*iter) or is_end_of_line(*iter))) ++iter;
    m_current_iter = iter;
    return unexpected(std::string(begin, iter), get_column(begin));
}

lexer_token        lexer::
unexpected(const std::string lexeme, std::size_t column) noexcept
{ return lexer_token(lt_kind::UNEXPECTED, lexeme, m_current_row, column); }

lexer_token        lexer::
hex_constant() noexcept
{
    const std::string::const_iterator begin = m_current_iter;
    get();
    while (is_hex_digit(peek())) get();

    if (peek() != ',' and peek() != ')') {
        if (not (is_space(peek()) or is_end_of_line(peek()))) {
            return unexpected(begin);
        }
    }

    const std::string result_lexeme(begin, m_current_iter);
    if (result_lexeme.length() == 1) return unexpected(result_lexeme, get_column(begin));
    return lexer_token(lt_kind::HEX_CONSTANT, result_lexeme, m_current_row, get_column(begin));
}

lexer_token        lexer::
bin_constant() noexcept
{
    const std::string::const_iterator begin = m_current_iter;
    get();
    while (is_bin_digit(peek())) get();

    if (peek() != ',' and peek() != ')') {
        if (not (is_space(peek()) or is_end_of_line(peek()))) {
            return unexpected(begin);
        }
    }

    const std::string result_lexeme(begin, m_current_iter);
    if (result_lexeme.length() == 1) return unexpected(result_lexeme, get_column(begin));
    return lexer_token(lt_kind::BINARY_CONSTANT, result_lexeme, m_current_row, get_column(begin));
}

lexer_token        lexer::
octal_constant() noexcept
{
    const std::string::const_iterator begin = m_current_iter;
    get();
    while (is_octal_digit(peek())) get();

    if (peek() != ',' and peek() != ')') {
        if (not (is_space(peek()) or is_end_of_line(peek()))) {
            return unexpected(begin);
        }
    }

    return lexer_token(lt_kind::OCTAL_CONSTANT, std::string(begin, m_current_iter), m_current_row, get_column(begin));
}

lexer_token        lexer::
decimal_constant() noexcept
{
    const std::string::const_iterator begin = m_current_iter;
    get();
    while (is_digit(peek())) get();

    if (peek() != ',' and peek() != ')') {
        if (not (is_space(peek()) or is_end_of_line(peek()))) {
            return unexpected(begin);
        }
    }

    return lexer_token(lt_kind::DECIMAL_CONSTANT, std::string(begin, m_current_iter), m_current_row, get_column(begin));
}

lexer_token        lexer::
directive() noexcept
{
    const std::string::const_iterator begin = m_current_iter;
    get();
    while (is_identifier_char(peek())) get();

    if (not (is_space(peek()) or is_end_of_line(peek()))) {
        return unexpected(begin);
    }

    const std::string result_lexeme(begin, m_current_iter);
    if (result_lexeme.length() == 1) {
        return unexpected(result_lexeme, get_column(begin));
    }

    // Check for underscores
    std::size_t underscores = std::count(result_lexeme.cbegin(), result_lexeme.cend(), '_');
    std::size_t name_length = result_lexeme.length() - 1;

    if (underscores == name_length) {
        return unexpected(result_lexeme, get_column(begin));
    }

    return lexer_token(lt_kind::DIRECTIVE, result_lexeme, m_current_row, get_column(begin));
}

lexer_token        lexer::
string() noexcept
{
    const std::string::const_iterator begin = m_current_iter;
    get();

    while (peek() != '\"') {
        get();
        if (not is_string_symbol(peek())) {
            return unexpected(begin);
        }
    }
    get();

    if (peek() != ',') {
        if (not (is_space(peek()) or is_end_of_line(peek()))) {
            return unexpected(begin);
        }
    }

    const std::string result_lexeme(begin, m_current_iter);
    if (result_lexeme.length() == 2) {
        return unexpected(result_lexeme, get_column(begin));
    }

    return lexer_token(lt_kind::STRING, result_lexeme, m_current_row, get_column(begin));
}


char               lexer::
peek() const noexcept
{ return *m_current_iter; }

char               lexer::
get() noexcept
{ return *m_current_iter++; }

std::size_t        lexer::
get_column(const std::string::const_iterator position) const noexcept
{   return position - m_current_begin + 1; }


bool               lexer::
is_space(const char c) const noexcept
{
    switch (c) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            return true;
    }
    return false;
}

bool               lexer::
is_end_of_line(const char c) const noexcept
{ return c == '\0'; }

bool               lexer::
is_digit(const char c) const noexcept
{ return c >= '0' and c <= '9'; }

bool               lexer::
is_letter(const char c) const noexcept
{ return c >= 'a' and c <= 'z' or c >= 'A' and c <= 'Z'; }

bool               lexer::
is_identifier_char(const char c) const noexcept
{ return is_letter(c) or is_digit(c) or c == '_'; }

bool               lexer::
is_hex_digit(const char c) const noexcept
{
    char x = tolower(c);
    return is_digit(c) or (x >= 'a' and x <= 'f');
}

bool               lexer::
is_bin_digit(const char c) const noexcept
{ return c == '0' or c == '1'; }

bool               lexer::
is_octal_digit(const char c) const noexcept
{ return c >= '0' and c <= '7'; }

bool               lexer::
is_string_symbol(const char c) const noexcept
{ return c >= 0x20 and c <= 0x7E; }

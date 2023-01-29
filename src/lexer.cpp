/*-------------------------------*
 |         MX  Assembler         |
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

    switch (peek()) {
        case ';': return comment();
        case ',': return atom(lt_kind::COMMA);
    }




//
//    if (is_digit(peek())) return decimal_constant();
//    if (is_identifier_char(peek())) return identifier();
//
//    switch (peek()) {
//        case '-': return decimal_constant();
//        case '%': return register_name();
//        case '$': return hex_constant();
//        case '.': return label();
//        case '\"': return string();
//
//        case ',': return atom(lexer_token::lt_kind::COMMA);
//        case '[': return atom(lexer_token::lt_kind::LEFT_SQUARE);
//        case ']': return atom(lexer_token::lt_kind::RIGHT_SQUARE);
//    }
//    return unexpected();
    lexer_token token(lt_kind::DIRECTIVE, std::string(1, peek()), m_current_row, get_current_column());
    ++m_current_iter;
    return token;
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
        return unexpected(result_lexeme, get_current_column());
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
    return unexpected(std::string(begin, iter), get_current_column());
}

lexer_token        lexer:: // TODO: CHECK
unexpected(const std::string lexeme, std::size_t column) noexcept
{ return lexer_token(lt_kind::UNEXPECTED, lexeme, m_current_row, column); }


char               lexer::
peek() const noexcept
{ return *m_current_iter; }

char               lexer::
get() noexcept
{ return *m_current_iter++; }

std::size_t        lexer::
get_column(const std::string::const_iterator position) const noexcept
{   return position - m_current_begin + 1; }

std::size_t        lexer::
get_current_column() const noexcept
{ return get_column(m_current_iter); }


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

////////////////////////////////////////////////////////////////////////////////////////
//
//lexer_token        lexer::
//decimal_constant() noexcept
//{
//    const char *const start = m_file;
//    get();
//    while (is_digit(peek())) get();
//
//    auto token = lexer_token(lexer_token::lt_kind::NUMBER, start, m_file);
//    if (token.lexeme().length() == 1 and token.lexeme()[0] == '-') {
//        return unexpected(token.lexeme());
//    }
//    return token;
//}
//
//
//

//
//lexer_token        lexer::
//unexpected(const std::string lexeme) noexcept
//{ return lexer_token(lexer_token::lt_kind::UNEXPECTED, lexeme, 0, 0); }
//
//lexer_token        lexer::
//comment() noexcept
//{
//    const char *const start = m_file;
//    get();
//    while (peek() != '\n' and peek() != '\0') get();
//    return lexer_token(lexer_token::lt_kind::COMMENT, start, m_file);
//}
//
//lexer_token        lexer::
//string() noexcept
//{
//    const char *const start = m_file;
//    get();
//    while (peek() != '\"') {
//        if (peek() == '\0') { return unexpected(std::string(start)); }
//        get();
//    }
//    return lexer_token(lexer_token::lt_kind::STRING, start, ++m_file);
//}
//
//lexer_token        lexer::
//register_name() noexcept
//{
//    const char *const start = m_file;
//    get();
//    while (is_letter(peek())) get();
//
//    auto token = lexer_token(lexer_token::lt_kind::REGISTER, start, m_file);
//    return token.lexeme().length() == 1 ? unexpected(token.lexeme()) : token;
//}
//
//lexer_token        lexer::
//hex_constant() noexcept
//{
//    const char *const start = m_file;
//    get();
//
//    if (peek() == '-') get();
//
//    while (is_hex_digit(peek())) get();
//
//    auto token = lexer_token(lexer_token::lt_kind::HEXADECIMAL_CONSTANT, start, m_file);
//    auto token_lexeme = token.lexeme();
//
//    if (token_lexeme.length() == 1) return unexpected(token_lexeme);
//    if (token_lexeme.length() == 2 and token_lexeme[1] == '-') return unexpected(token_lexeme);
//    return token;
//}
//
//bool               lexer::
//is_space_or_file_end(const char c) noexcept
//{ return is_space(c) or c == '\0'; }
//
//bool               lexer::
//is_hex_digit(const char c) noexcept
//{
//    char x = tolower(c);
//    return is_digit(x) or (x >= 'a' and x <= 'f');
//}
//
//




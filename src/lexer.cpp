/*-------------------------------*
 |        MOlex Assembler        |
 |             Lexer             |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#include "../include/lexer.hpp"

using namespace mxasm;
using lt_kind = lexer_token::lt_kind;


lexer::
lexer(const source_listing &source) noexcept
        : m_source_listing {source} {}


std::list<lexer_token>  lexer::
tokens()
{
    if (m_tokens.empty()) tokenize();
    return m_tokens;
}


void                    lexer::
tokenize()
{
    for (const auto &line : m_source_listing) {
        auto token = next(line);
        while (token.is_not(lt_kind::END_OF_LINE)) {
            if (token.is(lt_kind::UNEXPECTED)) {
                add_exception("Unexpected token at [" + std::to_string(token.row())
                              + ", " + std::to_string(token.column()) + "]:\n"
                              + token.lexeme());
            } else {
                m_tokens.push_back(token);
            }
            token = next(line);
        }
    }
    if (not m_exceptions.empty()) {
        throw std::move(m_exceptions);
    }
}

lexer_token             lexer::
next(const std::pair<std::size_t, std::string> &line) noexcept
{
    if (m_current_begin != line.second.cbegin()) {
        m_current_begin = m_current_iter = line.second.cbegin();
        m_current_row = line.first;
    }

    while (is_space(peek())) get();

    if (is_end_of_line(peek())) return lexer_token(lt_kind::END_OF_LINE);
    if (is_letter(peek()) or peek() == '_') return identifier_or_label_decl();

    if (is_digit(peek())) {
        if (peek() == '0') return octal_constant();
        return decimal_constant();
    }

    switch (peek()) {
        case ';' : return comment();
        case '.' : return directive();
        case '\"': return string();
        case '$' : return hex_constant();
        case '%' : return bin_constant();
        case ',' : return atom(lt_kind::COMMA);
        case '#' : return atom(lt_kind::HASH);
        case '(' : return atom(lt_kind::LEFT_PARENTHESIS);
        case ')' : return atom(lt_kind::RIGHT_PARENTHESIS);
        case '<' : return atom(lt_kind::LESS);
        case '>' : return atom(lt_kind::GREATER);
        case '*' : return atom(lt_kind::ASTERISK);
        case '=' : return atom(lt_kind::EQUALS);
    }
    return unexpected(m_current_iter);
}

lexer_token             lexer::
atom(const lt_kind kind) noexcept
{
    auto begin = m_current_iter;
    if (kind == lt_kind::RIGHT_PARENTHESIS) {
        auto next_symbol = *(std::next(m_current_iter));
        if (not (is_allowed_back_symbol(next_symbol) or next_symbol == ',')) {
            return unexpected(begin);
        }
    }
    return lexer_token(kind, std::string(1, get()), m_current_row, get_column_number(m_current_iter));
}

lexer_token             lexer::
unexpected(const std::string::const_iterator begin) noexcept
{
    auto iter = begin;
    ++iter;
    while (not (is_space(*iter) or is_end_of_line(*iter))) ++iter;
    m_current_iter = iter;
    return unexpected(std::string(begin, iter), get_column_number(begin));
}

lexer_token             lexer::
unexpected(std::string lexeme, const std::size_t column) noexcept
{ return lexer_token(lt_kind::UNEXPECTED, std::move(lexeme), m_current_row, column); }

lexer_token        lexer::
comment() noexcept
{
    auto begin = m_current_iter;
    get();
    while (not is_end_of_line(peek())) get();
    return lexer_token(lt_kind::COMMENT, std::string(begin, m_current_iter), m_current_row, get_column_number(begin));
}

lexer_token        lexer::
directive() noexcept
{
    auto begin = m_current_iter;
    get();
    while (is_identifier_char(peek())) get();

    if (not is_allowed_back_symbol(peek())) {
        return unexpected(begin);
    }

    const std::string result_lexeme(begin, m_current_iter);

    if (result_lexeme.length() == 1) {
        return unexpected(result_lexeme, get_column_number(begin));
    }

    if (not validate_identifier_name(result_lexeme.substr(1))) {
        return unexpected(result_lexeme, get_column_number(begin));
    }

    return lexer_token(lt_kind::DIRECTIVE, result_lexeme, m_current_row, get_column_number(begin));
}


lexer_token        lexer::
string() noexcept
{
    auto begin = m_current_iter;
    get();

    while (peek() != '\"') {
        if (not is_string_symbol(peek())) {
            return unexpected(begin);
        }
        get();
    }
    get();

    if (not (is_allowed_back_symbol(peek()) or peek() == ',')) {
        return unexpected(begin);
    }

    const std::string result_lexeme(begin, m_current_iter);
    if (result_lexeme.length() == 2) {
        return unexpected(result_lexeme, get_column_number(begin));
    }
    return lexer_token(lt_kind::STRING, result_lexeme, m_current_row, get_column_number(begin));
}

lexer_token             lexer::
hex_constant() noexcept
{
    auto begin = m_current_iter;
    get();
    while (is_hex_digit(peek())) get();

    if (not (is_allowed_back_symbol(peek()) or peek() == ',' or peek() == ')')) {
        return unexpected(begin);
    }

    const std::string result_lexeme(begin, m_current_iter);
    if (result_lexeme.length() == 1) return unexpected(result_lexeme, get_column_number(begin));
    return lexer_token(lt_kind::HEX_CONSTANT, result_lexeme, m_current_row, get_column_number(begin));
}

lexer_token             lexer::
bin_constant() noexcept
{
    auto begin = m_current_iter;
    get();
    while (is_bin_digit(peek())) get();

    if (not (is_allowed_back_symbol(peek()) or peek() == ',' or peek() == ')')) {
        return unexpected(begin);
    }

    const std::string result_lexeme(begin, m_current_iter);
    if (result_lexeme.length() == 1) return unexpected(result_lexeme, get_column_number(begin));
    return lexer_token(lt_kind::BINARY_CONSTANT, result_lexeme, m_current_row, get_column_number(begin));
}

lexer_token             lexer::
octal_constant() noexcept
{
    auto begin = m_current_iter;
    get();
    while (is_octal_digit(peek())) get();

    if (not (is_allowed_back_symbol(peek()) or peek() == ',' or peek() == ')')) {
        return unexpected(begin);
    }

    return lexer_token(lt_kind::OCTAL_CONSTANT, std::string(begin, m_current_iter), m_current_row,
                       get_column_number(begin));
}

lexer_token             lexer::
decimal_constant() noexcept
{
    auto begin = m_current_iter;
    get();
    while (is_digit(peek())) get();

    if (not (is_allowed_back_symbol(peek()) or peek() == ',' or peek() == ')')) {
        return unexpected(begin);
    }

    return lexer_token(lt_kind::DECIMAL_CONSTANT, std::string(begin, m_current_iter), m_current_row,
                       get_column_number(begin));
}

lexer_token             lexer::
identifier_or_label_decl() noexcept
{
    auto begin = m_current_iter;
    get();
    while (is_identifier_char(peek())) get();

    if (peek() == ':') {
        get();
    }

    const std::string result_lexeme(begin, m_current_iter);

    if (not validate_identifier_name(result_lexeme)) {
        return unexpected(result_lexeme, get_column_number(begin));
    }

    if (not (is_allowed_back_symbol(peek()) or peek() == ',' or peek() == ')')) {
        return unexpected(begin);
    }

    switch (std::count(result_lexeme.begin(), result_lexeme.end(), ':')) {
        case 0: return lexer_token(lt_kind::IDENTIFIER, result_lexeme, m_current_row, get_column_number(begin));
        case 1: {
            if (is_allowed_back_symbol(peek())) {
                if (result_lexeme.ends_with(':')) {
                    return lexer_token(lt_kind::LABEL_DECLARATION, result_lexeme, m_current_row,
                                       get_column_number(begin));
                }
            }
        }
    }

    return unexpected(begin);
}


std::size_t             lexer::
get_column_number(const std::string::const_iterator position) const noexcept
{ return std::distance(m_current_begin, position) + 1; }

void                    lexer::
add_exception(const std::string &exception) noexcept
{ m_exceptions.emplace_back(new lexer_exception(exception)); }

char                    lexer::
peek() const noexcept
{ return *m_current_iter; }

char                    lexer::
get() noexcept
{ return *m_current_iter++; }


bool                    lexer::
is_space(const char c) noexcept
{
    switch (c) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            return true;
        default:
            return false;
    }
}

bool                    lexer::
is_end_of_line(const char c) noexcept
{ return c == '\0'; }

bool                    lexer::
is_digit(const char c) noexcept
{ return c >= '0' and c <= '9'; }

bool                    lexer::
is_letter(const char c) noexcept
{ return c >= 'a' and c <= 'z' or c >= 'A' and c <= 'Z'; }

bool                    lexer::
is_identifier_char(const char c) noexcept
{ return is_letter(c) or is_digit(c) or c == '_'; }

bool                    lexer::
is_string_symbol(const char c) noexcept
{ return c >= 0x20 and c <= 0x7E; }

bool                    lexer::
is_hex_digit(const char c) noexcept
{
    char x = tolower(c);
    return is_digit(c) or (x >= 'a' and x <= 'f');
}

bool                    lexer::
is_bin_digit(const char c) noexcept
{ return c == '0' or c == '1'; }

bool                    lexer::
is_octal_digit(const char c) noexcept
{ return c >= '0' and c <= '7'; }

bool                    lexer::
validate_identifier_name(const std::string &id) noexcept
{
    if (id.empty()) {
        return false;
    }
    if (std::count(id.begin(), id.end(), ' ') == id.length()) {
        return false;
    }
    if (std::count(id.begin(), id.end(), '_') == id.length()) {
        return false;
    }
    return true;
}

bool                    lexer::
is_allowed_back_symbol(const char c) noexcept
{ return is_end_of_line(c) or is_space(c) or c == ';'; }

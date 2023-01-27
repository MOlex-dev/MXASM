
/*-------------------------------*
 |         MX  Assembler         |
 |             Lexer             |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#include "../include/lexer.hpp"


using namespace mxasm;



lexer::
lexer(const char *const file) noexcept
        : m_file{file} {}



void                              lexer::
tokenize()
{
    std::string exception_msg{};
    auto token = next();
    while (token.is_not(lexer_token::lt_kind::END_OF_FILE)) {
        if (token.is(lexer_token::lt_kind::UNEXPECTED))
            exception_msg.append(std::string("Unexpected token: ") + token.lexeme()  + '\n');
        else
            m_tokens.push_back(token);
        token = next();
    }
    if (not exception_msg.empty())
        throw std::domain_error(exception_msg);
}

std::list<lexer_token>&&     lexer::
tokens() noexcept
{ return std::move(m_tokens); }



lexer_token        lexer::                // todo: add negative numbers support
next() noexcept
{
    if (peek() == '\0') return lexer_token(lexer_token::lt_kind::END_OF_FILE, m_file, 1);
    while (is_space(peek())) lexer::get();
    if (peek() == '\0') return lexer_token(lexer_token::lt_kind::END_OF_FILE, m_file, 1);

    if (is_digit(peek())) return decimal_constant();
    if (is_identifier_char(peek())) return identifier();

    switch (peek()) {
        case '-': return decimal_constant();
        case '%': return register_name();
        case '$': return hex_constant();
        case '.': return label();
        case '\"': return string();
        case ';': return comment();

        case ',': return atom(lexer_token::lt_kind::COMMA);
        case '[': return atom(lexer_token::lt_kind::LEFT_SQUARE);
        case ']': return atom(lexer_token::lt_kind::RIGHT_SQUARE);
    }
    return unexpected();
}

lexer_token        lexer::
decimal_constant() noexcept
{
    const char *const start = m_file;
    get();
    while (is_digit(peek())) get();

    auto token = lexer_token(lexer_token::lt_kind::DECIMAL_CONSTANT, start, m_file);
    if (token.lexeme().length() == 1 and token.lexeme()[0] == '-') {
        return unexpected(token.lexeme());
    }
    return token;
}

lexer_token        lexer::
identifier() noexcept
{
    const char *const start = m_file;
    get();
    while (is_identifier_char(peek())) get();

    auto token = lexer_token(lexer_token::lt_kind::IDENTIFIER, start, m_file);
    auto token_lexeme = token.lexeme();

    if (std::count(token_lexeme.begin(), token_lexeme.end(), '_') == token_lexeme.length())
        return unexpected(token_lexeme);
    return token;
}

lexer_token        lexer::
atom(const lexer_token::lt_kind kind) noexcept
{ return lexer_token(kind, m_file++, 1); }

lexer_token        lexer::
unexpected() noexcept
{
    const char *const start = m_file;
    get();
    while (not is_space_or_file_end(peek())) get();
    return lexer_token(lexer_token::lt_kind::UNEXPECTED, start, m_file);
}

lexer_token        lexer::
unexpected(const std::string lexeme) noexcept
{ return lexer_token(lexer_token::lt_kind::UNEXPECTED, lexeme); }

lexer_token        lexer::
comment() noexcept
{
    const char *const start = m_file;
    get();
    while (peek() != '\n' and peek() != '\0') get();
    return lexer_token(lexer_token::lt_kind::COMMENT, start, m_file);
}

lexer_token        lexer::
string() noexcept
{
    const char *const start = m_file;
    get();
    while (peek() != '\"') {
        if (peek() == '\0') { return unexpected(std::string(start)); }
        get();
    }
    return lexer_token(lexer_token::lt_kind::STRING, start, ++m_file);
}

lexer_token        lexer::
register_name() noexcept
{
    const char *const start = m_file;
    get();
    while (is_letter(peek())) get();

    auto token = lexer_token(lexer_token::lt_kind::REGISTER, start, m_file);
    return token.lexeme().length() == 1 ? unexpected(token.lexeme()) : token;
}

lexer_token        lexer::
hex_constant() noexcept
{
    const char *const start = m_file;
    get();

    if (peek() == '-') get();

    while (is_hex_digit(peek())) get();

    auto token = lexer_token(lexer_token::lt_kind::HEXADECIMAL_CONSTANT, start, m_file);
    auto token_lexeme = token.lexeme();

    if (token_lexeme.length() == 1) return unexpected(token_lexeme);
    if (token_lexeme.length() == 2 and token_lexeme[1] == '-') return unexpected(token_lexeme);
    return token;
}

lexer_token        lexer::
label() noexcept
{
    const char *const start = m_file;
    get();
    while (is_identifier_char(peek())) get();

    if (peek() == ':') get();

    auto token = lexer_token(lexer_token::lt_kind::LABEL, start, m_file);
    auto token_lexeme = token.lexeme();

    if (token_lexeme.length() == 1) return unexpected(token.lexeme());

    return token;
}



bool               lexer::
is_identifier_char(const char c) noexcept
{ return is_letter(c) || is_digit(c) || c == '_';}

bool               lexer::
is_letter(const char c) noexcept
{ return c >= 'a' and c <= 'z' or c >= 'A' and c <= 'Z'; }

bool               lexer::
is_digit(const char c) noexcept
{ return c >= '0' and c <= '9'; }

bool               lexer::
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

bool               lexer::
is_space_or_file_end(const char c) noexcept
{ return is_space(c) or c == '\0'; }

bool               lexer::
is_hex_digit(const char c) noexcept
{
    char x = tolower(c);
    return is_digit(x) or (x >= 'a' and x <= 'f');
}



char               lexer::
peek() const noexcept
{ return *m_file; }

char               lexer::
get() noexcept
{ return *m_file++; }
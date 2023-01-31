/*-------------------------------*
 |        MOlex Assembler        |
 |            Parser             |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#include <stdexcept>

#include "../include/parser.hpp"

using namespace mxasm;
using lt_kind = mxasm::lexer_token::lt_kind;
using pt_kind = mxasm::parser_token::pt_kind;


parser::
parser(const lexer_tokens &input_tokens)
    : m_input_tokens {lexer_tokens_to_parser(input_tokens)}, m_lex_tokens {input_tokens} {}


parser_tokens      parser::
organized_input() const noexcept
{ return m_input_tokens; }


parser_tokens      parser::
lexer_tokens_to_parser(const lexer_tokens &input)
{
    std::string   exception_str;
    parser_tokens converted_tokens;
    for (const auto &token : input) {
        try {
            parser_token tmp;
            switch (token.kind()) {
                case lt_kind::COMMENT:
                    continue;
                case lt_kind::DIRECTIVE:
                case lt_kind::ASTERISK:
                    tmp = { check_directive_type(token.lexeme()), token.lexeme(), token.row(), token.column() };
                    break;
                case lt_kind::IDENTIFIER:
                    tmp = { check_identifier_type(token.lexeme()), token.lexeme(), token.row(), token.column() };
                    break;
                case lt_kind::BINARY_CONSTANT:
                case lt_kind::OCTAL_CONSTANT:
                case lt_kind::DECIMAL_CONSTANT:
                case lt_kind::HEX_CONSTANT:
                    tmp = { pt_kind::NUMBER, parse_number_to_hex(token), token.row(), token.column() };
                    break;
                case lt_kind::STRING:
                    tmp = { pt_kind::STRING, token.lexeme().substr(1, token.lexeme().length() - 2),
                            token.row(), token.column() };
                    break;
                case lt_kind::COMMA:
                    tmp = { pt_kind::COMMA, token.lexeme(), token.row(), token.column() };
                    break;
                case lt_kind::HASH:
                    tmp = { pt_kind::HASH, token.lexeme(), token.row(), token.column() };
                    break;
                case lt_kind::LEFT_PARENTHESIS:
                    tmp = { pt_kind::LEFT_PARENTHESIS, token.lexeme(), token.row(), token.column() };
                    break;
                case lt_kind::RIGHT_PARENTHESIS:
                    tmp = { pt_kind::RIGHT_PARENTHESIS, token.lexeme(), token.row(), token.column() };
                    break;
                case lt_kind::LESS:
                    tmp = { pt_kind::LESS, token.lexeme(), token.row(), token.column() };
                    break;
                case lt_kind::GREATER:
                    tmp = { pt_kind::GREATER, token.lexeme(), token.row(), token.column() };
                    break;
                case lt_kind::EQUALS:
                    tmp = { pt_kind::EQUALS, token.lexeme(), token.row(), token.column() };
                    break;
                default:
                    throw std::domain_error(std::string("Unexpected token: ") + token.lexeme() + '\n');
            }
            converted_tokens.push_back(tmp);
        } catch (const std::exception &except) {
            exception_str.append(std::string("Error at [") + std::to_string(token.row()) + ", "
                               + std::to_string(token.column()) + "]:\n" + except.what());
        }
    }
    if (not exception_str.empty()) {
        throw std::domain_error(exception_str);
    }
    return converted_tokens;
}

std::string             parser::
parse_number_to_hex(const lexer_token &token) const noexcept
{
    switch (token.kind()) {
        case lt_kind::HEX_CONSTANT:
            return token.lexeme().substr(1);
        case lt_kind::BINARY_CONSTANT:
            return change_number_base(token.lexeme().substr(1), 2, 16);
        case lt_kind::OCTAL_CONSTANT:
            return change_number_base(token.lexeme().substr(1), 8, 16);
        case lt_kind::DECIMAL_CONSTANT:
            return change_number_base(token.lexeme(), 10, 16);
    }
}

parser_token::pt_kind   parser::
check_directive_type(const std::string &lexeme) const
{
    std::string tmp = to_lower(lexeme);
    if (tmp == "*")       return pt_kind::DIRECTIVE_CODE_POSITION;
    if (tmp == ".define") return pt_kind::DIRECTIVE_MACRO;
    if (tmp == ".byte")   return pt_kind::DIRECTIVE_BYTE;
    if (tmp == ".word")   return pt_kind::DIRECTIVE_WORD;
    throw std::domain_error(std::string("There is no directive with such name: ") + lexeme + "\n");
}

parser_token::pt_kind   parser::
check_identifier_type(const std::string &lexeme) const
{
    std::string tmp = to_lower(lexeme);
    if (is_register_name(tmp))     return check_register_name(tmp);
    if (is_opcode(tmp))            return check_opcode_name(tmp);
    if (is_label_declaration(tmp)) return pt_kind::LABEL_DECLARATION;
    return pt_kind::IDENTIFIER;
}

pt_kind            parser::
check_opcode_name(const std::string &lexeme) const noexcept
{ return parser_token::get_opcode_name(lexeme); }

pt_kind            parser::
check_register_name(const std::string &lexeme) const noexcept
{
    if (lexeme == "x") return pt_kind::REGISTER_X;
    if (lexeme == "y") return pt_kind::REGISTER_Y;
}








bool               parser::
is_register_name(const std::string &str) const noexcept
{ return str == "x" or str == "y"; }

bool               parser::
is_opcode(const std::string &str) const noexcept
{ return parser_token::value_exists_in_opcodes(str); }

bool               parser::
is_label_declaration(const std::string &str) const noexcept
{ return *str.rbegin() == ':'; }
























//void               parser::
//tokenize()
//{
//    std::string exception_msg;
//
//    find_constants();
//
//    for (const auto &operation : m_input_tokens) {
//        try {
//            auto res_token = parse_line(operation);
//            m_tokens.push_back(res_token);
//        } catch (const std::exception &except) {
//            exception_msg.append(std::string(except.what()) + '\n');
//            continue;
//        }
//    }
//
//    if (not exception_msg.empty()) {
//        throw std::domain_error(exception_msg);
//    }
//}
//
//void               parser::
//find_constants()
//{
//    for (const auto &line : m_input_tokens) {
//        if (line.begin()->lexeme() == ".define") {
//            m_current = line.begin();
//            m_current_end = line.end();
//            parse_constant();
//
//        }
//
//    }
//
//}

void               parser::
parse_constant()
{
    get();
    if(peek().kind() == lt_kind::IDENTIFIER) { // TODO: Check const sizes

    }

}




parser_token       parser::
parse_line(const lexer_tokens &tokens)
{
    m_current = tokens.cbegin();
    m_current_end = tokens.cend();

    if (peek().kind() == lt_kind::DIRECTIVE or peek().kind() == lt_kind::ASTERISK) return directive();








    if (peek().is_number()) {
        std::string exception_msg = "At [" + std::to_string(m_current->row()) + ',' + std::to_string(m_current->column())
                                  + "]: " + "Expected operation, label, or directive, but "
                                  + lexer_token::lt_kind_to_string(m_current->kind()) + " found!\n"
                                  + m_current->lexeme() + '\n';

    //    throw std::domain_error(unexpected_token_message(get()));
    }









    return parser_token{};
}


parser_token       parser::
directive()
{
    if (get().lexeme() == "*")
    if (get().lexeme() == ".define");
    if (get().lexeme() == ".byte");
    if (get().lexeme() == ".word");
}

parser_token       parser::
directive_code_position()
{

}


lexer_token        parser::
peek() const noexcept
{ return *m_current; }

lexer_token        parser::
get() noexcept
{ return *m_current++; }


//std::string        parser::
//unexpected_token_message(const lexer_token &current, const lt_kind expected ...) const noexcept // TODO: Change lt_on pt_token kind
//{
//    std::string result_str = "At [" + std::to_string(current.row()) + ',' + std::to_string(current.column())
//                             + "]: " + "Expected ";
//
//    va_list args;
//    va_start(args, expected);
//
//    auto expected_kind = va_arg(args, lt_kind);
//
//    while (true) {
//        result_str.append(lexer_token::lt_kind_to_string(expected_kind) + ", ");
//    }
//
//
//
//
//
//    result_str.append("but " + lexer_token::lt_kind_to_string(current.kind()) + " found:\n"
//                      + current.lexeme());
//    return result_str;
//}
//
//
//





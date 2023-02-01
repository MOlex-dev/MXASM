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
#include <iostream>
serializable_tokens     parser::
tokens()
{
    try {
        if (m_out_tokens.empty()) tokenize();
    } catch (const std::exception &except) {
        throw std::domain_error(except.what());
    }
    return m_out_tokens;
}


void               parser::
tokenize()
{
    std::string exception_msg;

    try { find_and_replace_macros(); }
    catch (const std::exception &ex) {
        exception_msg.append(std::string("Macro declaration error:\n") + ex.what());
    }

    try { validate_labels(); }
    catch (const std::exception &ex) {
        exception_msg.append(std::string("Macro declaration error:\n") + ex.what());
    }


    //TODO: WORK HERE (VALIDATE .byte .word strings and byte squences)





    if (not exception_msg.empty()) {
        throw std::domain_error(exception_msg);
    }
}

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


void               parser::
find_and_replace_macros()
{
    std::map<std::string, std::string> macros_values;
    std::list<parser_tokens> tokens_row;
    std::size_t current_row = 0;

    // Generate list of command rows
    for (const auto &e : m_input_tokens) {
        if (e.row() == current_row) {
            tokens_row.rbegin()->push_back(e);
            continue;
        }
        current_row = e.row();
        tokens_row.push_back({e});
    }

    std::string exception_string;

    // Find macro
    for (const auto &line : tokens_row) {
        auto iter = line.begin();
        current_row = iter->row();
        std::string macro_name, macro_value;

        if (iter->kind() != pt_kind::DIRECTIVE_MACRO) continue;
        std::advance(iter, 1);
        if (iter == line.end()) {
            exception_string.append(std::string("Error at line ") + std::to_string(current_row)
                                    + std::string(". An IDENTIFIER was expected, but NEW LINE was found\n"));
            continue;
        }
        if (iter->kind() != pt_kind::IDENTIFIER) {
            exception_string.append(std::string("Error at [") + std::to_string(current_row) + ", "
                                  + std::to_string(iter->column()) + "]: An IDENTIFIER was expected, but "
                                  + parser_token::pt_kind_to_str(iter->kind()) + " was found\n");
            continue;
        }
        macro_name = iter->lexeme();
        std::advance(iter, 1);
        if (iter == line.end()) {
            exception_string.append(std::string("Error at line ") + std::to_string(current_row)
                                    + std::string(". A NUMBER was expected, but NEW LINE was found\n"));
            continue;
        }
        if (iter->kind() != pt_kind::NUMBER) {
            exception_string.append(std::string("Error at [") + std::to_string(current_row) + ", "
                                  + std::to_string(iter->column()) + "]: A NUMBER was expected, but "
                                  + parser_token::pt_kind_to_str(iter->kind()) + " was found\n");
            continue;
        }
        macro_value = iter->lexeme();
        std::advance(iter, 1);
        if (iter != line.end()) {
            exception_string.append(std::string("Error at [") + std::to_string(current_row) + ", "
                                  + std::to_string(iter->column()) + "]: A NEW LINE was expected, but "
                                  + parser_token::pt_kind_to_str(iter->kind()) + " was found\n");
            continue;
        }

        // Add to macro list
        if (macros_values.contains(macro_name)) {
            exception_string.append(std::string("Error at line ") + std::to_string(current_row)
                                  + ": Macro with name " + macro_name + " already exists\n");
            continue;
        }
        macros_values.emplace(to_lower(macro_name), macro_value);
    }
    if (not exception_string.empty()) {
        throw std::domain_error(exception_string);
    }

    // Replace macro declaration and change macro using tokens
    auto rem_iter = std::remove_if(tokens_row.begin(), tokens_row.end(), [](auto &row) {
        if (row.begin()->kind() == pt_kind::DIRECTIVE_MACRO) return true;
        return false;
    });
    tokens_row.erase(rem_iter, tokens_row.end());

    m_input_tokens.clear();
    for (auto &line : tokens_row) {
        for (auto &token : line) {
            if (token.is(pt_kind::IDENTIFIER)) {
                if (macros_values.contains(to_lower(token.lexeme()))) {
                    token.lexeme(macros_values[to_lower(token.lexeme())]);
                    token.kind(pt_kind::NUMBER);
                }
            }
            m_input_tokens.push_back(token);
        }
    }
}

void               parser::
validate_labels()
{






    std::map<std::string, std::string> labels;




    std::string exception_msg;
    std::size_t label_number {0};
    //fixme: try to do again list<lists>
    for (auto &token : m_input_tokens) {
        if (token.is(pt_kind::LABEL_DECLARATION)) {
            std::string label_name = to_lower(token.lexeme().substr(0, token.lexeme().length() - 1));

            //TODO: LABEL DECL CAN BE ONLY IN THE START OF STRING
            if (labels.contains(label_name)) {
                exception_msg.ap
            }

            if (labels.contains(new_lexeme)) {
                exception_msg.append(std::string("Error at line ") + std::to_string(token.row())
                                     + ": Label \'" + new_lexeme + "\' already exists\n");
            }

//FIXME TOLOWER!!!!!!!!!!!!!!!!


//                // Add to macro list
//                if (macros_values.contains(macro_name)) {
//                    exception_string.append();
//                    continue;
//                }
//                macros_values.emplace(to_lower(macro_name), macro_value);





        }

            // label actually exists
    }
//TOKEN NAME                  $5

    for (const auto &e : m_input_tokens) {

        
    }
//TODO: FIX

// .byte and .word for strings and bytes lists



    if (not exception_msg.empty()) {
        throw std::domain_error(exception_msg);
    }
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



//    result_str.append("but " + lexer_token::lt_kind_to_string(current.kind()) + " found:\n"
//                      + current.lexeme());
//    return result_str;
//}




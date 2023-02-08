/*-------------------------------*
 |        MOlex Assembler        |
 |             Parser            |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#include "../include/parser.hpp"

using namespace mxasm;
using lt_kind   = lexer_token::lt_kind;
using pt_kind   = parser_token::pt_kind;
using pt_opcode = parser_token::pt_opcode;


parser::
parser(std::list<lexer_token> &lexed_tokens)
{ organize_lexer_tokens(lexed_tokens); }


std::list<serializable_token>     parser::
tokens()
{
    if (m_tokens.empty()) tokenize();
    return m_tokens;
}


void                    parser::
tokenize()
{
    // Convert to parser list
    std::size_t current_row = 0;
    for (auto &line : m_lexer_tokens) {
        for (auto &token : line) {
            parser_token tk(token);
            switch (token.kind()) {
                case lt_kind::BINARY_CONSTANT:
                    tk.v_number(string_to_number(token.lexeme().substr(1), 2));
                    tk.kind(pt_kind::NUMBER); break;
                case lt_kind::OCTAL_CONSTANT:
                    tk.v_number(string_to_number(token.lexeme().substr(1), 8));
                    tk.kind(pt_kind::NUMBER); break;
                case lt_kind::DECIMAL_CONSTANT:
                    tk.v_number(string_to_number(token.lexeme(), 10));
                    tk.kind(pt_kind::NUMBER); break;
                case lt_kind::HEX_CONSTANT:
                    tk.v_number(string_to_number(token.lexeme().substr(1), 16));
                    tk.kind(pt_kind::NUMBER); break;
                case lt_kind::LABEL_DECLARATION:
                    tk.v_lexeme(token.lexeme().substr(0, token.lexeme().length() - 1));
                    tk.kind(pt_kind::LABEL_DECLARATION); break;
                case lt_kind::IDENTIFIER:
                    if (parser_token::is_opcode_or_register(token.lexeme())) {
                        tk.v_opcode(parser_token::get_opcode_by_name(token.lexeme()));
                        tk.kind(pt_kind::OPCODE);
                    } else {
                        tk.v_lexeme(token.lexeme());
                        tk.kind(pt_kind::_IDENTIFIER);
                    }
                    break;
                case lt_kind::ASTERISK:
                    tk.v_directive(parser_token::pt_directive::CODE_POSITION);
                    tk.kind(pt_kind::DIRECTIVE); break;
                case lt_kind::DIRECTIVE:
                    tk.v_directive(parser_token::get_directive_by_name(token.lexeme().substr(1)));
                    tk.kind(pt_kind::DIRECTIVE); break;
                case lt_kind::STRING:
                    tk.v_lexeme(token.lexeme().substr(1, token.lexeme().length() - 2));
                    tk.kind(pt_kind::STRING); break;
                case lt_kind::COMMA:
                    tk.kind(pt_kind::COMMA); break;
                case lt_kind::HASH:
                    tk.kind(pt_kind::HASH); break;
                case lt_kind::LESS:
                    tk.kind(pt_kind::LESS); break;
                case lt_kind::GREATER:
                    tk.kind(pt_kind::GREATER); break;
                case lt_kind::LEFT_PARENTHESIS:
                    tk.kind(pt_kind::LEFT_PARENTHESIS); break;
                case lt_kind::RIGHT_PARENTHESIS:
                    tk.kind(pt_kind::RIGHT_PARENTHESIS); break;
                case lt_kind::EQUALS:
                    tk.kind(pt_kind::EQUALS); break;
                default:
                    add_exception("Unexpected token at [" + std::to_string(token.row()) + ", "
                                  + std::to_string(token.column()) + "]:\n" + token.lexeme());
                    continue;
            }
            if (token.row() != current_row) {
                m_parser_tokens.push_back({tk});
                current_row = token.row();
                continue;
            }
            m_parser_tokens.rbegin()->push_back(tk);
        }
    }

    index_and_replace_constants();






    if (not m_exceptions.empty()) {
        throw std::move(m_exceptions);
    }


    // TODO: FROM HERE  TODO: CLEAR CONSTANTS (DEFINES)



    for (const auto &line : m_parser_tokens) {
        std::cout << line.begin()->row() << ": ";
        for (const auto &t : line) {
            auto tk = t.kind();
            std::cout << '{' << t.column() << ':' << tk << ':';
            if (tk == pt_kind::NUMBER) {
                std::cout << t.v_number();
            } else if (tk == pt_kind::OPCODE) {
                std::cout << t.v_opcode();
            } else if (tk == pt_kind::COMMA) {
            } else if (tk == pt_kind::STRING) {
                std::cout << t.v_lexeme();
            } else if (tk == pt_kind::DIRECTIVE) {
                std::cout << t.v_directive();
            } else {
                std::cout << t.v_lexeme();
            }
            std::cout << '}' << ";  ";
        }
        std::cout << '\n';
    }


    if (not m_exceptions.empty()) {
        throw std::move(m_exceptions);
    }
}

void index_and_replace_constants()
{
    

    //TODO FROM THERE

}


void                    parser::
organize_lexer_tokens(std::list<lexer_token> &lexed_tokens)
{
    std::size_t i = 0;
    for (auto &token : lexed_tokens) {
        if (token.is(lt_kind::COMMENT)) continue;
        if (token.row() != i) {
            i = token.row();
            m_lexer_tokens.push_back(std::list<lexer_token>());
        }
        m_lexer_tokens.rbegin()->push_back(std::move(token));
    }
}


void                    parser::
add_exception(const std::string &exception) noexcept
{ m_exceptions.emplace_back(new parser_exception(exception)); }





//using st_kind = mxasm::serializable_token::st_kind;
//using st_directive = mxasm::serializable_token::st_directive;
//using st_command = mxasm::serializable_token::st_command;
//using st_adr_mode = mxasm::serializable_token::st_cmd_ad_mode;

//void               parser::
//tokenize()
//{
//    std::string exception_msg;
//
//    try { find_and_replace_macros(); }
//    catch (const std::exception &ex) {
//        exception_msg.append(std::string("Macro declaration error:\n") + ex.what());
//    }
//
//    try { validate_labels(); }
//    catch (const std::exception &ex) {
//        exception_msg.append(std::string("Label error:\n") + ex.what());
//    }
//
//    std::size_t current_row {};
//    // Check if row starts not from directive, label declaration, or opcode
//    for (const auto &token : m_input_tokens) {
//        if (current_row == token.row()) continue;
//        current_row = token.row();
//        if (token.is_not(pt_kind::LABEL_DECLARATION) and
//            (not parser_token::is_opcode(token.kind())) and
//            (not parser_token::is_directive(token.kind()))) {
//            exception_msg.append(std::string("Error at [") + std::to_string(token.row()) + ", "
//                               + std::to_string(token.column()) + "]: DIRECTIVE, LABEL DECLARATION or OPCODE was " \
//                                 "expected, but " + parser_token::pt_kind_to_str(token.kind()) + " was found:\n"
//                               + token.lexeme() + '\n');
//        }
//    }
//    try { parse_tokens(); }
//    catch (const std::exception &ex) {
//        exception_msg.append(std::string("Parsing error:\n") + ex.what());
//    }
//
//    if (not exception_msg.empty()) {
//        throw std::domain_error(exception_msg);
//    }
//}
//
//void               parser::
//parse_tokens()
//{
//    std::list<parser_tokens> tokens_row;
//    std::size_t current_row = 0;
//
//    // Generate list of command rows
//    for (const auto &e : m_input_tokens) {
//        if (e.row() == current_row) {
//            tokens_row.rbegin()->push_back(e);
//            continue;
//        }
//        current_row = e.row();
//        tokens_row.push_back({ e });
//    }
//
//    std::string exception_msg;
//
//    for (const auto &line : tokens_row) {
//        if (m_current_end != line.cend()) {
//            m_current = line.cbegin();
//            m_current_end = line.cend();
//            current_row = line.cbegin()->row();
//        }
//
//        serializable_token outer_token;
//        switch (get().kind()) {
//
//
//            //TODO: WORK HERE (VALIDATE .byte .word strings and byte sequences)
//            // TODO: AFTER BYTES WE CAN HAVE ALSO LABEL
//
//
//            case pt_kind::DIRECTIVE_BYTE:
//
//                break;
//            case pt_kind::DIRECTIVE_WORD:
//
//                break;
//            case pt_kind::DIRECTIVE_CODE_POSITION:
//                if (m_current == m_current_end) {
//                    exception_msg.append(std::string("Error at line ") + std::to_string(current_row)
//                                         + ". An \'=\' was expected, but NEW LINE was found\n");
//                    continue;
//                }
//                if (m_current->is_not(pt_kind::EQUALS)) {
//                    exception_msg.append(std::string("Error at [") + std::to_string(current_row) + ", "
//                                        + std::to_string(m_current->column()) + "]: An '\'=\' was expected, but "
//                                        + parser_token::pt_kind_to_str(m_current->kind()) + " was found\n");
//                    continue;
//                }
//                std::advance(m_current, 1);
//                if (m_current == m_current_end) {
//                    exception_msg.append(std::string("Error at line ") + std::to_string(current_row)
//                                         + ". An NUMBER was expected, but NEW LINE was found\n");
//                    continue;
//                }
//                if (m_current->is_not(pt_kind::NUMBER)) {
//                    exception_msg.append(std::string("Error at [") + std::to_string(current_row) + ", "
//                                         + std::to_string(m_current->column()) + "]: A NUMBER was expected, but "
//                                         + parser_token::pt_kind_to_str(m_current->kind()) + " was found\n");
//                    continue;
//                }
//                std::advance(m_current, 1);
//                if (m_current != m_current_end) {
//                    exception_msg.append(std::string("Error at [") + std::to_string(current_row) + ", "
//                                         + std::to_string(m_current->column()) + "]: A NEW LINE was expected, but "
//                                         + parser_token::pt_kind_to_str(m_current->kind()) + " was found\n");
//                    continue;
//                }
//                outer_token.kind(st_kind::DIRECTIVE);
//                outer_token.directive(st_directive::CODE_POSITION);
//                break;
//        }
//        m_out_tokens.push_back(outer_token);
//    }
//    if (not exception_msg.empty()) {
//        throw std::domain_error(exception_msg);
//    }
//}

//std::string             parser::
//parse_number_to_hex(const lexer_token &token) const noexcept
//{
//    switch (token.kind()) {
//        case lt_kind::HEX_CONSTANT:
//            return token.lexeme().substr(1);
//        case lt_kind::BINARY_CONSTANT:
//            return change_number_base(token.lexeme().substr(1), 2, 16);
//        case lt_kind::OCTAL_CONSTANT:
//            return change_number_base(token.lexeme().substr(1), 8, 16);
//        case lt_kind::DECIMAL_CONSTANT:
//            return change_number_base(token.lexeme(), 10, 16);
//    }
//}
//
//parser_token::pt_kind   parser::
//check_directive_type(const std::string &lexeme) const
//{
//    std::string tmp = to_lower(lexeme);
//    if (tmp == "*")       return pt_kind::DIRECTIVE_CODE_POSITION;
//    if (tmp == ".define") return pt_kind::DIRECTIVE_MACRO;
//    if (tmp == ".byte")   return pt_kind::DIRECTIVE_BYTE;
//    if (tmp == ".word")   return pt_kind::DIRECTIVE_WORD;
//    throw std::domain_error(std::string("There is no directive with such name: ") + lexeme + "\n");
//}
//
//parser_token::pt_kind   parser::
//check_identifier_type(const std::string &lexeme) const
//{
//    std::string tmp = to_lower(lexeme);
//    if (is_register_name(tmp))     return check_register_name(tmp);
//    if (is_opcode(tmp))            return check_opcode_name(tmp);
//    if (is_label_declaration(tmp)) return pt_kind::LABEL_DECLARATION;
//    return pt_kind::IDENTIFIER;
//}
//
//pt_kind            parser::
//check_opcode_name(const std::string &lexeme) const noexcept
//{ return parser_token::get_opcode_name(lexeme); }

//
//
//void               parser::
//find_and_replace_macros()
//{
//    std::map<std::string, std::string> macros_values;
//    std::list<parser_tokens> tokens_row;
//    std::size_t current_row = 0;
//
//    // Generate list of command rows
//    for (const auto &e : m_input_tokens) {
//        if (e.row() == current_row) {
//            tokens_row.rbegin()->push_back(e);
//            continue;
//        }
//        current_row = e.row();
//        tokens_row.push_back({e});
//    }
//
//    std::string exception_string;
//
//    // Find macro
//    for (const auto &line : tokens_row) {
//        auto iter = line.begin();
//        current_row = iter->row();
//        std::string macro_name, macro_value;
//
//        if (iter->kind() != pt_kind::DIRECTIVE_MACRO) continue;
//        std::advance(iter, 1);
//        if (iter == line.end()) {
//            exception_string.append(std::string("Error at line ") + std::to_string(current_row)
//                                    + std::string(". An IDENTIFIER was expected, but NEW LINE was found\n"));
//            continue;
//        }
//        if (iter->kind() != pt_kind::IDENTIFIER) {
//            exception_string.append(std::string("Error at [") + std::to_string(current_row) + ", "
//                                  + std::to_string(iter->column()) + "]: An IDENTIFIER was expected, but "
//                                  + parser_token::pt_kind_to_str(iter->kind()) + " was found\n");
//            continue;
//        }
//        macro_name = iter->lexeme();
//        std::advance(iter, 1);
//        if (iter == line.end()) {
//            exception_string.append(std::string("Error at line ") + std::to_string(current_row)
//                                    + std::string(". A NUMBER was expected, but NEW LINE was found\n"));
//            continue;
//        }
//        if (iter->kind() != pt_kind::NUMBER) {
//            exception_string.append(std::string("Error at [") + std::to_string(current_row) + ", "
//                                  + std::to_string(iter->column()) + "]: A NUMBER was expected, but "
//                                  + parser_token::pt_kind_to_str(iter->kind()) + " was found\n");
//            continue;
//        }
//        macro_value = iter->lexeme();
//        std::advance(iter, 1);
//        if (iter != line.end()) {
//            exception_string.append(std::string("Error at [") + std::to_string(current_row) + ", "
//                                  + std::to_string(iter->column()) + "]: A NEW LINE was expected, but "
//                                  + parser_token::pt_kind_to_str(iter->kind()) + " was found\n");
//            continue;
//        }
//
//        // Add to macro list
//        if (macros_values.contains(macro_name)) {
//            exception_string.append(std::string("Error at line ") + std::to_string(current_row)
//                                  + ": Macro with name " + macro_name + " already exists\n");
//            continue;
//        }
//        macros_values.emplace(to_lower(macro_name), macro_value);
//    }
//    if (not exception_string.empty()) {
//        throw std::domain_error(exception_string);
//    }
//
//    // Replace macro declaration and change macro using tokens
//    auto rem_iter = std::remove_if(tokens_row.begin(), tokens_row.end(), [](auto &row) {
//        if (row.begin()->kind() == pt_kind::DIRECTIVE_MACRO) return true;
//        return false;
//    });
//    tokens_row.erase(rem_iter, tokens_row.end());
//
//    m_input_tokens.clear();
//    for (auto &line : tokens_row) {
//        for (auto &token : line) {
//            if (token.is(pt_kind::IDENTIFIER)) {
//                if (macros_values.contains(to_lower(token.lexeme()))) {
//                    token.lexeme(macros_values[to_lower(token.lexeme())]);
//                    token.kind(pt_kind::NUMBER);
//                }
//            }
//            m_input_tokens.push_back(token);
//        }
//    }
//}
//
//void               parser::
//validate_labels()
//{
//    std::map<std::string, std::string> labels;
//    std::size_t                        current_row = 0;
//
//    std::string exception_msg;
//    std::size_t label_number {0};
//
//    for (auto &token : m_input_tokens) {
//        if (token.row() == current_row) continue;
//        current_row = token.row();
//
//        if (token.is(pt_kind::LABEL_DECLARATION)) {
//            std::string label_name = to_lower(token.lexeme().substr(0, token.lexeme().length() - 1));
//            if (labels.contains(label_name)) {
//                exception_msg.append(std::string("Error at line ") + std::to_string(token.row())
//                                   + ": Label \'" + token.lexeme() + "\' already exists\n");
//                continue;
//            }
//
//            // Add to macro list
//            auto new_label_name = std::string("@") + std::to_string(label_number++);
//            labels.emplace(label_name, new_label_name);
//            token.lexeme(new_label_name);
//        }
//    }
//
//    current_row = 0;
//    for (auto &token : m_input_tokens) {
//        if (token.row() != current_row) {
//            current_row = token.row();
//            continue;
//        }
//
//        if (token.is_not(pt_kind::IDENTIFIER)) continue;
//        std::string label_name = to_lower(token.lexeme());
//
//        if (not labels.contains(label_name)) {
//            exception_msg.append(std::string("Error at [") + std::to_string(token.row()) + ", "
//                               + std::to_string(token.column()) + "]: Label or macro \'" + token.lexeme()
//                               + "\' doesn't exist\n");
//            continue;
//        }
//
//        token.lexeme(labels[label_name]);
//        token.kind(pt_kind::LABEL_CALL);
//    }
//    if (not exception_msg.empty()) {
//        throw std::domain_error(exception_msg);
//    }
//}
//
//
//bool               parser::
//is_register_name(const std::string &str) const noexcept
//{ return str == "x" or str == "y"; }
//
//bool               parser::
//is_opcode(const std::string &str) const noexcept
//{ return parser_token::value_exists_in_opcodes(str); }
//
//bool               parser::
//is_label_declaration(const std::string &str) const noexcept
//{ return *str.rbegin() == ':'; }

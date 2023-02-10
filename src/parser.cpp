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
#include <iomanip> //TODO: REMOVE THIS LIB

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


    // Check numbers
    index_and_replace_constants();
    validate_numbers_size();
    validate_code_pos_directives();
    find_byte_lines();



    if (not m_exceptions.empty()) {
        throw std::move(m_exceptions);
    }


    // TODO: FROM HERE





    for (auto &line : m_parser_tokens) {
        std::cout << std::setw(5) << std::dec << line.begin()->row() << ": ";

        for (auto &t : line) {
            auto tk = t.kind();

            std::cout << '{' << tk << ':';
            if (tk == pt_kind::NUMBER) {
                std::cout <<  std::hex << t.v_number();
            } else if (tk == pt_kind::DIRECTIVE) {
                std::cout << t.v_directive();
                if (t.v_directive() == parser_token::pt_directive::CODE_POSITION) {
                    std::cout << '=' << std::hex << t.v_number();
                } else if (t.v_directive() == parser_token::pt_directive::BYTE) {
                    std::cout << '=' << std::right;
                    const auto &bl = t.v_byteline();
                    for (const auto &c : bl) {
                        std::cout << std::setw(3) << std::hex << c;
                    }
                } else if (t.v_directive() == parser_token::pt_directive::WORD) {
                    std::cout << '=' << std::right;
                    const auto &bl = t.v_byteline();
                    for (const auto &c : bl) {
                        std::cout << std::setw(5) << std::hex << c;
                    }
                }
            }


            else if (tk == pt_kind::OPCODE) {
                std::cout << t.v_opcode();
            } else if (tk == pt_kind::COMMA) {
            } else if (tk == pt_kind::STRING) {
                std::cout << t.v_lexeme();
            } else {
                std::cout << t.v_lexeme();
            }
            std::cout << '}' << std::left << std::setw(4) << ',';
        }
        std::cout << '\n';
    }


    if (not m_exceptions.empty()) {
        throw std::move(m_exceptions);
    }
}

void                    parser::
validate_numbers_size()
{
    for (const auto &line : m_parser_tokens) {
        for (const auto &token : line) {
            if (token.kind() == pt_kind::NUMBER) {
                if (token.v_number() > 0xFF'FF) {
                    add_exception("Error at [" + std::to_string(token.row()) + ", " + std::to_string(token.column())
                                  + "]:\nNumber constant can't be greater than 0xFF\'FF");
                }
            }
        }
    }
}

void                    parser::
index_and_replace_constants()
{
    std::map<std::string, word_t> macro_table;

    // Find and validate .define
    for (auto &line : m_parser_tokens) {
        auto iter = line.begin();
        auto ln_end = line.end();
        if (iter->kind() != pt_kind::DIRECTIVE) continue;
        if (iter->v_directive() != parser_token::pt_directive::MACRO) continue;

        std::string macro_name;
        word_t      macro_value;


        std::advance(iter, 1);
        if (iter == ln_end) {
            add_exception("Error at line " + std::to_string(line.cbegin()->row())
                          + ":\nAn IDENTIFIER was expected, but NEW LINE was found");
            continue;
        }
        if (iter->kind() != pt_kind::_IDENTIFIER) {
            add_exception("Error at [" + std::to_string(iter->row()) + ", " + std::to_string(iter->column())
                          + "]:\nAn IDENTIFIER was expected, but " + parser_token::pt_kind_to_string(iter->kind())
                          + " was found");
            continue;
        }
        if (macro_table.contains(to_lower(iter->v_lexeme()))) {
            add_exception("Error at line " + std::to_string(iter->row()) + "\nRepeated declaration of macro: "
                          + iter->v_lexeme());
            continue;
        }
        macro_name = to_lower(iter->v_lexeme());

        std::advance(iter, 1);
        if (iter == ln_end) {
            add_exception("Error at line " + std::to_string(line.cbegin()->row())
                          + ":\nA NUMBER was expected, but NEW LINE was found");
            continue;
        }
        if (iter->kind() != pt_kind::NUMBER) {
            add_exception("Error at [" + std::to_string(iter->row()) + ", " + std::to_string(iter->column())
                          + "]:\nA NUMBER was expected, but " + parser_token::pt_kind_to_string(iter->kind())
                          + " was found");
            continue;
        }
        if (iter->v_number() > 0xFF'FF) {
            add_exception("Error at [" + std::to_string(iter->row()) + ", " + std::to_string(iter->column())
                          + "]:\nMaximal CONSTANT size is 0xFF'FF");
            continue;
        }
        macro_value = iter->v_number();

        std::advance(iter, 1);
        if (iter != ln_end) {
            add_exception("Error at [" + std::to_string(iter->row()) + ", " + std::to_string(iter->column())
                          + "]:\nA NEW LINE was expected, but " + parser_token::pt_kind_to_string(iter->kind())
                          + " was found");
            continue;
        }
        macro_table.emplace(macro_name, macro_value);
    }

    if (not m_exceptions.empty()) return;

    // Remove macro declarations from parser tokens
    erase_if(m_parser_tokens, [](auto &line) {
        return line.begin()->kind() == pt_kind::DIRECTIVE and
               line.begin()->v_directive() == parser_token::pt_directive::MACRO;
    });

    // Replace macros code
    for (auto &line : m_parser_tokens) {
        for (auto &token : line) {
            if (token.kind() == pt_kind::_IDENTIFIER) {
                std::string lex = to_lower(token.v_lexeme());
                if (macro_table.contains(lex)) {
                    token.kind(pt_kind::NUMBER);
                    token.v_number(macro_table.at(lex));
                } else {
                    token.kind(pt_kind::LABEL_CALL);
                }
            }
        }
    }
}

void                    parser::
validate_code_pos_directives()
{
    for (auto &line : m_parser_tokens) {
        auto element = std::find_if(line.begin(), line.end(), [](const auto &token) {
            return token.kind() == pt_kind::DIRECTIVE and
                   token.v_directive() == parser_token::pt_directive::CODE_POSITION;
        });

        auto ast = element;
        if (element == line.end()) continue;
        auto ln_end = line.end();
        std::advance(element, 1);

        if (element == ln_end) {
            add_exception("Error at line " + std::to_string(element->row())
                          + ":\nA \'=\' was expected, but NEW LINE was found");
            continue;
        }
        if (element->kind() != pt_kind::EQUALS) {
            add_exception("Error at [" + std::to_string(element->row()) + ", " + std::to_string(element->column())
                          + "]:\nA \'=\' was expected, but " + parser_token::pt_kind_to_string(element->kind())
                          + " was found");
            continue;
        }

        std::advance(element, 1);
        if (element == ln_end) {
            add_exception("Error at line " + std::to_string(element->row())
                          + ":\nA NUMBER was expected, but NEW LINE was found");
            continue;
        }
        if (element->kind() != pt_kind::NUMBER) {
            add_exception("Error at [" + std::to_string(element->row()) + ", " + std::to_string(element->column())
                          + "]:\nA NUMBER was expected, but " + parser_token::pt_kind_to_string(element->kind())
                          + " was found");
            continue;
        }

        ast->v_number(element->v_number());

        std::advance(element, 1);

        if (element != ln_end) {
            add_exception("Error at [" + std::to_string(element->row()) + ", " + std::to_string(element->column())
                          + "]:\nA NEW LINE was expected, but " + parser_token::pt_kind_to_string(element->kind())
                          + " was found");
            continue;
        }
    }

    // REMOVE "=$NUMBER"
    for (auto &line : m_parser_tokens) {
        auto element = std::find_if(line.begin(), line.end(), [](const auto &token) {
            return token.kind() == pt_kind::DIRECTIVE and
                   token.v_directive() == parser_token::pt_directive::CODE_POSITION;
        });

        if (element == line.end()) continue;
        std::advance(element, 1);
        line.erase(element, line.end());
    }
}

void                   parser::
find_byte_lines()
{
    for (auto &line: m_parser_tokens) {
        auto element = std::find_if(line.begin(), line.end(), [](const auto &token) {
            return token.kind() == pt_kind::DIRECTIVE and
            (
                token.v_directive() == parser_token::pt_directive::BYTE or
                token.v_directive() == parser_token::pt_directive::WORD
            );
        });

        auto opc = element;
        if (element == line.end()) continue;

        std::advance(element, 1);
        if (element == line.end()) {
            add_exception("Error at line " + std::to_string(opc->row())
                          + ":\nA NUMBER or STRING was expected, but NEW LINE was found");
            continue;
        }

//TODO HERE
        std::vector<word_t> byte_line{};
        do {
            if (element->kind() != pt_kind::NUMBER and element->kind() != pt_kind::STRING) {
                add_exception("Error at [" + std::to_string(element->row()) + ", " + std::to_string(element->column())
                              + "]:\nA NUMBER or STRING was expected, but " +
                              parser_token::pt_kind_to_string(element->kind()) + " was found");
                goto _end;
            }
            if (element->kind() == pt_kind::STRING) {
                for (const auto &c : element->v_lexeme()) {
                    byte_line.push_back(c);
                }
            } else {
                if (opc->v_directive() == parser_token::pt_directive::BYTE and element->v_number() > 0xFF) {
                    add_exception("Error at [" + std::to_string(element->row()) + ", " + std::to_string(element->column())
                                  + "]:\nConstant, defined by BYTE should not be greater than 0xFF");
                    goto _end;
                } else if (opc->v_directive() == parser_token::pt_directive::WORD and element->v_number() > 0xFF'FF) {
                    add_exception("Error at [" + std::to_string(element->row()) + ", " + std::to_string(element->column())
                                  + "]:\nConstant, defined by WORD should not be greater than 0xFF'FF");
                    goto _end;
                }
                byte_line.push_back(element->v_number());
            }

            std::advance(element, 1);
            if (element == line.end()) {
                goto _end;
            }
            if (element->kind() != pt_kind::COMMA) {
                add_exception("Error at [" + std::to_string(element->row()) + ", " + std::to_string(element->column())
                              + "]:\nA COMMA was expected, but " +
                              parser_token::pt_kind_to_string(element->kind()) + " was found");
                goto _end;
            }

            std::advance(element, 1);
            if (element == line.end()) {
                add_exception("Error at line " + std::to_string(opc->row())
                              + ":\nA NUMBER or STRING was expected, but NEW LINE was found");
                goto _end;
            }
            if (element->kind() != pt_kind::NUMBER and element->kind() != pt_kind::STRING) {
                add_exception("Error at [" + std::to_string(element->row()) + ", " + std::to_string(element->column())
                              + "]:\nA NUMBER or STRING was expected, but " +
                              parser_token::pt_kind_to_string(element->kind()) + " was found");
                goto _end;
            }

        } while (element != line.end());
_end:
        opc->v_byteline(byte_line);
    }

    for (auto &line : m_parser_tokens) {
        auto element = std::find_if(line.begin(), line.end(), [](const auto &token) {
            return token.kind() == pt_kind::DIRECTIVE and
                   (
                           token.v_directive() == parser_token::pt_directive::BYTE or
                           token.v_directive() == parser_token::pt_directive::WORD
                   );
        });

        if (element == line.end()) continue;
        std::advance(element, 1);
        line.erase(element, line.end());
    }
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

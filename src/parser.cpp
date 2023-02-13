/*-------------------------------*
 |        MOlex Assembler        |
 |             Parser            |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#include "../include/parser.hpp"

using namespace mxasm;
using lt_kind    = lexer_token::lt_kind;
using pt_kind    = parser_token::pt_kind;
using pt_opcode  = parser_token::pt_opcode;
using st_kind    = serializable_token::st_kind;
using st_command = serializable_token::st_command;


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


    // Check directives
    index_and_replace_constants();
    validate_numbers_size();
    validate_code_pos_directives();
    find_byte_lines();

    if (not m_exceptions.empty()) {
        throw std::move(m_exceptions);
    }

    // Rows can start from directive, opcode or label declaration
    for (const auto &l : m_parser_tokens) {
        switch (l.begin()->kind()) {
            case pt_kind::LABEL_DECLARATION:
            case pt_kind::DIRECTIVE:
                continue;
            case pt_kind::OPCODE:
                if (l.begin()->v_opcode() == parser_token::pt_opcode::REGISTER_X or
                    l.begin()->v_opcode() == parser_token::pt_opcode::REGISTER_Y) {
                    add_exception("Error at line " + std::to_string(l.begin()->row()) + ":\nLine can starts from " \
                                  "DIRECTIVE, LABEL DECLARATION, or OPCODE, but "
                                  + parser_token::pt_opcode_to_string(l.begin()->v_opcode())
                                  + " was found");
                }
                continue;
            default:
                add_exception("Error at line " + std::to_string(l.begin()->row()) + ":\nLine can starts from " \
                              "DIRECTIVE, LABEL DECLARATION, or OPCODE, but "
                              + parser_token::pt_kind_to_string(l.begin()->kind()) + " was found");
                continue;
        }
    }

    if (not m_exceptions.empty()) {
        throw std::move(m_exceptions);
    }

    validate_and_replace_labels();
    if (not m_exceptions.empty()) {
        throw std::move(m_exceptions);
    }

    parser_tokens_to_serializable();
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
                                  + "]:\nNumerical constant can't be greater than 0xFF\'FF");
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
validate_and_replace_labels()
{
    std::map<std::string, std::size_t> label_indexes;

    std::size_t label_id {0};
    for (auto &line : m_parser_tokens) {
        if (line.begin()->kind() != pt_kind::LABEL_DECLARATION) continue;

        std::string label_name = to_lower(line.begin()->v_lexeme());
        if (label_indexes.contains(label_name)) {
            add_exception("Error at line " + std::to_string(line.begin()->row())
                          + ":\nLabel " + line.begin()->v_lexeme() + " is actually exists");
            continue;
        }
        line.begin()->v_number(label_id);
        label_indexes.emplace(label_name, label_id);
        ++label_id;

        auto ti = std::next(line.begin());
        if (ti != line.end() and ti->kind() != pt_kind::DIRECTIVE) {
            if (ti->kind() != pt_kind::OPCODE) {
                add_exception("Error at line " + std::to_string(line.begin()->row())
                              + ":\nExpected NEW LINE, OPCODE, or DIRECTIVE, but "
                              + parser_token::pt_kind_to_string(ti->kind()) + " was found");
            }
            if (ti->v_opcode() == pt_opcode::REGISTER_X or ti->v_opcode() == pt_opcode::REGISTER_Y) {
                add_exception("Error at [" + std::to_string(ti->row()) + ", "
                              + std::to_string(ti->column()) + "]: Expected NEW LINE, OPCODE, or DIRECTIVE, but " \
                              "REGISTER NAME was found");
            }
        }
    }

    for (auto &line : m_parser_tokens) {
        for (auto &token : line) {
            if (token.kind() == pt_kind::LABEL_CALL) {
                std::string lex = to_lower(token.v_lexeme());
                if (label_indexes.contains(lex)) {
                    token.v_number(label_indexes.at(lex));
                    continue;
                }
                add_exception("Error at [" + std::to_string(token.row()) + ", " + std::to_string(token.column())
                              + "]: Non-existed label call\n" + token.v_lexeme());
            }
        }
    }
}
#include <iomanip> //TODO: REMOVE THIS LIB
#include <iostream>
void                    parser::
parser_tokens_to_serializable()
{
    for (auto &line : m_parser_tokens) {
        auto iter = line.begin();
        auto iend = line.end();

        if (iter->kind() == pt_kind::LABEL_DECLARATION) {
            serializable_token stoken(st_kind::LABEL);
            stoken.number(iter->v_number());
            std::advance(iter, 1);// TODO CHECK THIS
            if (iter == iend or iter->kind() == pt_kind::DIRECTIVE or iter->kind() == pt_kind::OPCODE) continue;
            add_exception("Error at line " + std::to_string(iter->row()) + ":\nA NEW LINE, DIRECTIVE, or OPCODE " \
                          "was expected, but " + parser_token::pt_kind_to_string(iter->kind()) + " was found");
        }

        if (iter->kind() == pt_kind::DIRECTIVE) {
            switch (iter->v_directive()) {
                case parser_token::pt_directive::CODE_POSITION: d_code_pos(iter, iend); break;
                case parser_token::pt_directive::BYTE: d_byte(iter, iend); break;
                case parser_token::pt_directive::WORD: d_word(iter, iend); break;
            }
            continue;
        }

        if (iter->kind() == pt_kind::OPCODE) {
            switch (iter->v_opcode()) {
                case pt_opcode::BRK: o_brk(iter, iend); break;

                case pt_opcode::CLC: o_clc(iter, iend); break;
                case pt_opcode::CLD: o_cld(iter, iend); break;
                case pt_opcode::CLI: o_cli(iter, iend); break;
                case pt_opcode::CLV: o_clv(iter, iend); break;

                case pt_opcode::NOP: o_nop(iter, iend); break;
            }
            continue;
        }

        add_exception("Unknown token at [" + std::to_string(iter->row()) + ", " + std::to_string(iter->column())
                      + "]:\nCommand can starts from OPCODE, LABEL DECLARATION, or DIRECTIVE, but "
                      + parser_token::pt_kind_to_string(iter->kind()) + " was found");
    }
}

void                    parser::
validate_end_of_command(const std::list<parser_token>::const_iterator &iter,
                        const std::list<parser_token>::const_iterator &end)
{
    auto nxt = std::next(iter);
    if (nxt == end) return;
    add_exception("Error at line " + std::to_string(iter->row()) + ":\nA NEW LINE was expected, but "
                  + parser_token::pt_kind_to_string(nxt->kind()) + " was found");
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



void                    parser::
d_code_pos(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::CODE_POS);
    stoken.number(beg->v_number());
    validate_end_of_command(beg, end);
    m_tokens.push_back(stoken);
}

void                    parser::
d_byte(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::BYTE);
    stoken.byteline(beg->v_byteline());
    validate_end_of_command(beg, end);
    m_tokens.push_back(stoken);
}

void                    parser::
d_word(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::WORD);
    stoken.byteline(beg->v_byteline());
    validate_end_of_command(beg, end);
    m_tokens.push_back(stoken);
}


void                    parser::
o_brk(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end) noexcept
{
    serializable_token stoken(serializable_token::st_kind::OPCODE);
    stoken.command(serializable_token::st_command::BRK_stk);
    validate_end_of_command(beg, end);
    m_tokens.push_back(stoken);
}



void                    parser::
o_clc(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end) noexcept
{
    serializable_token stoken(serializable_token::st_kind::OPCODE);
    stoken.command(serializable_token::st_command::CLC_imp);
    validate_end_of_command(beg, end);
    m_tokens.push_back(stoken);
}

void                    parser::
o_cld(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end) noexcept
{
    serializable_token stoken(serializable_token::st_kind::OPCODE);
    stoken.command(serializable_token::st_command::CLD_imp);
    validate_end_of_command(beg, end);
    m_tokens.push_back(stoken);
}

void                    parser::
o_cli(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end) noexcept
{
    serializable_token stoken(serializable_token::st_kind::OPCODE);
    stoken.command(serializable_token::st_command::CLI_imp);
    validate_end_of_command(beg, end);
    m_tokens.push_back(stoken);
}

void                    parser::
o_clv(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end) noexcept
{
    serializable_token stoken(serializable_token::st_kind::OPCODE);
    stoken.command(serializable_token::st_command::CLV_imp);
    validate_end_of_command(beg, end);
    m_tokens.push_back(stoken);
}

void                    parser::
o_nop(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end) noexcept
{
    serializable_token stoken(serializable_token::st_kind::OPCODE);
    stoken.command(serializable_token::st_command::NOP_imp);
    validate_end_of_command(beg, end);
    m_tokens.push_back(stoken);
}
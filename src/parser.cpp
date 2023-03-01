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
using adr_mode   = parser_token::adr_mode;


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
                    l.begin()->v_opcode() == parser_token::pt_opcode::REGISTER_Y or
                    l.begin()->v_opcode() == parser_token::pt_opcode::REGISTER_A) {
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
    auto list_iter = m_parser_tokens.begin();
    for (auto &line : m_parser_tokens) {
        ++list_iter;
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
            if (ti->v_opcode() == pt_opcode::REGISTER_X or ti->v_opcode() == pt_opcode::REGISTER_Y or
                ti->v_opcode() == pt_opcode::REGISTER_A) {
                add_exception("Error at [" + std::to_string(ti->row()) + ", "
                              + std::to_string(ti->column()) + "]: Expected NEW LINE, OPCODE, or DIRECTIVE, but " \
                              "REGISTER NAME was found");
            }
        }
        if (ti != line.end()) {
            m_parser_tokens.insert(list_iter, {ti, line.end()});
        }
        line.erase(ti, line.end());
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


#include <iomanip> //TODO: REMOVE THIS LIB
#include <iostream>
void                    parser::
parser_tokens_to_serializable()
{
    for (auto &line : m_parser_tokens) {
        auto iter = line.begin();
        auto iend = line.end();

        if (iter->kind() == pt_kind::LABEL_DECLARATION) {
            l_decl(iter, iend);
            continue;
        }

        if (iter->kind() == pt_kind::DIRECTIVE) {
            switch (iter->v_directive()) {
                case parser_token::pt_directive::CODE_POSITION: d_code_pos(iter, iend); break;
                case parser_token::pt_directive::BYTE: d_byteline(iter, iend, st_kind::BYTE); break;
                case parser_token::pt_directive::WORD: d_byteline(iter, iend, st_kind::WORD); break;
            }
            continue;
        }

        if (iter->kind() == pt_kind::OPCODE) {
            switch (iter->v_opcode()) {
                case pt_opcode::BRK: o_opc_imp(iter, iend, serializable_token::st_command::BRK_stk); break;
                case pt_opcode::CLC: o_opc_imp(iter, iend, serializable_token::st_command::CLC_imp); break;
                case pt_opcode::CLD: o_opc_imp(iter, iend, serializable_token::st_command::CLD_imp); break;
                case pt_opcode::CLI: o_opc_imp(iter, iend, serializable_token::st_command::CLI_imp); break;
                case pt_opcode::CLV: o_opc_imp(iter, iend, serializable_token::st_command::CLV_imp); break;
                case pt_opcode::DEX: o_opc_imp(iter, iend, serializable_token::st_command::DEX_imp); break;
                case pt_opcode::DEY: o_opc_imp(iter, iend, serializable_token::st_command::DEY_imp); break;
                case pt_opcode::INX: o_opc_imp(iter, iend, serializable_token::st_command::INX_imp); break;
                case pt_opcode::INY: o_opc_imp(iter, iend, serializable_token::st_command::INY_imp); break;
                case pt_opcode::NOP: o_opc_imp(iter, iend, serializable_token::st_command::NOP_imp); break;
                case pt_opcode::PHA: o_opc_imp(iter, iend, serializable_token::st_command::PHA_stk); break;
                case pt_opcode::PHP: o_opc_imp(iter, iend, serializable_token::st_command::PHP_stk); break;
                case pt_opcode::PHX: o_opc_imp(iter, iend, serializable_token::st_command::PHX_stk); break;
                case pt_opcode::PHY: o_opc_imp(iter, iend, serializable_token::st_command::PHY_stk); break;
                case pt_opcode::PLA: o_opc_imp(iter, iend, serializable_token::st_command::PLA_stk); break;
                case pt_opcode::PLP: o_opc_imp(iter, iend, serializable_token::st_command::PLP_stk); break;
                case pt_opcode::PLX: o_opc_imp(iter, iend, serializable_token::st_command::PLX_stk); break;
                case pt_opcode::PLY: o_opc_imp(iter, iend, serializable_token::st_command::PLY_stk); break;
                case pt_opcode::RTS: o_opc_imp(iter, iend, serializable_token::st_command::RTS_stk); break;
                case pt_opcode::RTI: o_opc_imp(iter, iend, serializable_token::st_command::RTI_stk); break;
                case pt_opcode::SEC: o_opc_imp(iter, iend, serializable_token::st_command::SEC_imp); break;
                case pt_opcode::SED: o_opc_imp(iter, iend, serializable_token::st_command::SED_imp); break;
                case pt_opcode::SEI: o_opc_imp(iter, iend, serializable_token::st_command::SEI_imp); break;
                case pt_opcode::STP: o_opc_imp(iter, iend, serializable_token::st_command::STP_imp); break;
                case pt_opcode::TAX: o_opc_imp(iter, iend, serializable_token::st_command::TAX_imp); break;
                case pt_opcode::TAY: o_opc_imp(iter, iend, serializable_token::st_command::TAY_imp); break;
                case pt_opcode::TSX: o_opc_imp(iter, iend, serializable_token::st_command::TSX_imp); break;
                case pt_opcode::TXA: o_opc_imp(iter, iend, serializable_token::st_command::TXA_imp); break;
                case pt_opcode::TXS: o_opc_imp(iter, iend, serializable_token::st_command::TXS_imp); break;
                case pt_opcode::TYA: o_opc_imp(iter, iend, serializable_token::st_command::TYA_imp); break;
                case pt_opcode::WAI: o_opc_imp(iter, iend, serializable_token::st_command::WAI_imp); break;

                case pt_opcode::JMP: o_jmp(iter, iend); break;
                case pt_opcode::JSR: o_jsr(iter, iend); break;
                case pt_opcode::ASL: o_asl(iter, iend); break;
                case pt_opcode::INC: o_inc(iter, iend); break;
                case pt_opcode::ROL: o_rol(iter, iend); break;
                case pt_opcode::DEC: o_dec(iter, iend); break;
                case pt_opcode::LSR: o_lsr(iter, iend); break;
                case pt_opcode::ROR: o_ror(iter, iend); break;
                case pt_opcode::STX: o_stx(iter, iend); break;
                case pt_opcode::LDX: o_ldx(iter, iend); break;
                case pt_opcode::TSB: o_tsb(iter, iend); break;
                case pt_opcode::ORA: o_ora(iter, iend); break;
                case pt_opcode::TRB: o_trb(iter, iend); break;
                case pt_opcode::BIT: o_bit(iter, iend); break;
                case pt_opcode::AND: o_and(iter, iend); break;
                case pt_opcode::EOR: o_eor(iter, iend); break;
                case pt_opcode::ADC: o_adc(iter, iend); break;
                case pt_opcode::CPX: o_cpx(iter, iend); break;
                case pt_opcode::SBC: o_sbc(iter, iend); break;
                case pt_opcode::STY: o_sty(iter, iend); break;
                case pt_opcode::STA: o_sta(iter, iend); break;
                case pt_opcode::STZ: o_stz(iter, iend); break;
                case pt_opcode::LDY: o_ldy(iter, iend); break;
                case pt_opcode::LDA: o_lda(iter, iend); break;
                case pt_opcode::CPY: o_cpy(iter, iend); break;
                case pt_opcode::CMP: o_cmp(iter, iend); break;

                case pt_opcode::BPL: o_opc_rel(iter, iend, st_command::BPL_rel); break;
                case pt_opcode::BMI: o_opc_rel(iter, iend, st_command::BMI_rel); break;
                case pt_opcode::BVC: o_opc_rel(iter, iend, st_command::BVC_rel); break;
                case pt_opcode::BVS: o_opc_rel(iter, iend, st_command::BVS_rel); break;
                case pt_opcode::BRA: o_opc_rel(iter, iend, st_command::BRA_rel); break;
                case pt_opcode::BCC: o_opc_rel(iter, iend, st_command::BCC_rel); break;
                case pt_opcode::BCS: o_opc_rel(iter, iend, st_command::BCS_rel); break;
                case pt_opcode::BNE: o_opc_rel(iter, iend, st_command::BNE_rel); break;
                case pt_opcode::BEQ: o_opc_rel(iter, iend, st_command::BEQ_rel); break;

                case pt_opcode::RMB0: o_rmb_or_smb(iter, iend, st_command::RMB0_zpg); break;
                case pt_opcode::RMB1: o_rmb_or_smb(iter, iend, st_command::RMB1_zpg); break;
                case pt_opcode::RMB2: o_rmb_or_smb(iter, iend, st_command::RMB2_zpg); break;
                case pt_opcode::RMB3: o_rmb_or_smb(iter, iend, st_command::RMB3_zpg); break;
                case pt_opcode::RMB4: o_rmb_or_smb(iter, iend, st_command::RMB4_zpg); break;
                case pt_opcode::RMB5: o_rmb_or_smb(iter, iend, st_command::RMB5_zpg); break;
                case pt_opcode::RMB6: o_rmb_or_smb(iter, iend, st_command::RMB6_zpg); break;
                case pt_opcode::RMB7: o_rmb_or_smb(iter, iend, st_command::RMB7_zpg); break;
                case pt_opcode::SMB0: o_rmb_or_smb(iter, iend, st_command::SMB0_zpg); break;
                case pt_opcode::SMB1: o_rmb_or_smb(iter, iend, st_command::SMB1_zpg); break;
                case pt_opcode::SMB2: o_rmb_or_smb(iter, iend, st_command::SMB2_zpg); break;
                case pt_opcode::SMB3: o_rmb_or_smb(iter, iend, st_command::SMB3_zpg); break;
                case pt_opcode::SMB4: o_rmb_or_smb(iter, iend, st_command::SMB4_zpg); break;
                case pt_opcode::SMB5: o_rmb_or_smb(iter, iend, st_command::SMB5_zpg); break;
                case pt_opcode::SMB6: o_rmb_or_smb(iter, iend, st_command::SMB6_zpg); break;
                case pt_opcode::SMB7: o_rmb_or_smb(iter, iend, st_command::SMB7_zpg); break;

                case pt_opcode::BBR0: o_bbr_or_bbs(iter, iend, st_command::BBR0_zpr); break;
                case pt_opcode::BBR1: o_bbr_or_bbs(iter, iend, st_command::BBR1_zpr); break;
                case pt_opcode::BBR2: o_bbr_or_bbs(iter, iend, st_command::BBR2_zpr); break;
                case pt_opcode::BBR3: o_bbr_or_bbs(iter, iend, st_command::BBR3_zpr); break;
                case pt_opcode::BBR4: o_bbr_or_bbs(iter, iend, st_command::BBR4_zpr); break;
                case pt_opcode::BBR5: o_bbr_or_bbs(iter, iend, st_command::BBR5_zpr); break;
                case pt_opcode::BBR6: o_bbr_or_bbs(iter, iend, st_command::BBR6_zpr); break;
                case pt_opcode::BBR7: o_bbr_or_bbs(iter, iend, st_command::BBR7_zpr); break;
                case pt_opcode::BBS0: o_bbr_or_bbs(iter, iend, st_command::BBS0_zpr); break;
                case pt_opcode::BBS1: o_bbr_or_bbs(iter, iend, st_command::BBS1_zpr); break;
                case pt_opcode::BBS2: o_bbr_or_bbs(iter, iend, st_command::BBS2_zpr); break;
                case pt_opcode::BBS3: o_bbr_or_bbs(iter, iend, st_command::BBS3_zpr); break;
                case pt_opcode::BBS4: o_bbr_or_bbs(iter, iend, st_command::BBS4_zpr); break;
                case pt_opcode::BBS5: o_bbr_or_bbs(iter, iend, st_command::BBS5_zpr); break;
                case pt_opcode::BBS6: o_bbr_or_bbs(iter, iend, st_command::BBS6_zpr); break;
                case pt_opcode::BBS7: o_bbr_or_bbs(iter, iend, st_command::BBS7_zpr); break;

            }
            continue;
        }

        add_exception("Unknown token at [" + std::to_string(iter->row()) + ", " + std::to_string(iter->column())
                      + "]:\nCommand can starts from OPCODE, LABEL DECLARATION, or DIRECTIVE, but "
                      + parser_token::pt_kind_to_string(iter->kind()) + " was found");
    }
}

void                    parser::
l_decl(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::LABEL);
    stoken.number(beg->v_number());
    validate_end_of_command(beg, end);
    m_tokens.push_back(stoken);
}

void                    parser::
d_code_pos(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::CODE_POS);
    stoken.number(beg->v_number());
    validate_end_of_command(beg, end);
    m_tokens.push_back(stoken);
}

void                    parser::
d_byteline(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end,
           serializable_token::st_kind dir_type)
{
    serializable_token stoken(dir_type);
    stoken.byteline(beg->v_byteline());
    validate_end_of_command(beg, end);
    m_tokens.push_back(stoken);
}

void                    parser::
o_opc_imp(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end,
          serializable_token::st_command opc)
{
    serializable_token stoken(serializable_token::st_kind::OPCODE);
    stoken.command(opc);
    validate_end_of_command(beg, end);
    m_tokens.push_back(stoken);
}

void                    parser::
o_opc_rel(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end,
          serializable_token::st_command opc)
{
    serializable_token stoken(st_kind::OPCODE);
    if (define_addr_mode(std::next(beg), end) == adr_mode::ABS_or_REL) {
        stoken.command(opc);
        stoken.number(std::next(beg)->v_number());
        if (std::next(beg)->kind() != pt_kind::LABEL_CALL) goto _err;
    } else {
_err:
        add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for " +
        parser_token::pt_kind_to_string(beg->kind()));
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_rmb_or_smb(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end,
             serializable_token::st_command opc)
{
    serializable_token stoken(st_kind::OPCODE);
    if (define_addr_mode(std::next(beg), end) == adr_mode::ZP) {
        stoken.command(opc);
        stoken.number(std::next(beg)->v_number());
    } else {
        add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for RMB or SMB");
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_bbr_or_bbs(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end,
             serializable_token::st_command opc)
{
    serializable_token stoken(st_kind::OPCODE);
    if (define_addr_mode(std::next(beg), end) == adr_mode::ZP_REL) {
        stoken.command(opc);
        std::advance(beg, 1);
        stoken.number(beg->v_number());
        std::advance(beg, 2);
        stoken.byteline({word_t(beg->v_number())});
    } else {
        add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for BBR or BBS");
    }
    m_tokens.push_back(stoken);
}


// ------------------------------------- INSTRUCTIONS -------------------------------------
void                    parser::
o_jmp(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::OPCODE);
    switch (define_addr_mode(std::next(beg), end)) {
        case adr_mode::ABS_or_REL:
            stoken.command(st_command::JMP_abs);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;


        default:
            add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for JMP");
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_jsr(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::OPCODE);
    if (define_addr_mode(std::next(beg), end) == adr_mode::ABS_or_REL) {
        stoken.command(st_command::JSR_abs);
        stoken.number(std::next(beg)->v_number());
        if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
        else { stoken.labelable(false); }
    } else {
        add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for JSR");
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_asl(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::OPCODE);
    switch (define_addr_mode(std::next(beg), end)) {
        case adr_mode::ABS_or_REL:
            stoken.command(st_command::ASL_abs);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::A:
        case adr_mode::STK_or_IMP:
            stoken.command(st_command::ASL_a);
            break;
        case adr_mode::ZP:
            stoken.command(st_command::ASL_zpg);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ZP_X:
            stoken.command(st_command::ASL_zpx);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ABS_X:
            stoken.command(st_command::ASL_abx);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        default:
            add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for ASL");
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_inc(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::OPCODE);
    switch (define_addr_mode(std::next(beg), end)) {
        case adr_mode::ABS_or_REL:
            stoken.command(st_command::INC_abs);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::A:
        case adr_mode::STK_or_IMP:
            stoken.command(st_command::INC_a);
            break;
        case adr_mode::ZP:
            stoken.command(st_command::INC_zpg);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ZP_X:
            stoken.command(st_command::INC_zpx);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ABS_X:
            stoken.command(st_command::INC_abx);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        default:
            add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for INC");
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_rol(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::OPCODE);
    switch (define_addr_mode(std::next(beg), end)) {
        case adr_mode::ABS_or_REL:
            stoken.command(st_command::ROL_abs);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::A:
        case adr_mode::STK_or_IMP:
            stoken.command(st_command::ROL_a);
            break;
        case adr_mode::ZP:
            stoken.command(st_command::ROL_zpg);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ZP_X:
            stoken.command(st_command::ROL_zpx);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ABS_X:
            stoken.command(st_command::ROL_abx);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        default:
            add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for ROL");
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_dec(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::OPCODE);
    switch (define_addr_mode(std::next(beg), end)) {
        case adr_mode::ABS_or_REL:
            stoken.command(st_command::DEC_abs);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::A:
        case adr_mode::STK_or_IMP:
            stoken.command(st_command::DEC_a);
            break;
        case adr_mode::ZP:
            stoken.command(st_command::DEC_zpg);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ZP_X:
            stoken.command(st_command::DEC_zpx);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ABS_X:
            stoken.command(st_command::DEC_abx);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        default:
            add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for DEC");
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_lsr(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::OPCODE);
    switch (define_addr_mode(std::next(beg), end)) {
        case adr_mode::ABS_or_REL:
            stoken.command(st_command::LSR_abs);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::A:
        case adr_mode::STK_or_IMP:
            stoken.command(st_command::LSR_a);
            break;
        case adr_mode::ZP:
            stoken.command(st_command::LSR_zpg);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ZP_X:
            stoken.command(st_command::LSR_zpx);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ABS_X:
            stoken.command(st_command::LSR_abx);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        default:
            add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for LSR");
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_ror(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::OPCODE);
    switch (define_addr_mode(std::next(beg), end)) {
        case adr_mode::ABS_or_REL:
            stoken.command(st_command::ROR_abs);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::A:
        case adr_mode::STK_or_IMP:
            stoken.command(st_command::ROR_a);
            break;
        case adr_mode::ZP:
            stoken.command(st_command::ROR_zpg);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ZP_X:
            stoken.command(st_command::ROR_zpx);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ABS_X:
            stoken.command(st_command::ROR_abx);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        default:
            add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for ROR");
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_stx(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::OPCODE);
    switch (define_addr_mode(std::next(beg), end)) {
        case adr_mode::ABS_or_REL:
            stoken.command(st_command::STX_abs);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ZP:
            stoken.command(st_command::STX_zpg);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ZP_Y:
            stoken.command(st_command::STX_zpy);
            stoken.number(std::next(beg)->v_number());
            break;
        default:
            add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for STX");
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_ldx(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::OPCODE);
    switch (define_addr_mode(std::next(beg), end)) {
        case adr_mode::ABS_or_REL:
            stoken.command(st_command::LDX_abs);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ZP:
            stoken.command(st_command::LDX_zpg);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ZP_Y:
            stoken.command(st_command::LDX_zpy);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ABS_Y:
            stoken.command(st_command::LDX_aby);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::IMM:
            stoken.command(st_command::LDX_imm);
            std::advance(beg, 2);
            if (beg->kind() == pt_kind::NUMBER) {
                stoken.number(beg->v_number());
            } else if (beg->kind() == pt_kind::LESS) {
                stoken.number(std::next(beg)->v_number());
                stoken.byteline({'<'});
            } else if (beg->kind() == pt_kind::GREATER) {
                stoken.number(std::next(beg)->v_number());
                stoken.byteline({'>'});
            }
            break;
        default:
            add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for LDX");
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_tsb(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::OPCODE);
    switch (define_addr_mode(std::next(beg), end)) {
        case adr_mode::ABS_or_REL:
            stoken.command(st_command::TSB_abs);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ZP:
            stoken.command(st_command::TSB_zpg);
            stoken.number(std::next(beg)->v_number());
            break;
        default:
            add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for TSB");
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_ora(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::OPCODE);
    switch (define_addr_mode(std::next(beg), end)) {
        case adr_mode::ABS_or_REL:
            stoken.command(st_command::ORA_abs);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ZP:
            stoken.command(st_command::ORA_zpg);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ZP_X:
            stoken.command(st_command::ORA_zpx);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ABS_X:
            stoken.command(st_command::ORA_abx);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ABS_Y:
            stoken.command(st_command::ORA_aby);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::IMM:
            stoken.command(st_command::ORA_imm);
            std::advance(beg, 2);
            if (beg->kind() == pt_kind::NUMBER) {
                stoken.number(beg->v_number());
            } else if (beg->kind() == pt_kind::LESS) {
                stoken.number(std::next(beg)->v_number());
                stoken.byteline({'<'});
            } else if (beg->kind() == pt_kind::GREATER) {
                stoken.number(std::next(beg)->v_number());
                stoken.byteline({'>'});
            }
            break;
        case adr_mode::ZP_IND:
            stoken.command(st_command::ORA_izp);
            std::advance(beg, 2);
            stoken.number(beg->v_number());
            break;
        case adr_mode::ZP_IND_Y:
            stoken.command(st_command::ORA_izy);
            std::advance(beg, 2);
            stoken.number(beg->v_number());
            break;
        case adr_mode::ZP_X_IND:
            stoken.command(st_command::ORA_izx);
            std::advance(beg, 2);
            stoken.number(beg->v_number());
            break;
        default:
            add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for ORA");
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_trb(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::OPCODE);
    switch (define_addr_mode(std::next(beg), end)) {
        case adr_mode::ABS_or_REL:
            stoken.command(st_command::TRB_abs);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ZP:
            stoken.command(st_command::TRB_zpg);
            stoken.number(std::next(beg)->v_number());
            break;
        default:
            add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for TRB");
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_bit(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::OPCODE);
    switch (define_addr_mode(std::next(beg), end)) {
        case adr_mode::ABS_or_REL:
            stoken.command(st_command::BIT_abs);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ZP:
            stoken.command(st_command::BIT_zpg);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ZP_X:
            stoken.command(st_command::BIT_zpx);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ABS_X:
            stoken.command(st_command::BIT_abx);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::IMM:
            stoken.command(st_command::BIT_imm);
            std::advance(beg, 2);
            if (beg->kind() == pt_kind::NUMBER) {
                stoken.number(beg->v_number());
            } else if (beg->kind() == pt_kind::LESS) {
                stoken.number(std::next(beg)->v_number());
                stoken.byteline({'<'});
            } else if (beg->kind() == pt_kind::GREATER) {
                stoken.number(std::next(beg)->v_number());
                stoken.byteline({'>'});
            }
            break;
        default:
            add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for BIT");
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_and(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::OPCODE);
    switch (define_addr_mode(std::next(beg), end)) {
        case adr_mode::ABS_or_REL:
            stoken.command(st_command::AND_abs);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ZP:
            stoken.command(st_command::AND_zpg);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ZP_X:
            stoken.command(st_command::AND_zpx);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ABS_X:
            stoken.command(st_command::AND_abx);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ABS_Y:
            stoken.command(st_command::AND_aby);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::IMM:
            stoken.command(st_command::AND_imm);
            std::advance(beg, 2);
            if (beg->kind() == pt_kind::NUMBER) {
                stoken.number(beg->v_number());
            } else if (beg->kind() == pt_kind::LESS) {
                stoken.number(std::next(beg)->v_number());
                stoken.byteline({'<'});
            } else if (beg->kind() == pt_kind::GREATER) {
                stoken.number(std::next(beg)->v_number());
                stoken.byteline({'>'});
            }
            break;
        case adr_mode::ZP_IND:
            stoken.command(st_command::AND_izp);
            std::advance(beg, 2);
            stoken.number(beg->v_number());
            break;
        case adr_mode::ZP_IND_Y:
            stoken.command(st_command::AND_izy);
            std::advance(beg, 2);
            stoken.number(beg->v_number());
            break;
        case adr_mode::ZP_X_IND:
            stoken.command(st_command::AND_izx);
            std::advance(beg, 2);
            stoken.number(beg->v_number());
            break;
        default:
            add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for AND");
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_eor(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::OPCODE);
    switch (define_addr_mode(std::next(beg), end)) {
        case adr_mode::ABS_or_REL:
            stoken.command(st_command::EOR_abs);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ZP:
            stoken.command(st_command::EOR_zpg);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ZP_X:
            stoken.command(st_command::EOR_zpx);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ABS_X:
            stoken.command(st_command::EOR_abx);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ABS_Y:
            stoken.command(st_command::EOR_aby);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::IMM:
            stoken.command(st_command::EOR_imm);
            std::advance(beg, 2);
            if (beg->kind() == pt_kind::NUMBER) {
                stoken.number(beg->v_number());
            } else if (beg->kind() == pt_kind::LESS) {
                stoken.number(std::next(beg)->v_number());
                stoken.byteline({'<'});
            } else if (beg->kind() == pt_kind::GREATER) {
                stoken.number(std::next(beg)->v_number());
                stoken.byteline({'>'});
            }
            break;
        case adr_mode::ZP_IND:
            stoken.command(st_command::EOR_izp);
            std::advance(beg, 2);
            stoken.number(beg->v_number());
            break;
        case adr_mode::ZP_IND_Y:
            stoken.command(st_command::EOR_izy);
            std::advance(beg, 2);
            stoken.number(beg->v_number());
            break;
        case adr_mode::ZP_X_IND:
            stoken.command(st_command::EOR_izx);
            std::advance(beg, 2);
            stoken.number(beg->v_number());
            break;
        default:
            add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for EOR");
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_adc(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::OPCODE);
    switch (define_addr_mode(std::next(beg), end)) {
        case adr_mode::ABS_or_REL:
            stoken.command(st_command::ADC_abs);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ZP:
            stoken.command(st_command::ADC_zpg);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ZP_X:
            stoken.command(st_command::ADC_zpx);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ABS_X:
            stoken.command(st_command::ADC_abx);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ABS_Y:
            stoken.command(st_command::ADC_aby);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::IMM:
            stoken.command(st_command::ADC_imm);
            std::advance(beg, 2);
            if (beg->kind() == pt_kind::NUMBER) {
                stoken.number(beg->v_number());
            } else if (beg->kind() == pt_kind::LESS) {
                stoken.number(std::next(beg)->v_number());
                stoken.byteline({'<'});
            } else if (beg->kind() == pt_kind::GREATER) {
                stoken.number(std::next(beg)->v_number());
                stoken.byteline({'>'});
            }
            break;
        case adr_mode::ZP_IND:
            stoken.command(st_command::ADC_izp);
            std::advance(beg, 2);
            stoken.number(beg->v_number());
            break;
        case adr_mode::ZP_IND_Y:
            stoken.command(st_command::ADC_izy);
            std::advance(beg, 2);
            stoken.number(beg->v_number());
            break;
        case adr_mode::ZP_X_IND:
            stoken.command(st_command::ADC_izx);
            std::advance(beg, 2);
            stoken.number(beg->v_number());
            break;
        default:
            add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for ADC");
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_cpx(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::OPCODE);
    switch (define_addr_mode(std::next(beg), end)) {
        case adr_mode::ABS_or_REL:
            stoken.command(st_command::CPX_abs);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ZP:
            stoken.command(st_command::CPX_zpg);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::IMM:
            stoken.command(st_command::CPX_imm);
            std::advance(beg, 2);
            if (beg->kind() == pt_kind::NUMBER) {
                stoken.number(beg->v_number());
            } else if (beg->kind() == pt_kind::LESS) {
                stoken.number(std::next(beg)->v_number());
                stoken.byteline({'<'});
            } else if (beg->kind() == pt_kind::GREATER) {
                stoken.number(std::next(beg)->v_number());
                stoken.byteline({'>'});
            }
            break;
        default:
            add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for CPX");
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_sbc(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::OPCODE);
    switch (define_addr_mode(std::next(beg), end)) {
        case adr_mode::ABS_or_REL:
            stoken.command(st_command::SBC_abs);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ZP:
            stoken.command(st_command::SBC_zpg);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ZP_X:
            stoken.command(st_command::SBC_zpx);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ABS_X:
            stoken.command(st_command::SBC_abx);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ABS_Y:
            stoken.command(st_command::SBC_aby);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::IMM:
            stoken.command(st_command::SBC_imm);
            std::advance(beg, 2);
            if (beg->kind() == pt_kind::NUMBER) {
                stoken.number(beg->v_number());
            } else if (beg->kind() == pt_kind::LESS) {
                stoken.number(std::next(beg)->v_number());
                stoken.byteline({'<'});
            } else if (beg->kind() == pt_kind::GREATER) {
                stoken.number(std::next(beg)->v_number());
                stoken.byteline({'>'});
            }
            break;
        case adr_mode::ZP_IND:
            stoken.command(st_command::SBC_izp);
            std::advance(beg, 2);
            stoken.number(beg->v_number());
            break;
        case adr_mode::ZP_IND_Y:
            stoken.command(st_command::SBC_izy);
            std::advance(beg, 2);
            stoken.number(beg->v_number());
            break;
        case adr_mode::ZP_X_IND:
            stoken.command(st_command::SBC_izx);
            std::advance(beg, 2);
            stoken.number(beg->v_number());
            break;
        default:
            add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for SBC");
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_sty(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::OPCODE);
    switch (define_addr_mode(std::next(beg), end)) {
        case adr_mode::ABS_or_REL:
            stoken.command(st_command::STY_abs);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ZP:
            stoken.command(st_command::STY_zpg);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ZP_X:
            stoken.command(st_command::STY_zpx);
            stoken.number(std::next(beg)->v_number());
            break;
        default:
            add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for STY");
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_sta(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::OPCODE);
    switch (define_addr_mode(std::next(beg), end)) {
        case adr_mode::ABS_or_REL:
            stoken.command(st_command::STA_abs);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ZP:
            stoken.command(st_command::STA_zpg);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ZP_X:
            stoken.command(st_command::STA_zpx);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ABS_X:
            stoken.command(st_command::STA_abx);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ABS_Y:
            stoken.command(st_command::STA_aby);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ZP_IND:
            stoken.command(st_command::STA_izp);
            std::advance(beg, 2);
            stoken.number(beg->v_number());
            break;
        case adr_mode::ZP_IND_Y:
            stoken.command(st_command::STA_izy);
            std::advance(beg, 2);
            stoken.number(beg->v_number());
            break;
        case adr_mode::ZP_X_IND:
            stoken.command(st_command::STA_izx);
            std::advance(beg, 2);
            stoken.number(beg->v_number());
            break;
        default:
            add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for STA");
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_stz(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::OPCODE);
    switch (define_addr_mode(std::next(beg), end)) {
        case adr_mode::ABS_or_REL:
            stoken.command(st_command::STZ_abs);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ZP:
            stoken.command(st_command::STZ_zpg);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ZP_X:
            stoken.command(st_command::STZ_zpx);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ABS_X:
            stoken.command(st_command::STZ_abx);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        default:
            add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for STZ");
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_ldy(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::OPCODE);
    switch (define_addr_mode(std::next(beg), end)) {
        case adr_mode::ABS_or_REL:
            stoken.command(st_command::LDY_abs);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ZP:
            stoken.command(st_command::LDY_zpg);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ZP_X:
            stoken.command(st_command::LDY_zpx);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ABS_X:
            stoken.command(st_command::LDY_abx);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::IMM:
            stoken.command(st_command::LDY_imm);
            std::advance(beg, 2);
            if (beg->kind() == pt_kind::NUMBER) {
                stoken.number(beg->v_number());
            } else if (beg->kind() == pt_kind::LESS) {
                stoken.number(std::next(beg)->v_number());
                stoken.byteline({'<'});
            } else if (beg->kind() == pt_kind::GREATER) {
                stoken.number(std::next(beg)->v_number());
                stoken.byteline({'>'});
            }
            break;
        default:
            add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for LDY");
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_lda(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::OPCODE);
    switch (define_addr_mode(std::next(beg), end)) {
        case adr_mode::ABS_or_REL:
            stoken.command(st_command::LDA_abs);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ZP:
            stoken.command(st_command::LDA_zpg);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ZP_X:
            stoken.command(st_command::LDA_zpx);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ABS_X:
            stoken.command(st_command::LDA_abx);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ABS_Y:
            stoken.command(st_command::LDA_aby);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::IMM:
            stoken.command(st_command::LDA_imm);
            std::advance(beg, 2);
            if (beg->kind() == pt_kind::NUMBER) {
                stoken.number(beg->v_number());
            } else if (beg->kind() == pt_kind::LESS) {
                stoken.number(std::next(beg)->v_number());
                stoken.byteline({'<'});
            } else if (beg->kind() == pt_kind::GREATER) {
                stoken.number(std::next(beg)->v_number());
                stoken.byteline({'>'});
            }
            break;
        case adr_mode::ZP_IND:
            stoken.command(st_command::LDA_izp);
            std::advance(beg, 2);
            stoken.number(beg->v_number());
            break;
        case adr_mode::ZP_IND_Y:
            stoken.command(st_command::LDA_izy);
            std::advance(beg, 2);
            stoken.number(beg->v_number());
            break;
        case adr_mode::ZP_X_IND:
            stoken.command(st_command::LDA_izx);
            std::advance(beg, 2);
            stoken.number(beg->v_number());
            break;
        default:
            add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for LDA");
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_cpy(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::OPCODE);
    switch (define_addr_mode(std::next(beg), end)) {
        case adr_mode::ABS_or_REL:
            stoken.command(st_command::CPY_abs);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ZP:
            stoken.command(st_command::CPY_zpg);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::IMM:
            stoken.command(st_command::CPY_imm);
            std::advance(beg, 2);
            if (beg->kind() == pt_kind::NUMBER) {
                stoken.number(beg->v_number());
            } else if (beg->kind() == pt_kind::LESS) {
                stoken.number(std::next(beg)->v_number());
                stoken.byteline({'<'});
            } else if (beg->kind() == pt_kind::GREATER) {
                stoken.number(std::next(beg)->v_number());
                stoken.byteline({'>'});
            }
            break;
        default:
            add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for CPY");
    }
    m_tokens.push_back(stoken);
}

void                    parser::
o_cmp(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    serializable_token stoken(st_kind::OPCODE);
    switch (define_addr_mode(std::next(beg), end)) {
        case adr_mode::ABS_or_REL:
            stoken.command(st_command::CMP_abs);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ZP:
            stoken.command(st_command::CMP_zpg);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ZP_X:
            stoken.command(st_command::CMP_zpx);
            stoken.number(std::next(beg)->v_number());
            break;
        case adr_mode::ABS_X:
            stoken.command(st_command::CMP_abx);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::ABS_Y:
            stoken.command(st_command::CMP_aby);
            stoken.number(std::next(beg)->v_number());
            if (std::next(beg)->kind() == pt_kind::LABEL_CALL) { stoken.labelable(true); }
            else { stoken.labelable(false); }
            break;
        case adr_mode::IMM:
            stoken.command(st_command::CMP_imm);
            std::advance(beg, 2);
            if (beg->kind() == pt_kind::NUMBER) {
                stoken.number(beg->v_number());
            } else if (beg->kind() == pt_kind::LESS) {
                stoken.number(std::next(beg)->v_number());
                stoken.byteline({'<'});
            } else if (beg->kind() == pt_kind::GREATER) {
                stoken.number(std::next(beg)->v_number());
                stoken.byteline({'>'});
            }
            break;
        case adr_mode::ZP_IND:
            stoken.command(st_command::CMP_izp);
            std::advance(beg, 2);
            stoken.number(beg->v_number());
            break;
        case adr_mode::ZP_IND_Y:
            stoken.command(st_command::CMP_izy);
            std::advance(beg, 2);
            stoken.number(beg->v_number());
            break;
        case adr_mode::ZP_X_IND:
            stoken.command(st_command::CMP_izx);
            std::advance(beg, 2);
            stoken.number(beg->v_number());
            break;
        default:
            add_exception("Error at line " + std::to_string(beg->row()) + ":\nUnavailable addressing mode for CMP");
    }
    m_tokens.push_back(stoken);
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

parser_token::adr_mode  parser::
define_addr_mode(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end)
{
    const auto tk = beg;
    if (beg == end) return adr_mode::STK_or_IMP;
    if (beg->kind() == pt_kind::OPCODE and beg->v_opcode() == pt_opcode::REGISTER_A) {
        std::advance(beg, 1);
        if (beg == end) return adr_mode::A;
    }

    else if (beg->kind() == pt_kind::NUMBER or beg->kind() == pt_kind::LABEL_CALL) {
        // If number
        if (beg->kind() == pt_kind::NUMBER) {
            auto num1 = beg->v_number();
            if (num1 <= 0xFF) {             // is zero page?
                std::advance(beg, 1);
                if (beg == end) return adr_mode::ZP;
                if (beg->kind() == pt_kind::COMMA) {
                    std::advance(beg, 1);
                    if (beg != end) {
                        if (beg->kind() == pt_kind::OPCODE) {
                            if (beg->v_opcode() == pt_opcode::REGISTER_X and std::next(beg) == end) return adr_mode::ZP_X;
                            if (beg->v_opcode() == pt_opcode::REGISTER_Y and std::next(beg) == end) return adr_mode::ZP_Y;
                        } else if (beg->kind() == pt_kind::LABEL_CALL and std::next(beg) == end) {
                            return adr_mode::ZP_REL;
                        }
                    }
                }
            } else {
                std::advance(beg, 1);
                if (beg == end) return adr_mode::ABS_or_REL;
                if (beg->kind() == pt_kind::COMMA) {
                    std::advance(beg, 1);
                    if (beg != end and beg->kind() == pt_kind::OPCODE) {
                        if (beg->v_opcode() == pt_opcode::REGISTER_X and std::next(beg) == end)
                            return adr_mode::ABS_X;
                        if (beg->v_opcode() == pt_opcode::REGISTER_Y and std::next(beg) == end)
                            return adr_mode::ABS_Y;
                    }
                }
            }
        } else if (beg->kind() == pt_kind::LABEL_CALL) {
            std::advance(beg, 1);
            if (beg == end) return adr_mode::ABS_or_REL;
            if (beg->kind() == pt_kind::COMMA) {
                std::advance(beg, 1);
                if (beg != end and beg->kind() == pt_kind::OPCODE) {
                    if (beg->v_opcode() == pt_opcode::REGISTER_X and std::next(beg) == end)
                        return adr_mode::ABS_X;
                    if (beg->v_opcode() == pt_opcode::REGISTER_Y and std::next(beg) == end)
                        return adr_mode::ABS_Y;
                }
            }
        }
    } else if (beg->kind() == pt_kind::HASH) {
        std::advance(beg, 1);
        if (beg != end) {
            if (beg->kind() == pt_kind::NUMBER) {
                auto num = beg->v_number();
                if (num <= 0xFF and std::next(beg) == end) {
                    return adr_mode::IMM;
                }
            } else if (beg->kind() == pt_kind::LESS or beg->kind() == pt_kind::GREATER) {
                std::advance(beg, 1);
                if (beg != end) {
                    if (beg->kind() == pt_kind::LABEL_CALL and std::next(beg) == end) {
                        return adr_mode::IMM;
                    }
                }
            }
        }
    }
    else if (beg->kind() == pt_kind::LEFT_PARENTHESIS) {
        std::advance(beg, 1);
        if (beg != end) {
            if (beg->kind() == pt_kind::NUMBER) {             // NUMBER after (
                auto num = beg->v_number();
                if (num <= 0xFF) {                            // Zero page
                    std::advance(beg, 1);
                    if (beg != end) {
                        if (beg->kind() == pt_kind::RIGHT_PARENTHESIS) {
                            if (std::next(beg) == end) {
                                return adr_mode::ZP_IND;
                            } else if (std::next(beg)->kind() == pt_kind::COMMA) {
                                std::advance(beg, 1);
                                if (std::next(beg) != end) {
                                    std::advance(beg, 1);
                                    if (beg->kind() == pt_kind::OPCODE and beg->v_opcode() == pt_opcode::REGISTER_Y) {
                                        if (std::next(beg) == end) {
                                            return parser_token::adr_mode::ZP_IND_Y;
                                        }
                                    }
                                }
                            }
                        } else if (beg->kind() == pt_kind::COMMA) {
                            if (std::next(beg) != end) {
                                std::advance(beg, 1);
                                if (beg->kind() == pt_kind::OPCODE and beg->v_opcode() == pt_opcode::REGISTER_X) {
                                    if (std::next(beg) != end and std::next(beg)->kind() == pt_kind::RIGHT_PARENTHESIS) {
                                        std::advance(beg, 1);
                                        if (std::next(beg) == end) {
                                            return adr_mode::ZP_X_IND;
                                        }
                                    }
                                }
                            }
                        }
                    }
                } else {                                        // Not zero page
                    std::advance(beg, 1);
                    if (beg != end) {
                        if (beg->kind() == pt_kind::RIGHT_PARENTHESIS and std::next(beg) == end) {
                            return adr_mode::ABS_IND;
                        }

                    }
                }

            }
            else if (beg->kind() == pt_kind::LABEL_CALL) {               // LABEL after (


            }
        }
    }

    return adr_mode::UNEXPECTED;
}
/*-------------------------------*
 |        MOlex Assembler        |
 |          Parser Token         |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#include "../include/parser_token.hpp"

using namespace mxasm;


parser_token::
parser_token(lexer_token other)
    : m_base_token {std::move(other)}
{
    m_row = m_base_token.row();
    m_column = m_base_token.column();
}





lexer_token             parser_token::
base_token() const noexcept
{ return m_base_token; }

parser_token::pt_kind   parser_token::
kind() const noexcept
{ return m_kind; }

std::size_t             parser_token::
v_number() const noexcept
{ return m_v_number; }

std::string             parser_token::
v_lexeme() const noexcept
{ return m_v_lexeme; }

std::size_t             parser_token::
row() const noexcept
{ return m_row; }

std::size_t             parser_token::
column() const noexcept
{ return m_column; }

parser_token::pt_opcode parser_token::
v_opcode() const noexcept
{ return m_v_opcode; }

parser_token::pt_directive   parser_token::
v_directive() const noexcept
{ return m_v_directive; }


void                    parser_token::
base_token(const lexer_token new_base_token)
{ m_base_token = new_base_token; }

void                    parser_token::
kind(const pt_kind opcode)
{ m_kind = opcode; }

void                    parser_token::
v_number(const std::size_t number)
{ m_v_number = number; }

void                    parser_token::
v_lexeme(const std::string lexeme)
{ m_v_lexeme = lexeme; }

void                    parser_token::
row(const std::size_t row_value)
{ m_row = row_value; }

void                    parser_token::
column(const std::size_t column_value)
{ m_column = column_value; }

void                    parser_token::
v_opcode(const pt_opcode opcode)
{ m_v_opcode = opcode; }

void                    parser_token::
v_directive(const pt_directive directive)
{ m_v_directive = directive; }


std::string             parser_token::
pt_kind_to_string(const pt_kind kind) noexcept
{ return pt_kind_string.at(kind); }

std::string             parser_token::
pt_opcode_to_string(const pt_opcode opcode) noexcept
{ return pt_opcode_string.at(opcode); }

std::string             parser_token::
pt_directive_to_string(const pt_directive directive) noexcept
{ return pt_directive_string.at(directive); }


bool                    parser_token::
is_opcode_or_register(const std::string lexeme) noexcept
{
    std::string new_lexeme = to_upper(lexeme);
    for (const auto &e : pt_opcode_string) {
        if (e.second == new_lexeme) return true;
    }
    return false;
}

parser_token::pt_opcode parser_token::
get_opcode_by_name(const std::string lexeme) noexcept
{
    std::string new_str = to_upper(lexeme);
    for (const auto &[opcode, str] : pt_opcode_string) {
        if (new_str == str) {
            return opcode;
        }
    }
    return pt_opcode::NOP;
}

parser_token::pt_directive   parser_token::
get_directive_by_name(const std::string lexeme) noexcept
{
    std::string new_str = to_upper(lexeme);
    for (const auto &[opcode, str] : pt_directive_string) {
        if (new_str == str) {
            return opcode;
        }
    }
}


const std::map<parser_token::pt_kind, std::string>    parser_token::
pt_kind_string
{
    { pt_kind::NUMBER,            "NUMBER"            },
    { pt_kind::LABEL_DECLARATION, "LABEL DECLARATION" },
    { pt_kind::OPCODE,            "OPCODE"            },
    { pt_kind::_IDENTIFIER,       "_IDENTIFIER"       },
    { pt_kind::COMMA,             "COMMA"             },
    { pt_kind::HASH,              "HASH"              },
    { pt_kind::LESS,              "LESS"              },
    { pt_kind::GREATER,           "GREATER"           },
    { pt_kind::LEFT_PARENTHESIS,  "LEFT PARENTHESIS"  },
    { pt_kind::RIGHT_PARENTHESIS, "RIGHT PARENTHESIS" },
    { pt_kind::EQUALS,            "EQUALS"            },
    { pt_kind::STRING,            "STRING"            },
    { pt_kind::DIRECTIVE,         "DIRECTIVE"         },
    { pt_kind::LABEL_CALL,        "LABEL CALL"        }
};

const std::map<parser_token::pt_opcode, std::string>  parser_token::
pt_opcode_string
{
    { pt_opcode::ADC,  "ADC"  }, { pt_opcode::AND,  "AND"  }, { pt_opcode::ASL,  "ASL"  }, { pt_opcode::BBR0, "BBR0" },
    { pt_opcode::BBR1, "BBR1" }, { pt_opcode::BBR2, "BBR2" }, { pt_opcode::BBR3, "BBR3" }, { pt_opcode::BBR4, "BBR4" },
    { pt_opcode::BBR5, "BBR5" }, { pt_opcode::BBR6, "BBR6" }, { pt_opcode::BBR7, "BBR7" }, { pt_opcode::BBS0, "BBS0" },
    { pt_opcode::BBS1, "BBS1" }, { pt_opcode::BBS2, "BBS2" }, { pt_opcode::BBS3, "BBS3" }, { pt_opcode::BBS4, "BBS4" },
    { pt_opcode::BBS5, "BBS5" }, { pt_opcode::BBS6, "BBS6" }, { pt_opcode::BBS7, "BBS7" }, { pt_opcode::BCC,  "BCC"  },
    { pt_opcode::BCS,  "BCS"  }, { pt_opcode::BEQ,  "BEQ"  }, { pt_opcode::BIT,  "BIT"  }, { pt_opcode::BMI,  "BMI"  },
    { pt_opcode::BNE,  "BNE"  }, { pt_opcode::BPL,  "BPL"  }, { pt_opcode::BRA,  "BRA"  }, { pt_opcode::BRK,  "BRK"  },
    { pt_opcode::BVC,  "BVC"  }, { pt_opcode::BVS,  "BVS"  }, { pt_opcode::CLC,  "CLC"  }, { pt_opcode::CLD,  "CLD"  },
    { pt_opcode::CLI,  "CLI"  }, { pt_opcode::CLV,  "CLV"  }, { pt_opcode::CMP,  "CMP"  }, { pt_opcode::CPY,  "CPY"  },
    { pt_opcode::CPX,  "CPX"  }, { pt_opcode::DEC,  "DEC"  }, { pt_opcode::DEX,  "DEX"  }, { pt_opcode::DEY,  "DEY"  },
    { pt_opcode::EOR,  "EOR"  }, { pt_opcode::INC,  "INC"  }, { pt_opcode::INX,  "INX"  }, { pt_opcode::INY,  "INY"  },
    { pt_opcode::JMP,  "JMP"  }, { pt_opcode::JSR,  "JSR"  }, { pt_opcode::LDA,  "LDA"  }, { pt_opcode::LDX,  "LDX"  },
    { pt_opcode::LDY,  "LDY"  }, { pt_opcode::LSR,  "LSR"  }, { pt_opcode::NOP,  "NOP"  }, { pt_opcode::ORA,  "ORA"  },
    { pt_opcode::PHA,  "PHA"  }, { pt_opcode::PHP,  "PHP"  }, { pt_opcode::PHX,  "PHX"  }, { pt_opcode::PHY,  "PHY"  },
    { pt_opcode::PLA,  "PLA"  }, { pt_opcode::PLP,  "PLP"  }, { pt_opcode::PLX,  "PLX"  }, { pt_opcode::PLY,  "PLY"  },
    { pt_opcode::RMB0, "RMB0" }, { pt_opcode::RMB1, "RMB1" }, { pt_opcode::RMB2, "RMB2" }, { pt_opcode::RMB3, "RMB3" },
    { pt_opcode::RMB4, "RMB4" }, { pt_opcode::RMB5, "RMB5" }, { pt_opcode::RMB6, "RMB6" }, { pt_opcode::RMB7, "RMB7" },
    { pt_opcode::ROL,  "ROL"  }, { pt_opcode::ROR,  "ROR"  }, { pt_opcode::RTI,  "RTI"  }, { pt_opcode::RTS,  "RTS"  },
    { pt_opcode::SBC,  "SBC"  }, { pt_opcode::SEC,  "SEC"  }, { pt_opcode::SED,  "SED"  }, { pt_opcode::SEI,  "SEI"  },
    { pt_opcode::SMB0, "SMB0" }, { pt_opcode::SMB1, "SMB1" }, { pt_opcode::SMB2, "SMB2" }, { pt_opcode::SMB3, "SMB3" },
    { pt_opcode::SMB4, "SMB4" }, { pt_opcode::SMB5, "SMB5" }, { pt_opcode::SMB6, "SMB6" }, { pt_opcode::SMB7, "SMB7" },
    { pt_opcode::STA,  "STA"  }, { pt_opcode::STP,  "STP"  }, { pt_opcode::STX,  "STX"  }, { pt_opcode::STY,  "STY"  },
    { pt_opcode::STZ,  "STZ"  }, { pt_opcode::TAX,  "TAX"  }, { pt_opcode::TAY,  "TAY"  }, { pt_opcode::TRB,  "TRB"  },
    { pt_opcode::TSB,  "TSB"  }, { pt_opcode::TSX,  "TSX"  }, { pt_opcode::TXA,  "TXA"  }, { pt_opcode::TXS,  "TXS"  },
    { pt_opcode::TYA,  "TYA"  }, { pt_opcode::WAI,  "WAI"  },
    { pt_opcode::REGISTER_X, "X" }, { pt_opcode::REGISTER_Y, "Y"}
};

const std::map<parser_token::pt_directive, std::string> parser_token::
pt_directive_string
{
    { pt_directive::CODE_POSITION, "CODE POSITION" },
    { pt_directive::MACRO,         "DEFINE"        },
    { pt_directive::BYTE,          "BYTE"          },
    { pt_directive::WORD,          "WORD"          }
};


std::ostream&           mxasm::
operator<<(std::ostream &os, const parser_token::pt_kind &kind)
{
    os << parser_token::pt_kind_to_string(kind);
    return os;
}

std::ostream&           mxasm::
operator<<(std::ostream &os, const parser_token::pt_opcode &opcode)
{
    os << parser_token::pt_opcode_to_string(opcode);
    return os;
}

std::ostream&           mxasm::
operator<<(std::ostream &os, const parser_token::pt_directive &directive)
{
    os << parser_token::pt_directive_to_string(directive);
    return os;
}

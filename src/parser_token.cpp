/*-------------------------------*
 |        MOlex Assembler        |
 |         Parser Token          |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#include <stdexcept>

#include "../include/parser_token.hpp"

using namespace mxasm;
using pt_kind = parser_token::pt_kind;


parser_token::
parser_token(const pt_kind &kind, const std::string lexeme, const std::size_t row, const std::size_t column)
    : m_kind {kind}, m_lexeme {lexeme}, m_row {row}, m_column {column} {}
















std::size_t        parser_token::
row() const noexcept
{ return m_row; }

std::size_t        parser_token::
column() const noexcept
{ return m_column; }

std::string        parser_token::
lexeme() const noexcept
{ return m_lexeme; }

pt_kind            parser_token::
kind() const noexcept
{ return m_kind; }

void               parser_token::
row(const std::size_t row) noexcept
{ m_row = row; }

void               parser_token::
column(const std::size_t column) noexcept
{ m_column = column; }

void               parser_token::
lexeme(std::string lexeme) noexcept
{ m_lexeme = lexeme; }

void               parser_token::
kind(const pt_kind kind) noexcept
{ m_kind = kind; }

bool               parser_token::
is(const pt_kind &kind) const noexcept
{ return m_kind == kind; }

bool               parser_token::
is_not(const pt_kind &kind) const noexcept
{ return m_kind != kind; }


std::map<pt_kind, std::string>    parser_token::
pt_kind_str
{
    { pt_kind::DIRECTIVE_CODE_POSITION, "DIRECTIVE CODE POSITION" },
    { pt_kind::DIRECTIVE_MACRO,         "DIRECTIVE MACRO"         },
    { pt_kind::DIRECTIVE_BYTE,          "DIRECTIVE BYTE"          },
    { pt_kind::DIRECTIVE_WORD,          "DIRECTIVE WORD"          },

    { pt_kind::REGISTER_X,              "REGISTER X"              },
    { pt_kind::REGISTER_Y,              "REGISTER Y"              },
    { pt_kind::LABEL_DECLARATION,       "LABEL DECLARATION"       },
    { pt_kind::LABEL_CALL,              "LABEL CALL"              },
    { pt_kind::IDENTIFIER,              "IDENTIFIER"              },
    { pt_kind::NUMBER,                  "NUMBER"                  },
    { pt_kind::STRING,                  "STRING"                  },

    { pt_kind::COMMA,             "COMMA"             },
    { pt_kind::HASH,              "HASH"              },
    { pt_kind::LEFT_PARENTHESIS,  "LEFT PARENTHESIS"  },
    { pt_kind::RIGHT_PARENTHESIS, "RIGHT PARENTHESIS" },
    { pt_kind::LESS,              "LESS"              },
    { pt_kind::GREATER,           "GREATER"           },
    { pt_kind::EQUALS,            "EQUALS"            },

    { pt_kind::ADC,  "ADC"  }, { pt_kind::AND,  "AND"  }, { pt_kind::ASL,  "ASL"  }, { pt_kind::BBR0, "BBR0" },
    { pt_kind::BBR1, "BBR1" }, { pt_kind::BBR2, "BBR2" }, { pt_kind::BBR3, "BBR3" }, { pt_kind::BBR4, "BBR4" },
    { pt_kind::BBR5, "BBR5" }, { pt_kind::BBR6, "BBR6" }, { pt_kind::BBR7, "BBR7" }, { pt_kind::BBS0, "BBS0" },
    { pt_kind::BBS1, "BBS1" }, { pt_kind::BBS2, "BBS2" }, { pt_kind::BBS3, "BBS3" }, { pt_kind::BBS4, "BBS4" },
    { pt_kind::BBS5, "BBS5" }, { pt_kind::BBS6, "BBS6" }, { pt_kind::BBS7, "BBS7" }, { pt_kind::BCC,  "BCC"  },
    { pt_kind::BCS,  "BCS"  }, { pt_kind::BEQ,  "BEQ"  }, { pt_kind::BIT,  "BIT"  }, { pt_kind::BMI,  "BMI"  },
    { pt_kind::BNE,  "BNE"  }, { pt_kind::BPL,  "BPL"  }, { pt_kind::BRA,  "BRA"  }, { pt_kind::BRK,  "BRK"  },
    { pt_kind::BVC,  "BVC"  }, { pt_kind::BVS,  "BVS"  }, { pt_kind::CLC,  "CLC"  }, { pt_kind::CLD,  "CLD"  },
    { pt_kind::CLI,  "CLI"  }, { pt_kind::CLV,  "CLV"  }, { pt_kind::CMP,  "CMP"  }, { pt_kind::CPY,  "CPY"  },
    { pt_kind::CPX,  "CPX"  }, { pt_kind::DEC,  "DEC"  }, { pt_kind::DEX,  "DEX"  }, { pt_kind::DEY,  "DEY"  },
    { pt_kind::EOR,  "EOR"  }, { pt_kind::INC,  "INC"  }, { pt_kind::INX,  "INX"  }, { pt_kind::INY,  "INY"  },
    { pt_kind::JMP,  "JMP"  }, { pt_kind::JSR,  "JSR"  }, { pt_kind::LDA,  "LDA"  }, { pt_kind::LDX,  "LDX"  },
    { pt_kind::LDY,  "LDY"  }, { pt_kind::LSR,  "LSR"  }, { pt_kind::NOP,  "NOP"  }, { pt_kind::ORA,  "ORA"  },
    { pt_kind::PHA,  "PHA"  }, { pt_kind::PHP,  "PHP"  }, { pt_kind::PHX,  "PHX"  }, { pt_kind::PHY,  "PHY"  },
    { pt_kind::PLA,  "PLA"  }, { pt_kind::PLP,  "PLP"  }, { pt_kind::PLX,  "PLX"  }, { pt_kind::PLY,  "PLY"  },
    { pt_kind::RMB0, "RMB0" }, { pt_kind::RMB1, "RMB1" }, { pt_kind::RMB2, "RMB2" }, { pt_kind::RMB3, "RMB3" },
    { pt_kind::RMB4, "RMB4" }, { pt_kind::RMB5, "RMB5" }, { pt_kind::RMB6, "RMB6" }, { pt_kind::RMB7, "RMB7" },
    { pt_kind::ROL,  "ROL"  }, { pt_kind::ROR,  "ROR"  }, { pt_kind::RTI,  "RTI"  }, { pt_kind::RTS,  "RTS"  },
    { pt_kind::SBC,  "SBC"  }, { pt_kind::SEC,  "SEC"  }, { pt_kind::SED,  "SED"  }, { pt_kind::SEI,  "SEI"  },
    { pt_kind::SMB0, "SMB0" }, { pt_kind::SMB1, "SMB1" }, { pt_kind::SMB2, "SMB2" }, { pt_kind::SMB3, "SMB3" },
    { pt_kind::SMB4, "SMB4" }, { pt_kind::SMB5, "SMB5" }, { pt_kind::SMB6, "SMB6" }, { pt_kind::SMB7, "SMB7" },
    { pt_kind::STA,  "STA"  }, { pt_kind::STP,  "STP"  }, { pt_kind::STX,  "STX"  }, { pt_kind::STY,  "STY"  },
    { pt_kind::STZ,  "STZ"  }, { pt_kind::TAX,  "TAX"  }, { pt_kind::TAY,  "TAY"  }, { pt_kind::TRB,  "TRB"  },
    { pt_kind::TSB,  "TSB"  }, { pt_kind::TSX,  "TSX"  }, { pt_kind::TXA,  "TXA"  }, { pt_kind::TXS,  "TXS"  },
    { pt_kind::TYA,  "TYA"  }, { pt_kind::WAI,  "WAI"  }
};

std::string        parser_token::
pt_kind_to_str(const pt_kind &kind) noexcept
{ return pt_kind_str.at(kind); }

bool               parser_token::
value_exists_in_opcodes(const std::string &str) noexcept
{
    for (const auto &[first, second] : pt_kind_str) {
        if (second == to_upper(str)) {
            return true;
        }
    }
    return false;
}

pt_kind            parser_token::
get_opcode_name(const std::string &str)
{
    for (const auto &[first, second] : pt_kind_str) {
        if (second == to_upper(str)) {
            return first;
        }
    }
    throw std::invalid_argument("There is no opcode with such name: " + to_upper(str) + "\n");
}

bool               parser_token::
is_opcode(const pt_kind &kind) noexcept
{ return kind >= pt_kind::ADC and kind <= pt_kind::WAI; }

bool               parser_token::
is_directive(const pt_kind &kind) noexcept
{
    return    kind == pt_kind::DIRECTIVE_BYTE
           or kind == pt_kind::DIRECTIVE_WORD
           or kind == pt_kind::DIRECTIVE_CODE_POSITION;
}


std::ostream &     mxasm::
operator<<(std::ostream &os, const mxasm::parser_token::pt_kind &kind) {
    os << mxasm::parser_token::pt_kind_str.at(kind);
    return os;
}
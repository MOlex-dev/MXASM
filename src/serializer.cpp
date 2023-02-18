/*-------------------------------*
 |        MOlex Assembler        |
 |           Serializer          |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#include "../include/serializer.hpp"

using namespace mxasm;
using st_kind = serializable_token::st_kind;
using st_command = serializable_token::st_command;

serializer::
serializer(std::list<serializable_token> &tokens)
    : m_tokens {std::move(tokens)}
{
    m_program.reserve(0x10'00);
    for (word_t i = 0; i < 0x10'00; ++i) m_program.push_back(0);
}


std::vector<byte_t>     serializer::
binary_program()
{
    serialize();
    return m_program;
}


void                    serializer::
serialize()
{
    std::map<word_t, word_t> label_address;
    std::map<std::pair<word_t, word_t>, bool> labelable_commands; // Address - Label - two_bytes

    for (const auto &op : m_tokens) {
        if (op.kind() == st_kind::LABEL) {
            label_address.emplace(op.number(), m_write_address);
            continue;
        }
        if (op.kind() == st_kind::CODE_POS) {
            m_write_address = op.number();
            continue;
        }
        if (op.kind() == st_kind::BYTE) {
            for (const auto &b : op.byteline()) {
                write_byte_to_memory(b & 0x00'FF);
            }
            continue;
        }
        if (op.kind() == st_kind::WORD) {
            for (const auto &w : op.byteline()) {
                write_word_to_memory(w);
            }
            continue;
        }

        if (op.kind() == st_kind::OPCODE) {
            byte_t code;
            word_t data;
            switch (op.command()) {
                case st_command::BRK_stk:                                      // Implied and Stack
                case st_command::CLC_imp:
                case st_command::CLD_imp:
                case st_command::CLI_imp:
                case st_command::CLV_imp:
                case st_command::DEX_imp:
                case st_command::DEY_imp:
                case st_command::INX_imp:
                case st_command::INY_imp:
                case st_command::NOP_imp:
                case st_command::PHA_stk:
                case st_command::PHP_stk:
                case st_command::PHX_stk:
                case st_command::PHY_stk:
                case st_command::PLA_stk:
                case st_command::PLP_stk:
                case st_command::PLX_stk:
                case st_command::PLY_stk:
                case st_command::RTS_stk:
                case st_command::RTI_stk:
                case st_command::SEC_imp:
                case st_command::SED_imp:
                case st_command::SEI_imp:
                case st_command::STP_imp:
                case st_command::TAX_imp:
                case st_command::TAY_imp:
                case st_command::TSX_imp:
                case st_command::TXA_imp:
                case st_command::TXS_imp:
                case st_command::TYA_imp:
                case st_command::WAI_imp:

                case st_command::ASL_a:
                case st_command::INC_a:
                case st_command::ROL_a:
                case st_command::DEC_a:
                case st_command::LSR_a:
                case st_command::ROR_a:
                    code = static_cast<byte_t>(op.command());
                    write_byte_to_memory(code);
                    break;

                case st_command::JMP_abs:
                case st_command::JSR_abs:
                case st_command::ASL_abs:
                case st_command::INC_abs:
                case st_command::ROL_abs:
                case st_command::DEC_abs:
                case st_command::LSR_abs:
                case st_command::ROR_abs:
                case st_command::STX_abs:
                case st_command::LDX_abs:
                case st_command::TSB_abs:
                case st_command::ORA_abs:
                case st_command::TRB_abs:
                case st_command::BIT_abs:
                case st_command::AND_abs:
                case st_command::EOR_abs:
                case st_command::ADC_abs:
                case st_command::CPX_abs:
                case st_command::SBC_abs:
                case st_command::STY_abs:
                case st_command::STA_abs:
                case st_command::STZ_abs:
                case st_command::LDY_abs:
                case st_command::LDA_abs:
                case st_command::CPY_abs:
                case st_command::CMP_abs:
                    code = static_cast<byte_t>(op.command());
                    write_byte_to_memory(code);
                    if (op.labelable()) {
                        labelable_commands.emplace(std::make_pair(m_write_address, op.number()), true);
                        data = 0xFF'FF;
                    } else {
                        data = op.number();
                    }
                    write_word_to_memory(data);
                    break;





            }
            continue;
        }
    }

    for (const auto &[address, two_bytes] : labelable_commands) {
        m_write_address = address.first;
        if (two_bytes) {
            write_word_to_memory(label_address.at(address.second));
        } else {
            write_byte_to_memory(label_address.at(address.second));
        }
    }
    m_program.erase(m_program.begin() + m_end_of_program + 1, m_program.end());
}

void                    serializer::
write_byte_to_memory(const byte_t value)
{
    if (m_write_address < 0x0600) return;
    m_program[m_write_address - 0x06'00] = value;
    m_end_of_program = std::max<word_t>(m_end_of_program, m_write_address - 0x06'00);
    ++m_write_address;
}

void                    serializer::
write_word_to_memory(const word_t value)
{
    write_byte_to_memory(value & 0x00'FF);
    write_byte_to_memory((value >> 8) & 0x00'FF);
}
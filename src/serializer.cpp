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
    std::map<word_t, word_t> relative_labels; // address - label

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

                case st_command::ORA_abx:
                case st_command::ORA_aby:
                case st_command::AND_abx:
                case st_command::AND_aby:
                case st_command::EOR_abx:
                case st_command::EOR_aby:
                case st_command::ADC_abx:
                case st_command::ADC_aby:
                case st_command::STA_abx:
                case st_command::STA_aby:
                case st_command::LDA_abx:
                case st_command::LDA_aby:
                case st_command::CMP_abx:
                case st_command::CMP_aby:
                case st_command::SBC_abx:
                case st_command::SBC_aby:
                case st_command::BIT_abx:
                case st_command::LDY_abx:
                case st_command::STZ_abx:
                case st_command::INC_abx:
                case st_command::DEC_abx:
                case st_command::ASL_abx:
                case st_command::ROL_abx:
                case st_command::ROR_abx:
                case st_command::LSR_abx:
                case st_command::LDX_aby:

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

                case st_command::BPL_rel:
                case st_command::BMI_rel:
                case st_command::BVC_rel:
                case st_command::BVS_rel:
                case st_command::BRA_rel:
                case st_command::BCC_rel:
                case st_command::BCS_rel:
                case st_command::BNE_rel:
                case st_command::BEQ_rel:
                    code = static_cast<byte_t>(op.command()); // ELSE 00
                    write_byte_to_memory(code);
                    relative_labels.emplace(m_write_address, op.number());
                    write_byte_to_memory(0);
                    break;

                case st_command::RMB0_zpg:
                case st_command::RMB1_zpg:
                case st_command::RMB2_zpg:
                case st_command::RMB3_zpg:
                case st_command::RMB4_zpg:
                case st_command::RMB5_zpg:
                case st_command::RMB6_zpg:
                case st_command::RMB7_zpg:
                case st_command::SMB0_zpg:
                case st_command::SMB1_zpg:
                case st_command::SMB2_zpg:
                case st_command::SMB3_zpg:
                case st_command::SMB4_zpg:
                case st_command::SMB5_zpg:
                case st_command::SMB6_zpg:
                case st_command::SMB7_zpg:

                case st_command::ORA_zpg:
                case st_command::TSB_zpg:
                case st_command::ASL_zpg:
                case st_command::TRB_zpg:
                case st_command::STZ_zpg:
                case st_command::BIT_zpg:
                case st_command::AND_zpg:
                case st_command::ROL_zpg:
                case st_command::EOR_zpg:
                case st_command::LSR_zpg:
                case st_command::ADC_zpg:
                case st_command::ROR_zpg:
                case st_command::STY_zpg:
                case st_command::STA_zpg:
                case st_command::STX_zpg:
                case st_command::LDY_zpg:
                case st_command::LDA_zpg:
                case st_command::LDX_zpg:
                case st_command::CMP_zpg:
                case st_command::CPY_zpg:
                case st_command::CPX_zpg:
                case st_command::INC_zpg:
                case st_command::DEC_zpg:
                case st_command::SBC_zpg:

                case st_command::ORA_zpx:
                case st_command::BIT_zpx:
                case st_command::STZ_zpx:
                case st_command::ASL_zpx:
                case st_command::STY_zpx:
                case st_command::LDY_zpx:
                case st_command::AND_zpx:
                case st_command::ROL_zpx:
                case st_command::EOR_zpx:
                case st_command::LSR_zpx:
                case st_command::ADC_zpx:
                case st_command::ROR_zpx:
                case st_command::STA_zpx:
                case st_command::LDA_zpx:
                case st_command::CMP_zpx:
                case st_command::DEC_zpx:
                case st_command::SBC_zpx:
                case st_command::INC_zpx:
                case st_command::STX_zpy:
                case st_command::LDX_zpy:
                    code = static_cast<byte_t>(op.command());
                    write_byte_to_memory(code);
                    write_byte_to_memory(op.number());
                    break;

                case st_command::BBR0_zpr:
                case st_command::BBR1_zpr:
                case st_command::BBR2_zpr:
                case st_command::BBR3_zpr:
                case st_command::BBR4_zpr:
                case st_command::BBR5_zpr:
                case st_command::BBR6_zpr:
                case st_command::BBR7_zpr:
                case st_command::BBS0_zpr:
                case st_command::BBS1_zpr:
                case st_command::BBS2_zpr:
                case st_command::BBS3_zpr:
                case st_command::BBS4_zpr:
                case st_command::BBS5_zpr:
                case st_command::BBS6_zpr:
                case st_command::BBS7_zpr:
                    code = static_cast<byte_t>(op.command());
                    write_byte_to_memory(code);
                    write_byte_to_memory(op.number());
                    relative_labels.emplace(m_write_address, op.byteline()[0]);
                    write_byte_to_memory(0xFF);
                    break;

            }
            continue;
        }
    }

    // For labels
    for (const auto &[address, two_bytes] : labelable_commands) {
        m_write_address = address.first;
        if (two_bytes) {
            write_word_to_memory(label_address.at(address.second));
        } else {
            write_byte_to_memory(label_address.at(address.second));
        }
    }

    // For relative
    for (const auto &[address, lbl] : relative_labels) {
        m_write_address = address;

        int32_t distance = label_address.at(lbl) - (m_write_address + 1); // Standard offset from next instruction
        if (distance < -128 or distance > 127) {
            write_byte_to_memory(0x00);
            continue;
        }
        write_byte_to_memory(distance);
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
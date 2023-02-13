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
    for (const auto &op : m_tokens) {
        if (op.kind() == st_kind::LABEL) {

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
                write_byte_to_memory(w & 0x00'FF);
                write_byte_to_memory((w >> 8) & 0x00'FF);
            }
            continue;
        }

        if (op.kind() == st_kind::OPCODE) {
            switch (op.command()) {
                case st_command::BRK_stk:

                case st_command::CLC_imp:
                case st_command::CLD_imp:
                case st_command::CLI_imp:
                case st_command::CLV_imp:

                case st_command::NOP_imp:

                    auto code = static_cast<byte_t>(op.command());
                    write_byte_to_memory(code);
            }





            continue;
        }
    }
    m_program.erase(m_program.begin() + m_end_of_program + 1, m_program.end());
}

void                    serializer::
write_byte_to_memory(const byte_t value)
{
    m_program[m_write_address - 0x06'00] = value;
    m_end_of_program = std::max<word_t>(m_end_of_program, m_write_address - 0x06'00);
    ++m_write_address;
}
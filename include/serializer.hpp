/*-------------------------------*
 |        MOlex Assembler        |
 |           Serializer          |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#pragma once

#include <vector>
#include <map>

#include "serializable_token.hpp"


namespace mxasm
{
    class serializer
    {
    public:
        explicit serializer(std::list<serializable_token> &tokens);
        std::vector<byte_t> binary_program();

    private:
        const std::list<serializable_token> m_tokens;
        std::vector<byte_t>                 m_program       {};
        word_t                              m_write_address {0x0600};
        word_t                              m_end_of_program {0};

        void serialize();
        void write_byte_to_memory(const byte_t value);
        void write_word_to_memory(const word_t value);
    };
}

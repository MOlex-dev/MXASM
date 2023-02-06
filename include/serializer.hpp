/*-------------------------------*
 |        MOlex Assembler        |
 |          Serializer           |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#pragma once

#include <map>

#include "../include/util.hpp"
#include "../include/serializable_token.hpp"

namespace mxasm
{
    class serializer
    {
    public:
        enum class opcodes
        {

        };
        explicit serializer(const serializable_tokens &tokens);



    private:
        const serializable_tokens &m_input_tokens;
        static std::map<opcodes, std::string> opcode_str;

    };
}


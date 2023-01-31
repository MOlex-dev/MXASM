/*-------------------------------*
 |        MOlex Assembler        |
 |          Serializer           |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#pragma once

#include "../include/util.hpp"

namespace mxasm
{
    class serializer
    {
    public:
        enum class opcodes
        {

        };
        explicit serializer(const parser_tokens &tokens);



    private:
        const parser_tokens &m_input_tokens;
        static std::map<opcodes, std::string> opcode_str;

    };
}


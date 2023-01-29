/*-------------------------------*
 |        MOlex Assembler        |
 |            Parser             |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#pragma once

#include "../include/parser_token.hpp"
#include "../include/util.hpp"


namespace mxasm
{
    class parser
    {
    public:
        explicit parser(const lexer_tokens &input_tokens);


    private:
        const lexer_tokens &m_input_tokens;
        parser_tokens       m_tokens;

    };
}
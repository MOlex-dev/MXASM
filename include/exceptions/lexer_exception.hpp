/*-----------------------------------*
 |          MOlex Assembler          |
 |          Lexer Exception          |
 |                                   |
 |         Author: MOlex-dev         |
 *-----------------------------------*/

#pragma once

#include "mxasm_exception.hpp"

namespace mxasm
{
    class lexer_exception : public mxasm_exception
    {
    public:
        explicit lexer_exception(std::string message) noexcept;
    };
}
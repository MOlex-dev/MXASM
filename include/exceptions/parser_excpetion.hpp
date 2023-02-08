/*-----------------------------------*
 |          MOlex Assembler          |
 |          Parser Exception         |
 |                                   |
 |         Author: MOlex-dev         |
 *-----------------------------------*/

#pragma once

#include "mxasm_exception.hpp"

namespace mxasm
{
    class parser_exception : public mxasm_exception
    {
    public:
        explicit parser_exception(std::string message) noexcept;
    };
}
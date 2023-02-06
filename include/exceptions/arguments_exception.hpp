/*-----------------------------------*
 |          MOlex Assembler          |
 |        Arguments Exception        |
 |                                   |
 |         Author: MOlex-dev         |
 *-----------------------------------*/

#pragma once

#include "mxasm_exception.hpp"

namespace mxasm
{
    class arguments_exception : public mxasm_exception
    {
    public:
        explicit arguments_exception(std::string message) noexcept;
    };
}
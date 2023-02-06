/*-----------------------------------*
 |          MOlex Assembler          |
 |        Arguments Exception        |
 |                                   |
 |         Author: MOlex-dev         |
 *-----------------------------------*/

#include "../../include/exceptions/arguments_exception.hpp"

using namespace mxasm;


arguments_exception::
arguments_exception(std::string message) noexcept
    : mxasm_exception(std::move(message))
{ exception_type("arguments_exception"); }

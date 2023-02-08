/*-----------------------------------*
 |          MOlex Assembler          |
 |          Parser Exception         |
 |                                   |
 |         Author: MOlex-dev         |
 *-----------------------------------*/

#include "../../include/exceptions/parser_excpetion.hpp"

using namespace mxasm;


parser_exception::
parser_exception(std::string message) noexcept
    : mxasm_exception(std::move(message))
{ exception_type("parser_exception"); }

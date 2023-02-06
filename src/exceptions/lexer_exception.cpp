/*-----------------------------------*
 |          MOlex Assembler          |
 |          Lexer Exception          |
 |                                   |
 |         Author: MOlex-dev         |
 *-----------------------------------*/

#include "../../include/exceptions/lexer_exception.hpp"

using namespace mxasm;


lexer_exception::
lexer_exception(std::string message) noexcept
    : mxasm_exception(std::move(message))
{ exception_type("lexer_exception"); }

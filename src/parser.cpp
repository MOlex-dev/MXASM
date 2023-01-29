/*-------------------------------*
 |        MOlex Assembler        |
 |            Parser             |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#include "../include/parser.hpp"

using namespace mxasm;


parser::
parser(const lexer_tokens &input_tokens)
    : m_input_tokens {input_tokens} {}


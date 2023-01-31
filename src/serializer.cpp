/*-------------------------------*
 |        MOlex Assembler        |
 |          Serializer           |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#include "../include/serializer.hpp"

using namespace mxasm;


serializer::
serializer(const parser_tokens &tokens)
    : m_input_tokens {tokens} {}

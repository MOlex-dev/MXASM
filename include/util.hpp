/*-------------------------------*
 |        MOlex Assembler        |
 |             Utils             |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#pragma once

#include <string>
#include <list>
#include <vector>

#include "../include/lexer_token.hpp"
#include "../include/parser_token.hpp"


namespace mxasm
{
    typedef uint8_t byte;
    typedef std::list<std::pair<std::size_t, std::string>> source_listing;
    typedef std::list<lexer_token>                         lexer_tokens;
    typedef std::list<parser_token>                        parser_tokens;

    void           validate_source_file_path(const std::string path);
    source_listing open_source_code(const std::string file_path);
}

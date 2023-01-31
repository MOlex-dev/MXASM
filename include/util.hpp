/*-------------------------------*
 |        MOlex Assembler        |
 |             Utils             |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#pragma once

#include <string>
#include <list>

#include "../include/lexer_token.hpp"
#include "../include/parser_token.hpp"


namespace mxasm
{
    typedef uint8_t  byte;
    typedef uint16_t word;
    typedef std::list<std::pair<std::size_t, std::string>> source_listing;

    void           validate_source_file_path(const std::string path);
    source_listing open_source_code(const std::string file_path);
    std::string    to_lower(const std::string &default_string);
    std::string    to_upper(const std::string &default_string);
    std::uint8_t   math_repr_of_char(const char c);
    char           char_repr_of_num(const uint8_t n);
    std::string    change_number_base(const std::string &number, const uint8_t def_base, const uint8_t new_base);
}

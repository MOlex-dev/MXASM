/*-------------------------------*
 |        MOlex Assembler        |
 |             Utils             |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#pragma once

#include <vector>
#include <string>
#include <list>
#include <fstream>
#include <memory>

#include "exceptions/arguments_exception.hpp"


namespace mxasm
{
    typedef uint8_t  byte_t;
    typedef uint16_t word_t;
    typedef std::list<std::pair<std::size_t, std::string>> source_listing;
    typedef std::list<std::unique_ptr<mxasm_exception>>    exception_list;

    std::string    get_source_file_path_from_cmd(const std::vector<std::string> &arguments);
    source_listing open_source_code(const std::string file_path);

    std::string to_lower(const std::string &default_string);
    std::string to_upper(const std::string &default_string);
    
}






namespace mxasm
{

    std::uint8_t   math_repr_of_char(const char c);
    char           char_repr_of_num(const uint8_t n);
    std::string    change_number_base(const std::string &number, const uint8_t def_base, const uint8_t new_base);
}

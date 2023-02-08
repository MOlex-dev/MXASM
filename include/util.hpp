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
#include <cmath>

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

    uint8_t  get_char_digit_value(const char c) noexcept;
    uint64_t string_to_number(const std::string str, const uint8_t base) noexcept;
    
}

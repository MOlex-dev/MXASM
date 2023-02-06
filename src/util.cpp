/*-------------------------------*
 |        MOlex Assembler        |
 |             Utils             |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/



#include "../include/util.hpp"

using namespace mxasm;


std::string             mxasm::
get_source_file_path_from_cmd(const std::vector<std::string> &arguments)
{
    if (arguments.size() == 1) {
        throw arguments_exception("No input file");
    }
    if (arguments.size() > 2) {
        throw arguments_exception("Too many arguments");
    }

    std::string path_to_file = arguments.at(1);

    if (path_to_file.length() < 5 or path_to_file.substr(path_to_file.length() - 4, 4) != ".asm") {
        throw arguments_exception("Wrong source code file name or extension: \'" + path_to_file
                                  + "\'. Should be [name].asm");
    }

    return path_to_file;
}

source_listing          mxasm::
open_source_code(const std::string file_path)
{
    std::ifstream file_reader(file_path);

    if (not file_reader.is_open()) {
        throw arguments_exception("Can't open source code file \'" + file_path + '\'');
    }

    source_listing source_code;
    std::string    buffer;
    std::size_t    line_number {0};

    while (std::getline(file_reader, buffer)) {
        ++line_number;
        if (buffer.empty() or buffer.length() == std::count(buffer.begin(), buffer.end(), ' ')) {
            continue;
        }
        source_code.emplace_back(line_number, buffer);
    }

    return source_code;
}


std::string             mxasm::
to_lower(const std::string &default_string)
{
    std::string res = default_string;
    std::for_each(res.begin(), res.end(), [](auto &c) { c = std::tolower(c); });
    return res;
}

std::string             mxasm::
to_upper(const std::string &default_string)
{
    std::string res = default_string;
    std::for_each(res.begin(), res.end(), [](auto &c) { c = std::toupper(c); });
    return res;
}




















#include <cmath>


std::uint8_t       mxasm::
math_repr_of_char(const char c)
{
    if (c >= '0' and c <= '9') return c - '0';
    if (c >= 'A' and c <= 'Z') return c - 'A' + 0xA;
    if (c >= 'a' and c <= 'z') return c - 'a' + 0xA;
    return 0;
}

char               mxasm::
char_repr_of_num(const uint8_t n)
{
    if (n >= 0 and n <= 9) return '0' + n;
    if (n >= 10 and n <= 35) return 'a' + n - 10;
    return 0;
}

std::string        mxasm::
change_number_base(const std::string &number, const uint8_t def_base, const uint8_t new_base)
{
    uint64_t number_in_dec = 0;
    for (std::size_t i = 0; i < number.length(); ++i) {
        number_in_dec += math_repr_of_char(number.at(number.length() - 1 - i)) * std::pow(def_base, i);
    }

    std::string new_number;
    do {
        new_number.push_back(char_repr_of_num(number_in_dec % new_base));
        number_in_dec /= new_base;
    } while (number_in_dec);
    std::reverse(new_number.begin(), new_number.end());
    return new_number;
}



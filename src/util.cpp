/*-------------------------------*
 |        MOlex Assembler        |
 |             Utils             |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#include <fstream>

#include "../include/util.hpp"

using namespace mxasm;


void               mxasm::
validate_source_file_path(const std::string path)
{
    if (path.length() < 5 or path.substr(path.length() - 4, 4) != ".asm") {
        throw std::invalid_argument("Wrong source file extension. Must be [name].asm\n");
    }
}

source_listing   mxasm::
open_source_code(const std::string file_path)
{
    std::ifstream reader(file_path);
    if (not reader.is_open()) {
        throw std::runtime_error("Source code file opening error!\n");
    }

    source_listing source_code;
    std::string buffer;
    std::size_t line_number {1};

    while (std::getline(reader, buffer)) {
        source_code.emplace_back(line_number, buffer);
        ++line_number;
    }

    return source_code;
}

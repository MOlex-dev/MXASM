/*-------------------------------*
 |        MOlex Assembler        |
 |             Utils             |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#pragma once

#include <string>
#include <list>

namespace mxasm
{
    typedef uint8_t byte;

    void validate_source_file_path(const std::string path);
    std::list<std::pair<std::size_t, std::string>> open_source_code(const std::string file_path);
}

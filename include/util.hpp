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
    typedef std::list<std::pair<std::size_t, std::string>> source_listing;

    void           validate_source_file_path(const std::string path);
    source_listing open_source_code(const std::string file_path);
}

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
    file_reader.close();
    return source_code;
}

void                    mxasm::
write_program_to_file(const std::vector<byte_t> &program, const std::string out_name)
{
    std::ofstream file_writer(out_name, std::ios_base::binary);
    if (not file_writer.is_open()) {
        throw arguments_exception("Can't create program binary file!");
    }
    for (const auto byte : program) {
        file_writer << byte;
    }
    file_writer.close();
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

uint8_t                 mxasm::
get_char_digit_value(const char c) noexcept
{
    const char d = toupper(c);
    if (d >= '0' and d <= '9') return d - '0';
    if (d >= 'A' and d <= 'Z') return d - 'A' + 0xA;
    return 0;
}

uint64_t                mxasm::
string_to_number(const std::string str, const uint8_t base) noexcept
{
    uint64_t result {0};

    for (std::size_t i = 0; i < str.length(); ++i) {
        result += get_char_digit_value(str.at(str.length() - 1 - i)) * std::pow(base, i);
    }
    return result;
}

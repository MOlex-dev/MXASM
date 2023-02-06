/*-----------------------------------*
 |          MOlex Assembler          |
 |         Custom Exceptions         |
 |                                   |
 |         Author: MOlex-dev         |
 *-----------------------------------*/

#pragma once

#include <string>


namespace mxasm
{
    class mxasm_exception
    {
    public:
        explicit mxasm_exception(std::string message) noexcept;

        std::string message() const noexcept;
        std::string type() const noexcept;

    protected:
        void exception_type(std::string type) noexcept;

    private:
        std::string m_exception_message;
        std::string m_exception_type;
    };
}

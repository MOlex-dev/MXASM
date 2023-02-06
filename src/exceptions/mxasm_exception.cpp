/*-----------------------------------*
 |          MOlex Assembler          |
 |         Custom Exceptions         |
 |                                   |
 |         Author: MOlex-dev         |
 *-----------------------------------*/

#include "../../include/exceptions/mxasm_exception.hpp"

using namespace mxasm;


mxasm_exception::
mxasm_exception(std::string message) noexcept
    : m_exception_message {std::move(message)}, m_exception_type {"mxasm_exception"} {}


std::string             mxasm_exception::
message() const noexcept
{ return m_exception_message; }

std::string             mxasm_exception::
type() const noexcept
{ return m_exception_type; }


void                    mxasm_exception::
exception_type(std::string type) noexcept
{ m_exception_type = std::move(type); }

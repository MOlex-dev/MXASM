/*-------------------------------*
 |        MOlex Assembler        |
 |       Serializable Token      |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#include "../include/serializable_token.hpp"

using namespace mxasm;
using st_kind = mxasm::serializable_token::st_kind;
using st_command = mxasm::serializable_token::st_command;


serializable_token::
serializable_token(const st_kind token_kind)
    : m_kind {token_kind} {}


st_kind                 serializable_token::
kind() const noexcept
{ return m_kind; }

st_command              serializable_token::
command() const noexcept
{ return m_command; }

std::vector<word_t>     serializable_token::
byteline() const noexcept
{ return m_byteline; }

word_t                  serializable_token::
number() const noexcept
{ return m_number; }


void                    serializable_token::
kind(const st_kind kind) noexcept
{ m_kind = kind; }

void                    serializable_token::
command(const st_command command) noexcept
{ m_command = command; }

void                    serializable_token::
byteline(std::vector<word_t> line) noexcept
{ m_byteline = std::move(line); }

void                    serializable_token::
number(const word_t value) noexcept
{ m_number = value; }


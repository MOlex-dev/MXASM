/*-------------------------------*
 |        MOlex Assembler        |
 |      Serializable Token       |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#include "../include/serializable_token.hpp"

using namespace mxasm;
using st_kind = mxasm::serializable_token::st_kind;
using st_command = mxasm::serializable_token::st_command;
using st_directive = mxasm::serializable_token::st_directive;


st_kind            serializable_token::
kind() const noexcept
{ return m_kind; }

st_command         serializable_token::
command() const noexcept
{ return  m_command; }

st_directive       serializable_token::
directive() const noexcept
{ return m_directive; }

word               serializable_token::
number() const noexcept
{ return m_number; }

std::vector<byte>  serializable_token::
bytes() const noexcept
{ return m_bytes; }


void               serializable_token::
kind(const st_kind &kind) noexcept
{ m_kind = kind; }

void               serializable_token::
command(const st_command &command) noexcept
{ m_command = command; }

void               serializable_token::
directive(const st_directive &directive) noexcept
{ m_directive = directive; }

void               serializable_token::
number(const word number) noexcept
{ m_number = number; }

void               serializable_token::
bytes(const std::vector<byte> &lst) noexcept
{ m_bytes = lst; }

std::ostream &     mxasm::
operator<<(std::ostream &os, const mxasm::serializable_token &token)
{
    //todo: fix here
    return os;
}
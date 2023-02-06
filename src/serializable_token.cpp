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
using st_adr_mode = mxasm::serializable_token::st_cmd_ad_mode;


st_kind            serializable_token::
kind() const noexcept
{ return m_kind; }

st_command         serializable_token::
command() const noexcept
{ return  m_command; }

st_directive       serializable_token::
directive() const noexcept
{ return m_directive; }

word_t               serializable_token::
number() const noexcept
{ return m_number; }

std::vector<byte_t>  serializable_token::
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
number(const word_t number) noexcept
{ m_number = number; }

void               serializable_token::
bytes(const std::vector<byte_t> &lst) noexcept
{ m_bytes = lst; }

st_adr_mode    serializable_token::
adr_mode() const noexcept
{ return m_mode; }

st_adr_mode    serializable_token::
adr_mode(const st_cmd_ad_mode &mode) noexcept
{ return m_mode = mode; }


std::string    serializable_token::lexeme() const noexcept
{ return m_lexeme; }

std::string  serializable_token::
lexeme(const std::string &str) noexcept
{m_lexeme = str; }

//std::ostream &     mxasm::
//operator<<(std::ostream &os, mxasm::serializable_token &token)
//{
//    if (token.kind() == st_kind::LABEL_DECLARATION) {
//        std::cout << "LABEL DECLARATION: " << token.lexeme() << std::endl;
//    } else if (token.kind() == st_kind::DIRECTIVE) {
//        if (token.directive() == st_directive::CODE_POSITION) {
//            std::cout << "CODE POSITION: " << token.number() << std::endl;
//        } else if (token.directive() == st_directive::BYTE_LINE) {
//            std::cout << "Bytes: ";
//            for (const auto &e : token.bytes()) {
//                std::cout << e << ' ';
//            }
//            std::cout << '\n';
//        }
//    } else if (token.kind() == serializable_token::st_kind::COMMAND) {
//        std::cout << token.command_name() << " ";
//        if (token.is_labeled) {
//            std::cout << "labeled";
//        } else {
//            std::cout << token.number();
//        }
//
//    }
//    //todo: fix here
//    return os;
//}
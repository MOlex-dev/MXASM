/*-------------------------------*
 |        MOlex Assembler        |
 |             Parser            |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#pragma once

#include <list>
#include <map>

#include "../include/lexer_token.hpp"
#include "../include/parser_token.hpp"
#include "../include/serializable_token.hpp"
#include "../include/exceptions/parser_excpetion.hpp"
#include "../include/util.hpp"


namespace mxasm
{
    class parser
    {
    public:
        explicit parser(std::list<lexer_token> &lexed_tokens);

        std::list<serializable_token> tokens();

    private:
        std::list<std::list<lexer_token>>  m_lexer_tokens;
        std::list<std::list<parser_token>> m_parser_tokens;
        std::list<serializable_token>      m_tokens;
        exception_list                     m_exceptions;

        void tokenize();
        void validate_numbers_size();
        void validate_code_pos_directives();
        void find_byte_lines();
        void index_and_replace_constants();
        void validate_and_replace_labels();
        void parser_tokens_to_serializable();

        void organize_lexer_tokens(std::list<lexer_token> &lexed_tokens);
        void add_exception(const std::string &exception) noexcept;

        void l_decl(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);



        void validate_end_of_command(const std::list<parser_token>::const_iterator &iter,
                                     const std::list<parser_token>::const_iterator &end);
        void d_code_pos(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);
        void d_byteline(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end,
                        serializable_token::st_kind dir_type);
        void o_opc_imp(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end,
                       serializable_token::st_command opc);

        void o_jmp(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);
        void o_jsr(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);
        void o_asl(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);
        void o_inc(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);
        void o_rol(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);
        void o_dec(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);
        void o_lsr(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);
        void o_ror(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);
        void o_stx(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);
        void o_ldx(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);
        void o_tsb(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);
        void o_ora(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);
        void o_trb(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);
        void o_bit(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);
        void o_and(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);
        void o_eor(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);
        void o_adc(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);
        void o_cpx(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);
        void o_sbc(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);
        void o_sty(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);
        void o_sta(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);
        void o_stz(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);
        void o_ldy(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);
        void o_lda(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);
        void o_cpy(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);
        void o_cmp(std::list<parser_token>::iterator beg, std::list<parser_token>::iterator end);








        parser_token::adr_mode define_addr_mode(std::list<parser_token>::iterator beg,
                                                std::list<parser_token>::iterator end);

    };
}

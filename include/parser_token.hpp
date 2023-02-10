/*-------------------------------*
 |        MOlex Assembler        |
 |          Parser Token         |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#pragma once

#include <string>
#include <map>

#include "../include/util.hpp"
#include "../include/lexer_token.hpp"


namespace mxasm
{
    class parser_token
    {
    public:
        enum class pt_kind
        {
            NUMBER,

            EQUALS,




            DIRECTIVE,

            LABEL_DECLARATION,
            OPCODE,
            STRING,
            LABEL_CALL,
            _IDENTIFIER,

            LEFT_PARENTHESIS,
            RIGHT_PARENTHESIS,
            COMMA,
            HASH,
            LESS,
            GREATER,
        };

        enum class pt_directive
        { CODE_POSITION, BYTE, WORD, MACRO };

        enum class pt_opcode
        {
            ADC, AND, ASL,
            BBR0, BBR1, BBR2, BBR3, BBR4, BBR5, BBR6, BBR7,
            BBS0, BBS1, BBS2, BBS3, BBS4, BBS5, BBS6, BBS7,
            BCC, BCS, BEQ, BIT, BMI, BNE, BPL, BRA, BRK, BVC, BVS,
            CLC, CLD, CLI, CLV, CMP, CPY, CPX,
            DEC, DEX, DEY,
            EOR,
            INC, INX, INY,
            JMP, JSR,
            LDA, LDX, LDY, LSR,
            NOP,
            ORA,
            PHA, PHP, PHX, PHY, PLA, PLP, PLX, PLY,
            RMB0, RMB1, RMB2, RMB3, RMB4, RMB5, RMB6, RMB7,
            ROL, ROR, RTI, RTS,
            SBC, SEC, SED, SEI,
            SMB0, SMB1, SMB2, SMB3, SMB4, SMB5, SMB6, SMB7,
            STA, STP, STX, STY, STZ,
            TAX, TAY, TRB, TSB, TSX, TXA, TXS, TYA,
            WAI,
            REGISTER_X, REGISTER_Y
        };

        explicit parser_token(lexer_token other);

        lexer_token          base_token() const noexcept;
        pt_kind              kind() const noexcept;
        std::size_t          v_number() const noexcept;
        std::string          v_lexeme() const noexcept;
        std::size_t          row() const noexcept;
        std::size_t          column() const noexcept;
        pt_opcode            v_opcode() const noexcept;
        pt_directive         v_directive() const noexcept;
        std::vector<word_t> &v_byteline() noexcept;

        void base_token(const lexer_token new_base_token);
        void kind(const pt_kind opcode);
        void v_number(const std::size_t number);
        void v_lexeme(const std::string lexeme);
        void row(const std::size_t row_value);
        void column(const std::size_t column_value);
        void v_opcode(const pt_opcode opcode);
        void v_directive(const pt_directive directive);
        void v_byteline(std::vector<word_t> &vc) noexcept;

        static std::string  pt_kind_to_string(const pt_kind kind) noexcept;
        static std::string  pt_opcode_to_string(const pt_opcode opcode) noexcept;
        static std::string  pt_directive_to_string(const pt_directive directive) noexcept;
        static pt_opcode    get_opcode_by_name(const std::string lexeme) noexcept;
        static pt_directive get_directive_by_name(const std::string lexeme) noexcept;
        static bool         is_opcode_or_register(const std::string lexeme) noexcept;

    private:
        std::size_t m_row;
        std::size_t m_column;
        lexer_token m_base_token;
        pt_kind     m_kind;

        std::size_t         m_v_number;
        std::string         m_v_lexeme;
        pt_opcode           m_v_opcode;
        pt_directive        m_v_directive;
        std::vector<word_t> m_v_byteline;



        const static std::map<pt_kind, std::string>      pt_kind_string;
        const static std::map<pt_opcode, std::string>    pt_opcode_string;
        const static std::map<pt_directive, std::string> pt_directive_string;

    };

    std::ostream &operator<<(std::ostream &os, const parser_token::pt_kind &kind);
    std::ostream &operator<<(std::ostream &os, const parser_token::pt_opcode &kind);
    std::ostream &operator<<(std::ostream &os, const parser_token::pt_directive &directive);
}

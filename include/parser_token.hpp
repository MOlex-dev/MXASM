/*-------------------------------*
 |        MOlex Assembler        |
 |         Parser Token          |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#pragma once

#include <string>
#include <map>
#include <list>

#include "../include/util.hpp"


namespace mxasm
{
    class parser_token
    {
    public:
        enum class pt_kind
        {
            DIRECTIVE_CODE_POSITION,
            DIRECTIVE_MACRO,
            DIRECTIVE_BYTE,
            DIRECTIVE_WORD,

            REGISTER_X,
            REGISTER_Y,

            LABEL_DECLARATION,
            IDENTIFIER,           // Is macro or label name
            NUMBER,
            STRING,

            COMMA,
            HASH,
            LEFT_PARENTHESIS,
            RIGHT_PARENTHESIS,
            LESS,
            GREATER,
            EQUALS,

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
        };

        parser_token() = default;
        parser_token(const pt_kind &kind, const std::string lexeme, const std::size_t row, const std::size_t column);


        std::size_t row() const noexcept;
        std::size_t column() const noexcept;
        std::string lexeme() const noexcept;
        pt_kind     kind() const noexcept;

        void row(const std::size_t row) noexcept;
        void column(const std::size_t column) noexcept;
        void lexeme(std::string lexeme) noexcept;
        void kind(const pt_kind kind) noexcept;

        bool               is(const pt_kind &kind) const noexcept;
        bool               is_not(const pt_kind &kind) const noexcept;
        static std::string pt_kind_to_str(const pt_kind &kind) noexcept;
        static bool        value_exists_in_opcodes(const std::string &str) noexcept;
        static pt_kind     get_opcode_name(const std::string &str);

    private:
        pt_kind     m_kind   {};
        std::size_t m_row    {};
        std::size_t m_column {};
        std::string m_lexeme {};

        static std::map<pt_kind, std::string> pt_kind_str;
        friend std::ostream &operator<<(std::ostream &os, const mxasm::parser_token::pt_kind &kind);
    };

    typedef std::list<parser_token>                        parser_tokens;
    std::ostream &operator<<(std::ostream &os, const mxasm::parser_token::pt_kind &kind);
}
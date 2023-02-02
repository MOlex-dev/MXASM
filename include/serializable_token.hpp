/*-------------------------------*
 |        MOlex Assembler        |
 |      Serializable Token       |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#pragma once

#include <list>
#include <vector>

#include "../include/util.hpp"


namespace mxasm
{
    class serializable_token
    {
    public:
        enum class st_kind
        { COMMAND, DIRECTIVE, LABEL_DECLARATION };
        enum class st_directive
        { CODE_POSITION, BYTE_LINE };

        enum st_command
        {
            BRK_stk = 0x00, ORA_izx = 0x01,                 TSB_zpg = 0x04, ORA_zpg = 0x05, ASL_zpg = 0x06, RMB0_zpg = 0x07, PHP_stk = 0x08, ORA_imm = 0x09, ASL_a   = 0x0A,                 TSB_abs = 0x0C, ORA_abs = 0x0D, ASL_abs = 0x0E, BBR0_zpr = 0x0F,
            BPL_rel = 0x10, ORA_izy = 0x11, ORA_izp = 0x12, TRB_zpg = 0x14, ORA_zpx = 0x15, ASL_zpx = 0x16, RMB1_zpg = 0x17, CLC_imp = 0x18, ORA_aby = 0x19, INC_a   = 0x1A,                 TRB_abs = 0x1C, ORA_abx = 0x1D, ASL_abx = 0x1E, BBR1_zpr = 0x1F,
            JSR_abs = 0x20, AND_izx = 0x21,                 BIT_zpg = 0x24, AND_zpg = 0x25, ROL_zpg = 0x26, RMB2_zpg = 0x27, PLP_stk = 0x28, AND_imm = 0x29, ROL_a   = 0x2A,                 BIT_abs = 0x2C, AND_abs = 0x2D, ROL_abs = 0x2E, BBR2_zpr = 0x2F,
            BMI_rel = 0x30, AND_izy = 0x31, AND_izp = 0x32, BIT_zpx = 0x34, AND_zpx = 0x35, ROL_zpx = 0x36, RMB3_zpg = 0x37, SEC_imp = 0x38, AND_aby = 0x39, DEC_a   = 0x3A,                 BIT_abx = 0x3C, AND_abx = 0x3D, ROL_abx = 0x3E, BBR3_zpr = 0x3F,
            RTI_stk = 0x40, EOR_izx = 0x41,                                 EOR_zpg = 0x45, LSR_zpg = 0x46, RMB4_zpg = 0x47, PHA_stk = 0x48, EOR_imm = 0x49, LSR_a   = 0x4A,                 JMP_abs = 0x4C, EOR_abs = 0x4D, LSR_abs = 0x4E, BBR4_zpr = 0x4F,
            BVC_rel = 0x50, EOR_izy = 0x51, EOR_izp = 0x52,                 EOR_zpx = 0x55, LSR_zpx = 0x56, RMB5_zpg = 0x57, CLI_imp = 0x58, EOR_aby = 0x59, PHY_stk = 0x5A,                                 EOR_abx = 0x5D, LSR_abx = 0x5E, BBR5_zpr = 0x5F,
            RTS_stk = 0x60, ADC_izx = 0x61,                 STZ_zpg = 0x64, ADC_zpg = 0x65, ROR_zpg = 0x66, RMB6_zpg = 0x67, PLA_stk = 0x68, ADC_imm = 0x69, ROR_a   = 0x6A,                 JMP_ind = 0x6C, ADC_abs = 0x6D, ROR_abs = 0x6E, BBR6_zpr = 0x6F,
            BVS_rel = 0x70, ADC_izy = 0x71, ADC_izp = 0x72, STZ_zpx = 0x74, ADC_zpx = 0x75, ROR_zpx = 0x76, RMB7_zpg = 0x77, SEI_imp = 0x78, ADC_aby = 0x79, PLY_stk = 0x7A,                 JMP_iax = 0x7C, ADC_abx = 0x7D, ROR_abx = 0x7E, BBR7_zpr = 0x7F,
            BRA_rel = 0x80, STA_izx = 0x81,                 STY_zpg = 0x84, STA_zpg = 0x85, STX_zpg = 0x86, SMB0_zpg = 0x87, DEY_imp = 0x88, BIT_imm = 0x89, TXA_imp = 0x8A,                 STY_abs = 0x8C, STA_abs = 0x8D, STX_abs = 0x8E, BBS0_zpr = 0x8F,
            BCC_rel = 0x90, STA_izy = 0x91, STA_izp = 0x92, STY_zpx = 0x94, STA_zpx = 0x95, STX_zpy = 0x96, SMB1_zpg = 0x97, TYA_imp = 0x98, STA_aby = 0x99, TXS_imp = 0x9A,                 STZ_abs = 0x9C, STA_abx = 0x9D, STZ_abx = 0x9E, BBS1_zpr = 0x9F,
            LDY_imm = 0xA0, LDA_izx = 0xA1, LDX_izp = 0xA2, LDY_zpg = 0xA4, LDA_zpg = 0xA5, LDX_zpg = 0xA6, SMB2_zpg = 0xA7, TAY_imp = 0xA8, LDA_imm = 0xA9, TAX_imp = 0xAA,                 LDY_abs = 0xAC, LDA_abs = 0xAD, LDX_abs = 0xAE, BBS2_zpr = 0xAF,
            BCS_rel = 0xB0, LDA_izy = 0xB1, LDA_izp = 0xB2, LDY_zpx = 0xB4, LDA_zpx = 0xB5, LDX_zpy = 0xB6, SMB3_zpg = 0xB7, CLV_imp = 0xB8, LDA_aby = 0xB9, TSX_imp = 0xBA,                 LDY_abx = 0xBC, LDA_abx = 0xBD, LDX_aby = 0xBE, BBS3_zpr = 0xBF,
            CPY_imm = 0xC0, CMP_izx = 0xC1,                 CPY_zpg = 0xC4, CMP_zpg = 0xC5, DEC_zpg = 0xC6, SMB4_zpg = 0xC7, INY_imp = 0xC8, CMP_imm = 0xC9, DEX_imp = 0xCA, WAI_imp = 0xCB, CPY_abs = 0xCC, CMP_abs = 0xCD, DEC_abs = 0xCE, BBS4_zpr = 0xCF,
            BNE_rel = 0xD0, CMP_izy = 0xD1, CMP_izp = 0xD2,                 CMP_zpx = 0xD5, DEC_zpx = 0xD6, SMB5_zpg = 0xD7, CLD_imp = 0xD8, CMP_aby = 0xD9, PHX_stk = 0xDA, STP_imp = 0xDB,                 CMP_abx = 0xDD, DEC_abx = 0xDE, BBS5_zpr = 0xDF,
            CPX_imm = 0xE0, SBC_izx = 0xE1,                 CPX_zpg = 0xE4, SBC_zpg = 0xE5, INC_zpg = 0xE6, SMB6_zpg = 0xE7, INX_imp = 0xE8, SBC_imm = 0xE9, NOP_imp = 0xEA,                 CPX_abs = 0xEC, SBC_abs = 0xED, INC_abs = 0xEE, BBS6_zpr = 0xEF,
            BEQ_rel = 0xF0, SBC_izy = 0xF1, SBC_izp = 0xF2,                 SBC_zpx = 0xF5, INC_zpx = 0xF6, SMB7_zpg = 0xF7, SED_imp = 0xF8, SBC_aby = 0xF9, PLX_stk = 0xFA,                                 SBC_abx = 0xFD, INC_abx = 0xFE, BBS7_zpr = 0xFF
        };

        enum class st_cmd_ad_mode
        {
            IMM,
            ABS, ABX, ABY,
            IND, IAX,
            REL,
            STK, IMP, A,
            ZPG, ZPR, ZPX, ZPY,
            IZP, IZX, IZY,
        };

        st_kind           kind() const noexcept;
        st_command        command() const noexcept;
        st_directive      directive() const noexcept;
        word              number() const noexcept;
        std::vector<byte> bytes() const noexcept;

        void kind(const st_kind &kind) noexcept;
        void command(const st_command &command) noexcept;
        void directive(const st_directive &directive) noexcept;
        void number(const word number) noexcept;
        void bytes(const std::vector<byte> &lst) noexcept;

    private:
        st_kind      m_kind;
        st_command   m_command;
        st_directive m_directive;

        word              m_number;
        std::vector<byte> m_bytes;

        friend std::ostream &operator<<(std::ostream &os, const mxasm::serializable_token &token);
    };

    typedef std::list<serializable_token> serializable_tokens;
    std::ostream &operator<<(std::ostream &os, const mxasm::serializable_token &token);
}



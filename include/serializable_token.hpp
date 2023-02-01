/*-------------------------------*
 |        MOlex Assembler        |
 |      Serializable Token       |
 |                               |
 |       Author: MOlex-dev       |
 *-------------------------------*/

#pragma once

#include <list>


namespace mxasm
{
    class serializable_token
    {
    public:
        enum st_kind
        {
            BRK_stk = 0x00,



        };



    private:
        st_kind m_kind;

    };

    typedef std::list<serializable_token> serializable_tokens;
}



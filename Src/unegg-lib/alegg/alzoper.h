#ifndef __ALZ_OPERATOR_H__
#define __ALZ_OPERATOR_H__

#include "../../common/seven_stream.h"
#include "alzstruct.h"

namespace NArchive
{
    namespace NAlz
    {
        seven_istream& operator >> (seven_istream& _Istr, HEADER& p);
        seven_istream& operator >> (seven_istream& _Istr, FILE_INFO& p);
    }
}

#endif

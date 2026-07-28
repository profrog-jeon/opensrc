#ifndef __ALZ_STRUCTURE_H__
#define __ALZ_STRUCTURE_H__

#include "../../Type.h"

namespace NArchive
{
    namespace NAlz
    {
        enum FormatHeaderSignature
        {
            fhsAlz = 0x015A4C41,
            fhsFile = 0x015A4C42,
            fhsEnd = 0x015A4C43,
            fhsComment = 0x015A4C45,
            fhsSplit = 0x035A4C43,
            fhsEndFile = 0x025A4C43,

            fhsLast = 0x00000000
        };

        struct HEADER
        {
            UInt16 version;
            UInt16 id;
        };

        struct FILE_INFO
        {
            std::basic_string<EGG_TCHAR> filename;
            DWORD attributes;
#if 0
            UInt64 lastModified;
#endif

            struct BLOCK_INFO
            {
                enum CompressMethod
                {
                    cmStore, cmDeprecated1, cmDeflate, cmDeprecated2
                };
                UInt16 compressMethod;
                UInt64 unpackSize;
                UInt64 packSize;
                UInt32 crc;
            } blockInfo;

            struct ZIP_CRYPTO
            {
                enum
                {
                    cryptoDataSize = 12
                };
                BYTE verifyData[cryptoDataSize];
            } zipCrypto;
        };
    }
}

#endif

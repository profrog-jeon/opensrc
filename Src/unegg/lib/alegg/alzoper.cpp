#include "StdAfx.h"
#include "alzoper.h"

#ifndef _WIN32
#include <iconv.h>
#endif

namespace NArchive
{
    namespace NAlz
    {
        seven_istream& operator >> (seven_istream& _Istr, HEADER& p)
        {
            _Istr >> p.version >> p.id;
            return _Istr;
        }

        enum FileAttributes
        {
            faReadonly = 0x01,
            faHidden = 0x02,
            faSystem = 0x04,
            faDeprecated1 = 0x08,
            faDirectory = 0x10,
            faArchive = 0x20,
            faSymLink = 0x40,

            faAnyFile = 0x47,

            faLast = 0x00
        };

        enum FileBitFlags
        {
            fbfEncrypted = 0x01,
            fbfCommented = 0x08,
            fbfSizeMask = 0xF0,

            fbfLast = 0x00
        };

        inline UInt32 GetSizeLength(UInt16 bifFlags)
        {
            return (int)((bifFlags & fbfSizeMask) >> 4);
        }

        inline HRESULT GetInteger(const BYTE* from, int size, UInt64* to)
        {
            HRESULT ret = S_OK;
            switch (size)
            {
            case 1:     *to = *(const BYTE*)from;       break;
            case 2:     *to = *(const UInt16*)from;     break;
            case 4:     *to = *(const UInt32*)from;     break;
            case 8:     *to = *(const UInt64*)from;     break;
            default:    ret = E_INVALIDARG;             break;
            }
            return ret;
        }

        seven_istream& operator >> (seven_istream& _Istr, FILE_INFO& p)
        {
            UInt16 filenameLen;
            BYTE attributes;
            UInt32 datetime;
            UInt16 bitFlags;
            _Istr >> filenameLen >> attributes >> datetime >> bitFlags;

            UInt32 processedSize;
            if (bitFlags & fbfSizeMask)
            {
                _Istr >> p.blockInfo.compressMethod >> p.blockInfo.crc;

                UInt32 sizeLen = GetSizeLength(bitFlags);
                BYTE buffer[8];
                TINOK(_Istr.read(buffer, sizeLen, &processedSize));
                TINOK(GetInteger(buffer, sizeLen, &p.blockInfo.packSize));
                TINOK(_Istr.read(buffer, sizeLen, &processedSize));
                TINOK(GetInteger(buffer, sizeLen, &p.blockInfo.unpackSize));
            }
            else
            {
                p.blockInfo.compressMethod = 0;
                p.blockInfo.crc = 0;
                p.blockInfo.packSize = 0;
                p.blockInfo.unpackSize = 0;
            }

            if (filenameLen)
            {
                HRESULT result;
                char* filename = new char[filenameLen + 1];
                if ((result = _Istr.read(filename, (UInt32)filenameLen, &processedSize)) == S_OK)
                {
                    if (filenameLen == processedSize)
                    {
                        filename[filenameLen] = 0;
                        LPSTR pszTemp;
#ifdef _WIN32
                        while ((pszTemp = strchr(filename, '/')) != NULL)
                        {
                            *pszTemp = '\\';
                        }
                        p.filename = wstring(filename);
#else
                        while ((pszTemp = strchr(filename, '\\')) != NULL)
                        {
                            *pszTemp = '/';
                        }

                        size_t inbytes = filenameLen;
                        size_t outbytes = filenameLen * 2;

                        iconv_t cd = iconv_open("UTF-8", "EUC-KR");
                        char* filename_utf8 = (char*)malloc(outbytes);
                        memset(filename_utf8, 0, outbytes);

                        char* inbuffer = filename;
                        char* outbuffer = filename_utf8;
                        iconv(cd, &inbuffer, &inbytes, &outbuffer, &outbytes);

                        p.filename = filename_utf8;
                        free(filename_utf8);
                        iconv_close(cd);
#endif
                    }
                    else
                    {
                        result = HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
                    }
                }
                delete[] filename;

                if (result != S_OK)
                {
                    throw result;
                }
            }

            if (bitFlags & fbfEncrypted)
            {
                TINOK(_Istr.read(p.zipCrypto.verifyData, FILE_INFO::ZIP_CRYPTO::cryptoDataSize, &processedSize));
                if (processedSize != FILE_INFO::ZIP_CRYPTO::cryptoDataSize)
                {
                    throw HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
                }
            }
            else
            {
                memset(p.zipCrypto.verifyData, 0, FILE_INFO::ZIP_CRYPTO::cryptoDataSize);
            }

            p.attributes = 0;
            if (attributes & faReadonly)
            {
                p.attributes |= FILE_ATTRIBUTE_READONLY;
            }
            if (attributes & faHidden)
            {
                p.attributes |= FILE_ATTRIBUTE_HIDDEN;
            }
            if (attributes & faSystem)
            {
                p.attributes |= FILE_ATTRIBUTE_SYSTEM;
            }
            if (attributes & faDirectory)
            {
                p.attributes |= FILE_ATTRIBUTE_DIRECTORY;
            }

            return _Istr;
        }
    }
}

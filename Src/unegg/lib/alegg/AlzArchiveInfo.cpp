#include "StdAfx.h"
#include "AlzArchiveInfo.h"

#include "../../common/IPassword.h"
#include "../../common/PropID.h"
#include "../PropVariant.h"

#include "../../common/StoreCoder.h"
#include "../../common/DeflateCoder.h"
#include "../../common/CoderImpl.h"

#include "../../common/ZipCrypto.h"

#include "PropService.h"

#include "alzoper.h"

namespace NArchive
{
    namespace NAlz
    {
        CPackedFileInfo::CPackedFileInfo(seven_istream& sis)
        {
            TINOK(LoadPackedFileInfo(sis));
        }

        bool CPackedFileInfo::IsDirectory() const
        {
            return (attributes & FILE_ATTRIBUTE_DIRECTORY);
        }

        bool CPackedFileInfo::IsEncrypted() const
        {
            static BYTE zeroData[ZIP_CRYPTO::cryptoDataSize] = { 0, };
            return (memcmp(zipCrypto.verifyData, zeroData, ZIP_CRYPTO::cryptoDataSize) != 0);
        }

        int CPackedFileInfo::ExtractTo(IInStream* inStream, ISequentialOutStream* outStream, EXTRACT_CALLBACK& callback)
        {
            CComPtr<IInStream> instream(inStream);
            CComPtr<ISequentialOutStream> outstream(outStream);
            int ret = NArchive::NExtract::NOperationResult::kOK;

            if (IsEncrypted())
            {
                if (password_.size() == 0)
                {
                    CComPtr<ICryptoGetTextPassword> cryptoGetTextPassword;
                    if (callback.extractCallback->QueryInterface(IID_ICryptoGetTextPassword, (void**)&cryptoGetTextPassword) == S_OK)
                    {
                        BSTR password;
                        if ((ret = cryptoGetTextPassword->CryptoGetTextPassword(&password)) == S_OK)
                        {
                            UINT cbPassword = SysStringByteLen(password);
                            LPWSTR pszPassword = (LPWSTR)malloc(cbPassword + sizeof(WCHAR));
                            memset(pszPassword, 0, cbPassword + sizeof(WCHAR));
                            memcpy(pszPassword, password, cbPassword);
                            password_ = tstring(pszPassword);
                            free(pszPassword);
                            SysFreeString(password);
                        }
                    }
                }

                if (password_.size() != 0)
                {
                    if (callback.decryptor)
                    {
                        CZipDecryptor* zipDecryptor = dynamic_cast<CZipDecryptor*>((IDecryptor*)(callback.decryptor));
                        if (zipDecryptor)
                        {
                            zipDecryptor->RestoreKeys();
                        }
                    }
                    else
                    {
                        callback.decryptor = new CZipDecryptor(string(password_));
                    }

                    if (callback.decryptor)
                    {
                        unsigned char data[ZIP_CRYPTO::cryptoDataSize];
                        memcpy(data, zipCrypto.verifyData, ZIP_CRYPTO::cryptoDataSize);
                        if ((callback.decryptor->Decrypt(data, ZIP_CRYPTO::cryptoDataSize) != S_OK)
                            || (data[11] != (blockInfo.crc >> 24)))
                        {
                            ret = NArchive::NExtract::NOperationResult::kWrongPassword;
                        }
                    }
                    else
                    {
                        ret = NArchive::NExtract::NOperationResult::kWrongPassword;
                    }

                    if (ret == NArchive::NExtract::NOperationResult::kWrongPassword)
                    {
                        password_.clear();
                    }
                }
                else if (ret != HRESULT_FROM_WIN32(ERROR_CANCELLED))
                {
                    ret = NArchive::NExtract::NOperationResult::kWrongPassword;
                }
            }

            if (ret == NArchive::NExtract::NOperationResult::kOK)
            {
                CComPtr<IDecoder> decoder;
                switch (blockInfo.compressMethod)
                {
                case BLOCK_INFO::cmStore:       decoder = new CStoreCoder(callback.buffer, callback.coderCallback);     break;
                case BLOCK_INFO::cmDeflate:     decoder = new CDeflateCoder(callback.buffer, callback.coderCallback);   break;
                }

                if (decoder)
                {
                    UInt64 pos;
                    if ((inStream->Seek((Int64)offset_, SEEK_SET, &pos) == S_OK) && (offset_ == pos))
                    {
                        HRESULT result = decoder->Decode(inStream, outStream, blockInfo.packSize, blockInfo.unpackSize, callback.decryptor);
                        if (result == S_OK)
                        {
                            ret = decoder->GetResult(blockInfo.crc);
                        }
                        else
                        {
                            ret = (int)result;
                        }
                    }
                    else
                    {
                        ret = NArchive::NExtract::NOperationResult::kUnexpectedEnd;
                    }
                }
                else
                {
                    ret = NArchive::NExtract::NOperationResult::kUnsupportedMethod;
                }
            }

            return ret;
        }

        HRESULT CPackedFileInfo::LoadPackedFileInfo(seven_istream& sis)
        {
            sis >> *this;

            HRESULT ret = sis.seek(0, SEEK_CUR, &offset_);
            if (ret == S_OK)
            {
                ret = sis.seek(GetPackedSize(), SEEK_CUR, NULL);
            }
            return ret;
        }


        std::vector<BYTE> g_FileProps
        {
            kpidPath, kpidSize, kpidPackSize,
#if 0
            kpidMTime,
#endif
            kpidComment, kpidAttrib, kpidEncrypted
        };

        std::vector<BYTE> g_ArchiveProps
        {
            kpidComment
        };

        CInArchive::CInArchive()
        {
        }

        CInArchive::~CInArchive()
        {
        }

        HRESULT CInArchive::QueryInterface(REFIID riid, void **ppvObject)
        {
            HRESULT ret = S_OK;
            if (riid == IID_IUnknown)
            {
                *ppvObject = (void*)(IUnknown*)(IInArchive*)this;
                AddRef();
            }
            else if (riid == IID_IInArchive)
            {
                *ppvObject = (void*)(IInArchive*)this;
                AddRef();
            }
            else
            {
                ret = E_NOINTERFACE;
                *ppvObject = NULL;
            }
            return ret;
        }

        HRESULT CInArchive::Open(IInStream *stream, const UInt64*, IArchiveOpenCallback*)
        {
            HRESULT ret = S_OK;
            if (stream)
            {
                std::list<CPackedFileInfo> fileList;

                seven_istream sis(stream);
                UInt32 signature;

                try
                {
                    UInt32 endInfos[4];
                    sis.seek(16, SEEK_END, NULL);
                    sis.read(endInfos, 16, NULL);
                    sis.seek(0, SEEK_SET, NULL);

                    sis >> signature;
                    if (signature == fhsAlz)
                    {
                        HEADER h;
                        sis >> h;
                    }
                    else
                    {
                        ret = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
                    }

                    if (ret == S_OK)
                    {
                        do
                        {
                            sis >> signature;
                            switch (signature)
                            {
                            case fhsFile:   fileList.push_back(CPackedFileInfo(sis));   break;
                            }
                        } while (signature == fhsFile);

                        if (signature == fhsComment)
                        {
                            if (4 < endInfos[1])
                            {
                                ret = LoadComments(sis, endInfos[1] - sizeof(signature));
                            }
                            else
                            {
                                ret = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
                            }
                        }
                    }
                }
                catch (operation_exception& ex)
                {
                    ret = ex.result;
                }
                catch (...)
                {
                    ret = E_FAIL;
                }

                if (ret == S_OK)
                {
                    if (!inStream_)
                    {
                        inStream_ = stream;
                    }

                    if (files_.size() == 0)
                    {
                        files_.reserve(fileList.size());
                        for (auto it = fileList.begin(); it != fileList.end(); ++it)
                        {
                            files_.push_back(std::move(*it));
                        }
                    }
                }
                else if (HRESULT_CODE(ret) == ERROR_BAD_FORMAT)
                {
                    ret = S_FALSE;
                }
            }
            else
            {
                ret = E_INVALIDARG;
            }
            return ret;
        }

        HRESULT CInArchive::Close()
        {
            inStream_.Release();
            return S_OK;
        }

        HRESULT CInArchive::GetNumberOfItems(UInt32 *numItems)
        {
            HRESULT ret = S_OK;
            if (numItems)
            {
                *numItems = files_.size();
            }
            else
            {
                ret = E_INVALIDARG;
            }
            return ret;
        }

        HRESULT CInArchive::Extract(const UInt32* indices, UInt32 numItems, Int32, IArchiveExtractCallback *extractCallback)
        {
            HRESULT ret = S_OK;
            CComPtr<IArchiveExtractCallback> callback(extractCallback);

            UInt64 totalSize = 0;

            bool allFiles = !(indices && (numItems > 0));
            UInt32 itemCount = allFiles ? files_.size() : numItems;
            {
                UInt32 itemIndex = 0;
                for (UInt32 i = 0; (ret == S_OK) && (i < itemCount); i++)
                {
                    itemIndex = allFiles ? i : indices[i];
                    if (itemIndex < files_.size())
                    {
                        totalSize += files_[itemIndex].GetSize();
                    }
                    else
                    {
                        ret = E_INVALIDARG;
                    }
                }
            }

            if (ret == S_OK)
            {
                ret = callback->SetTotal(totalSize);

                EXTRACT_CALLBACK callbackStruct;
                callbackStruct.extractCallback = callback;
                callbackStruct.coderCallback = new CCoderCallback(callback);
                callbackStruct.buffer = new CCodeBuffer();

                int extractResult = 0;

                UInt32 itemIndex = 0;
                UINT64 completed = 0;
                UINT64 fileSize;
                for (UInt32 i = 0; (ret == S_OK) && (i < itemCount); i++)
                {
                    itemIndex = allFiles ? i : indices[i];

                    CComPtr<ISequentialOutStream> outStream;
                    if ((ret = callback->GetStream(itemIndex, &outStream, 0)) == S_OK)
                    {
                        fileSize = files_[itemIndex].GetSize();
                        if (outStream)
                        {
                            callback->PrepareOperation(0);
                            if (fileSize)
                            {
                                extractResult = files_[itemIndex].ExtractTo(inStream_, outStream, callbackStruct);
                            }
                            ret = callback->SetOperationResult(extractResult);
                            if ((HRESULT)extractResult == HRESULT_FROM_WIN32(ERROR_CANCELLED))
                            {
                                ret = (HRESULT)extractResult;
                            }
                        }
                        else
                        {
                            completed += fileSize;
                            callbackStruct.extractCallback->SetCompleted(&completed);
                        }
                    }
                }
            }

            return ret;
        }

        HRESULT CInArchive::GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value)
        {
            HRESULT ret = S_OK;
            if (index < files_.size())
            {
                CPackedFileInfo& info = files_[index];

                NWindows::NCOM::CPropVariant prop;
                switch (propID)
                {
                case kpidPath:          prop = info.GetPath();                              break;
                case kpidSize:          prop = info.GetSize();                              break;
                case kpidPackSize:      prop = info.GetPackedSize();                        break;
#if 0
                case kpidMTime:         prop = info.GetLastModified();                      break;
#endif
                case kpidComment:       prop = GetComment(index);                           break;
                case kpidAttrib:        prop = info.GetAttributes();                        break;
                case kpidEncrypted:     prop = info.IsEncrypted();                          break;
                case kpidIsDir:         prop = info.IsDirectory();                          break;
                default:                ret = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);      break;
                }
                if (ret == S_OK)
                {
                    prop.Detach(value);
                }
            }
            else
            {
                ret = E_INVALIDARG;
            }
            return ret;
        }

        HRESULT CInArchive::GetNumberOfProperties(UInt32 *numProps)
        {
            return PropService::GetPropertyCount(g_FileProps, numProps);
        }

        HRESULT CInArchive::GetPropertyInfo(UInt32 index, BSTR *name, PROPID *propID, VARTYPE *varType)
        {
            return PropService::GetPropertyInfo(g_FileProps, index, name, propID, varType);
        }

        HRESULT CInArchive::GetArchiveProperty(PROPID propID, PROPVARIANT *value)
        {
            HRESULT ret = S_OK;
            NWindows::NCOM::CPropVariant prop;
            switch (propID)
            {
            case kpidComment:   prop = comment_.c_str();                        break;
            default:            ret = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);  break;
            }
            if (ret == S_OK)
            {
                prop.Detach(value);
            }
            return ret;
        }

        HRESULT CInArchive::GetNumberOfArchiveProperties(UInt32 *numProps)
        {
            return PropService::GetPropertyCount(g_ArchiveProps, numProps);
        }

        HRESULT CInArchive::GetArchivePropertyInfo(UInt32 index, BSTR *name, PROPID *propID, VARTYPE *varType)
        {
            return PropService::GetPropertyInfo(g_ArchiveProps, index, name, propID, varType);
        }

        HRESULT CInArchive::LoadComments(seven_istream& sis, UInt32 commentSize)
        {
            HRESULT ret = S_OK;
            UInt32 index;
            UInt16 size;
            UInt32 processedSize;

            UInt16 dataSize = 0;
            LPSTR commentData = NULL;
            while ((ret == S_OK) && commentSize)
            {
                sis >> index >> size;
                if (commentData == NULL)
                {
                    dataSize = size;
                    commentData = new char[dataSize + 1];
                }
                else if (dataSize < size)
                {
                    delete[] commentData;
                    dataSize = size;
                    commentData = new char[dataSize + 1];
                }

                ret = sis.read(commentData, (UInt32)size, &processedSize);
                if (ret == S_OK)
                {
                    if (size == processedSize)
                    {
                        commentData[size] = 0;

                        if (index == 0xFFFFFFFF)
                        {
                            comment_ = wstring(commentData);
                        }
                        else
                        {
                            comments_[index] = wstring(commentData);
                        }
                    }
                    else
                    {
                        ret = HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
                    }
                }

                commentSize -= 6;
                commentSize -= processedSize;
            }

            if (commentData)
            {
                delete[] commentData;
            }
            return ret;
        }

        LPCWSTR CInArchive::GetComment(UInt32 index)
        {
            std::map<UInt32, std::basic_string<WCHAR>>::iterator it = comments_.find(index);
            if (it != comments_.end())
            {
                return it->second.c_str();
            }
            return L"";
        }
    }
}

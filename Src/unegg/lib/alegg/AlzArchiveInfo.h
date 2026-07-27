#ifndef __ALZ_ARCHIVE_INFO_H__
#define __ALZ_ARCHIVE_INFO_H__

#include "../../common/IArchive.h"
#include "../../common/ICrypto.h"
#include "../../common/ICoder.h"

#include "../../common/UnknownImpl.h"
#include "../../common/seven_stream.h"

#include "alzstruct.h"

namespace NArchive
{
    namespace NAlz
    {
        typedef struct _EXTRACT_CALLBACK
        {
            CComPtr<IArchiveExtractCallback> extractCallback;
            CComPtr<ICoderCallback> coderCallback;
            CComPtr<ICodeBuffer> buffer;
            CComPtr<IDecryptor> decryptor;
        } EXTRACT_CALLBACK, *PEXTRACT_CALLBACK;

        class CPackedFileInfo : public FILE_INFO
        {
        public:
            explicit CPackedFileInfo(seven_istream& sis);

            bool IsDirectory() const;
            bool IsEncrypted() const;

            UInt64 GetSize() const { return blockInfo.unpackSize; }
            UInt64 GetPackedSize() const { return blockInfo.packSize; }
            LPCWSTR GetPath() const { return filename.c_str(); }
            UInt32 GetAttributes() const { return attributes; }

            // return is NArchive::NExtract::NOperationResult or HRESULT
            int ExtractTo(IInStream* inStream, ISequentialOutStream* outStream, EXTRACT_CALLBACK& callback);

        private:
            HRESULT LoadPackedFileInfo(seven_istream& sis);

            UInt64 offset_;
            std::basic_string<TCHAR> password_;
        };

        class CInArchive : public IAddRefReleaseImpl<IInArchive>
        {
        public:
            CInArchive();
            virtual ~CInArchive();

            // IUnknown
            virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);

            // IInArchive
            virtual HRESULT STDMETHODCALLTYPE Open(IInStream *stream, const UInt64 *maxCheckStartPosition, IArchiveOpenCallback *openCallback);
            virtual HRESULT STDMETHODCALLTYPE Close();

            virtual HRESULT STDMETHODCALLTYPE GetNumberOfItems(UInt32 *numItems);
            virtual HRESULT STDMETHODCALLTYPE Extract(const UInt32* indices, UInt32 numItems, Int32 testMode, IArchiveExtractCallback *extractCallback);

            virtual HRESULT STDMETHODCALLTYPE GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value);
            virtual HRESULT STDMETHODCALLTYPE GetNumberOfProperties(UInt32 *numProps);
            virtual HRESULT STDMETHODCALLTYPE GetPropertyInfo(UInt32 index, BSTR *name, PROPID *propID, VARTYPE *varType);

            virtual HRESULT STDMETHODCALLTYPE GetArchiveProperty(PROPID propID, PROPVARIANT *value);
            virtual HRESULT STDMETHODCALLTYPE GetNumberOfArchiveProperties(UInt32 *numProps);
            virtual HRESULT STDMETHODCALLTYPE GetArchivePropertyInfo(UInt32 index, BSTR *name, PROPID *propID, VARTYPE *varType);

        private:
            HRESULT LoadComments(seven_istream& sis, UInt32 commentSize);

            LPCWSTR GetComment(UInt32 index);

            std::vector<CPackedFileInfo> files_;
            std::map<UInt32, std::basic_string<WCHAR>> comments_;
            std::basic_string<WCHAR> comment_;

            CComPtr<IInStream> inStream_;
        };
    }
}

#endif

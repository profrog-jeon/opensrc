#ifndef __CALLBACKS_H__
#define __CALLBACKS_H__

#include "UnknownImpl.h"

#include "IArchive.h"
#include "IPassword.h"

class CArchiveOpenCallback: public IAddRefReleaseImpl<
    IArchiveOpenCallback, ICryptoGetTextPassword, IArchiveOpenVolumeCallback
>
{
public:
    START_QUERYINTERFACE
        QUERY_UNKNOWN(IArchiveOpenCallback)
        QUERY_INTERFACE_IID(IArchiveOpenCallback)
        QUERY_INTERFACE_IID(ICryptoGetTextPassword)
        QUERY_INTERFACE_IID(IArchiveOpenVolumeCallback)
    END_QUERYINTERFACE

    STDMETHOD(SetTotal)(const UInt64 *files, const UInt64 *bytes);
    STDMETHOD(SetCompleted)(const UInt64 *files, const UInt64 *bytes);
    STDMETHOD(CryptoGetTextPassword)(BSTR *password);
    STDMETHOD(GetProperty)(PROPID propID, PROPVARIANT *value);
    STDMETHOD(GetStream)(const TCHAR *name, IInStream **inStream);

    CArchiveOpenCallback() {}

private:
    tstring folderPath_;
    tstring fileName_;
};

class CArchiveExtractCallback: public IAddRefReleaseImpl<
    IArchiveExtractCallback,
    ICryptoGetTextPassword
>
{
public:
    START_QUERYINTERFACE
        QUERY_UNKNOWN(IArchiveExtractCallback)
        QUERY_INTERFACE_IID(IArchiveExtractCallback)
        QUERY_INTERFACE_IID(ICryptoGetTextPassword)
    END_QUERYINTERFACE

    // IProgress
    STDMETHOD(SetTotal)(UInt64 size);
    STDMETHOD(SetCompleted)(const UInt64 *completeValue);

    // IArchiveExtractCallback
    STDMETHOD(GetStream)(UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode);
    STDMETHOD(PrepareOperation)(Int32 askExtractMode);
    STDMETHOD(SetOperationResult)(Int32 resultEOperationResult);

    // ICryptoGetTextPassword
    STDMETHOD(CryptoGetTextPassword)(BSTR *aPassword);

    CArchiveExtractCallback(IInArchive* archive, LPCTSTR s, LPCTSTR p)
        : archive_(archive), destPath_(s), password_(p)
    {}

private:
    IInArchive* archive_;
    tstring destPath_;
    tstring password_;
    CComPtr<ISequentialOutStream> outStream_;
};

#endif

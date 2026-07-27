#include "StdAfx.h"

#ifndef _WIN32
#include <unistd.h>
#include <sys/stat.h>
#endif

#include "common/ErrorCodes.h"
#include "common/Callbacks.h"
#include "common/PropID.h"

#include "lib/PropVariant.h"
#include "lib/alegg/EggArchiveInfo.h"
#include "lib/alegg/AlzArchiveInfo.h"

#include "exports.h"

HRESULT OpenArchive(LPCTSTR path, IInArchive** archive)
{
    HRESULT ret = S_OK;

    CComPtr<IInStream> file;

    CArchiveOpenCallback *openCallbackSpec = new CArchiveOpenCallback();
    CComPtr<IArchiveOpenCallback> openCallback(openCallbackSpec);

    ret = openCallbackSpec->GetStream(wstring(path).c_str(), &file);
    if (ret != S_OK)
    {
        return ret;
    }

    CComPtr<IInArchive> _archive = new NArchive::NEgg::CInArchive();

    const UInt64 scanSize = 0;
    ret = _archive->Open(file, &scanSize, openCallback);

    if (ret == S_FALSE)
    {
        _archive = new NArchive::NAlz::CInArchive();
        ret = _archive->Open(file, &scanSize, openCallback);
    }

    if (ret == S_OK)
    {
        *archive = _archive.Detach();
    }

    return ret;
}

HRESULT ExtractArchive(IInArchive* archive, LPCTSTR destination, LPCTSTR password)
{
    CComPtr<IInArchive> _archive(archive);
    CArchiveExtractCallback *extractCallbackSpec = new CArchiveExtractCallback(archive, destination, password);
    CComPtr<IArchiveExtractCallback> extractCallback(extractCallbackSpec);
    return archive->Extract(NULL, -1, false, extractCallback);
}

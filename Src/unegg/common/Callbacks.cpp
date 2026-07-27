#include "StdAfx.h"
#include "Callbacks.h"
#include "ErrorCodes.h"

#ifndef _WIN32
#include <unistd.h>
#include <sys/stat.h>
#endif

#include "PropID.h"
#include "GeneralFileStream.h"

#include "../lib/PropVariant.h"

#ifdef _WIN32
#define FOLDER_SEPARATOR '\\'
#else
#define FOLDER_SEPARATOR '/'
#endif

constexpr char separator_a = FOLDER_SEPARATOR;
constexpr wchar_t separator_w = COL_GLUE(L, FOLDER_SEPARATOR);

#ifdef UNICODE
#define separator separator_w
#else
#define separator separator_a
#endif

void CreateDirectories(LPCTSTR path)
{
#ifdef _WIN32
    if (!CreateDirectory(path, NULL) && (GetLastError() == ERROR_PATH_NOT_FOUND))
#else
    if ((mkdir(tstring(path).toutf8().c_str(), 0700) == -1) && (errno == ENOENT))
#endif
    {
        LPTSTR _path = new TCHAR[_tcslen(path) + 1];
        _tcscpy(_path, path);

        LPTSTR p = _tcsrchr(_path, separator);
        if (p)
        {
            *p = (TCHAR)NULL;
            CreateDirectories(_path);
            *p = separator;

#ifdef _WIN32
            CreateDirectory(path, NULL);
#else
            mkdir(tstring(path).toutf8().c_str(), 0700);
#endif
        }

        delete[] _path;
    }
}

STDMETHODIMP CArchiveOpenCallback::SetTotal(const UInt64 *files, const UInt64 *bytes)
{
    return S_OK;
}

STDMETHODIMP CArchiveOpenCallback::SetCompleted(const UInt64 *files, const UInt64 *bytes)
{
    return S_OK;
}

STDMETHODIMP CArchiveOpenCallback::CryptoGetTextPassword(BSTR *password)
{
    return ercInvalidPassword;
}

STDMETHODIMP CArchiveOpenCallback::GetProperty(PROPID propID, PROPVARIANT *value)
{
    HRESULT ret = S_OK;
    NWindows::NCOM::CPropVariant prop;
    switch (propID)
    {
    case kpidPath:
    {
        tstring path;
        if (folderPath_.size())
        {
            path.format(TEXT("%s%c%s"), folderPath_.c_str(), separator, fileName_.c_str());
        }
        else
        {
            path = fileName_;
        }
        prop = wstring(path).c_str();
        break;
    }
    case kpidName:      prop = wstring(fileName_).c_str();              break;
    default:            ret = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);  break;
    }
    prop.Detach(value);
    return ret;
}

STDMETHODIMP CArchiveOpenCallback::GetStream(const wchar_t *name, IInStream **inStream)
{
    HRESULT ret = S_OK;
    if (name)
    {
        if (wcsrchr(name, separator_w))
        {
            LPWSTR path = new wchar_t[wcslen(name) + 1];
            wcscpy(path, name);

            LPCWSTR folderPath = path;
            LPWSTR fileName = wcsrchr(path, separator_w);
            (*fileName) = (WCHAR)NULL;
            fileName++;

            folderPath_ = tstring(folderPath);
            fileName_ = tstring(fileName);
            delete[] path;
        }
        else
        {
            fileName_ = tstring(name);
        }

        tstring path;
        if (folderPath_.size())
        {
            path.format(TEXT("%s%c%s"), folderPath_.c_str(), separator, fileName_.c_str());
        }
        else
        {
            path = fileName_;
        }

        CInGeneralFileStream* fileStream = new CInGeneralFileStream();
        CComPtr<IInStream> _inStream(fileStream);
        if (fileStream->Open(path))
        {
            *inStream = _inStream.Detach();
        }
        else
        {
#ifdef _WIN32
            if (GetLastError() == ERROR_FILE_NOT_FOUND)
#else
            if (errno == ENOENT)
#endif
            {
                ret = ercFileNotFound;
            }
            else
            {
                ret = ercOpenFailed;
            }
        }
    }
    else
    {
        ret = ercInvalidParameters;
    }
    return ret;
}

STDMETHODIMP CArchiveExtractCallback::SetTotal(UInt64 size)
{
    return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::SetCompleted(const UInt64 *completeValue)
{
    return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::GetStream(UInt32 index,
    ISequentialOutStream **outStream, Int32 askExtractMode)
{
    *outStream = 0;

    if (askExtractMode != NArchive::NExtract::NAskMode::kExtract)
    {
        return S_OK;
    }

    tstring fullPath;
    {
        // Get Name
        NWindows::NCOM::CPropVariant prop;
        RINOK(archive_->GetProperty(index, kpidPath, &prop));

        if (prop.vt != VT_BSTR)
        {
            return E_FAIL;
        }
        fullPath = std::basic_string<TCHAR>(prop.bstrVal, SysStringLen(prop.bstrVal));

        // Zip Slip Vulnerability
#ifdef _WIN32
        fullPath.replace("..\\", "__\\");
#else
        fullPath.replace("../", "__/");
#endif
    }

    bool isDir = false;
    {
        NWindows::NCOM::CPropVariant prop;
        RINOK(archive_->GetProperty(index, kpidIsDir, &prop));
        if (prop.vt == VT_BOOL)
        {
            isDir = (prop.boolVal != VARIANT_FALSE);
        }
        else if (prop.vt != VT_EMPTY)
        {
            return E_FAIL;
        }
    }

    tstring realPath = destPath_ + separator + fullPath;
    if (isDir)
    {
        CreateDirectories(realPath);
    }
    else
    {
        CreateDirectories(realPath.substr(0, realPath.find_last_of(separator)).c_str());

#ifdef _WIN32
        DeleteFile(realPath);
#else
        remove(realPath.toutf8().c_str());
#endif

        COutGeneralFileStream* outFileStreamSpec = new COutGeneralFileStream;
        outStream_ = outFileStreamSpec;
        if (outFileStreamSpec->Create(realPath))
        {
            *outStream = outFileStreamSpec;
            (*outStream)->AddRef();
        }
    }

    return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::PrepareOperation(Int32 askExtractMode)
{
    return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::SetOperationResult(Int32 resultEOperationResult)
{
    outStream_.Release();
    if (resultEOperationResult == NArchive::NExtract::NOperationResult::kWrongPassword)
    {
        return ercInvalidPassword;
    }
    return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::CryptoGetTextPassword(BSTR *aPassword)
{
    if (password_.size() != 0)
    {
        *aPassword = SysAllocString(wstring(password_).c_str());
        return S_OK;
    }
    return ercInvalidPassword;
}

#include "pch.h"
#include "EggHelper.h"
#include "../../Src/unegg/unegg.h"

ECODE ReadEggFile(std::tstring strFilePath, std::list<ST_EGG_ARCHIVE_INTERNAL_FILE_INFO>& outInternals)
{
	ECODE nRet = EC_SUCCESS;

	try
	{
        CComPtr<IInStream> file;

        CArchiveOpenCallback* openCallbackSpec = new CArchiveOpenCallback();
        CComPtr<IArchiveOpenCallback> openCallback(openCallbackSpec);

        nRet = openCallbackSpec->GetStream(core::WCSFromTCS(strFilePath).c_str(), &file);
        if (nRet != S_OK)
            throw core::exception_format(TEXT("GetStream:%s failure"), strFilePath.c_str());

        const UInt64 scanSize = 1 << 23;

        CComPtr<IInArchive> archive = new NArchive::NEgg::CInArchive();
        nRet = archive->Open(file, &scanSize, openCallback);
        if (nRet == S_FALSE)
        {
            archive = new NArchive::NAlz::CInArchive();
            nRet = archive->Open(file, &scanSize, openCallback);
        }
        if (nRet != S_OK)
            throw core::exception_format(TEXT("Unexpected file format."));

        UInt32 numItems = 0;
        HRESULT ret = archive->GetNumberOfItems(&numItems);
        for (UInt32 i = 0; (ret == S_OK) && (i < numItems); i++)
        {
            ST_EGG_ARCHIVE_INTERNAL_FILE_INFO stInternalFileInfo;
            stInternalFileInfo.dwIndex = i;

            NWindows::NCOM::CPropVariant prop;
            ret = archive->GetProperty(i, kpidSize, &prop);
            stInternalFileInfo.qwUncompressedSize = prop.uhVal.QuadPart;
            ret = archive->GetProperty(i, kpidPackSize, &prop);
            stInternalFileInfo.qwCompressedSize = prop.uhVal.QuadPart;

            if ((ret = archive->GetProperty(i, kpidPath, &prop)) != S_OK)
                throw core::exception_format(TEXT("Failed to reading internal file path."));

            if (prop.vt != VT_BSTR)
                throw core::exception_format(TEXT("Unexpected internal file path perperty type:%d"), prop.vt);

            const std::string strFileNameU8 = core::UTF8FromWCS(prop.bstrVal, SysStringLen(prop.bstrVal));
            stInternalFileInfo.strFilePath = core::TCSFromUTF8(strFileNameU8);

            outInternals.push_back(stInternalFileInfo);
        }
	}
	catch (const std::exception& e)
	{
		core::Log_Error("%s", e.what());
		return nRet;
	}

	return EC_SUCCESS;
}

ECODE ExtractEggFile(std::tstring strFilePath, const std::list<ST_EGG_ARCHIVE_INTERNAL_FILE_INFO>& Targets, std::map<std::tstring, std::vector<BYTE>>& outData)
{
    ECODE nRet = EC_SUCCESS;

    try
    {
        nRet = EC_INVALID_ARGUMENT;
        if (Targets.empty())
            throw core::exception_format(TEXT("Target is EMPTY"));

        CComPtr<IInStream> file;

        CArchiveOpenCallback* openCallbackSpec = new CArchiveOpenCallback();
        CComPtr<IArchiveOpenCallback> openCallback(openCallbackSpec);

        nRet = openCallbackSpec->GetStream(core::WCSFromTCS(strFilePath).c_str(), &file);
        if (nRet != S_OK)
            throw core::exception_format(TEXT("GetStream:%s failure"), strFilePath.c_str());

        const UInt64 scanSize = 1 << 23;

        CComPtr<IInArchive> archive = new NArchive::NEgg::CInArchive();
        nRet = archive->Open(file, &scanSize, openCallback);
        if (nRet == S_FALSE)
        {
            archive = new NArchive::NAlz::CInArchive();
            nRet = archive->Open(file, &scanSize, openCallback);
        }
        if (nRet != S_OK)
            throw core::exception_format(TEXT("Unexpected file format."));

        const std::tstring strModuleDir = core::ExtractDirectory(core::GetFileName());
        const std::tstring strTempDir = strModuleDir + TEXT("/temp-egg");
        core::RemoveDirectoryRecursively(strTempDir);
        core::CreateDirectory(strTempDir.c_str());

        std::vector<UInt32> vecIndex;
        for (auto iter : Targets)
            vecIndex.push_back(iter.dwIndex);

        CArchiveExtractCallback* extractCallbackSpec = new CArchiveExtractCallback(archive, core::WCSFromUCS(strTempDir).c_str(), TEXT(""));
        CComPtr<IArchiveExtractCallback> extractCallback(extractCallbackSpec);
        archive->Extract((const UInt32*)vecIndex.data(), (UInt32)vecIndex.size(), false, extractCallback);

        for (auto iter : Targets)
        {
            const std::tstring strFilePath = strTempDir + TEXT("/") + iter.strFilePath;
            if (!core::IsFileExist(strFilePath))
                continue;

            std::vector<BYTE> vecFileContents;
            core::ReadFileContents(strFilePath, vecFileContents);
            DeleteFile(strFilePath.c_str());

            if (vecFileContents.empty())
            {
                core::Log_Warn(TEXT("Extract failure, %s"), iter.strFilePath.c_str());
                continue;
            }

            outData[iter.strFilePath] = std::move(vecFileContents);
        }
    }
    catch (const std::exception& e)
    {
        core::Log_Error("%s", e.what());
        return nRet;
    }

    return EC_SUCCESS;
}

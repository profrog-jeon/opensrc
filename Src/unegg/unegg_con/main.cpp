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

typedef struct _COMMAND_LINE_INFO
{
    enum CommandList
    {
        clList,
        clExtract,
        clVersion,
        clHelp,

        clCount
    };
    int command;

    tstring archivePath;
    tstring destination;
    tstring password;

    _COMMAND_LINE_INFO() : command(clCount) {}
} COMMAND_LINE_INFO, *PCOMMAND_LINE_INFO;

HRESULT Extract(IInArchive* archive, const COMMAND_LINE_INFO& cli)
{
    CComPtr<IInArchive> _archive(archive);
    CArchiveExtractCallback *extractCallbackSpec = new CArchiveExtractCallback(archive, cli.destination.c_str(), cli.password.c_str());
    CComPtr<IArchiveExtractCallback> extractCallback(extractCallbackSpec);
    return archive->Extract(NULL, -1, false, extractCallback);
}

HRESULT List(IInArchive* archive, const COMMAND_LINE_INFO& cli)
{
    CComPtr<IInArchive> _archive(archive);
    UInt32 numItems = 0;
    HRESULT ret = archive->GetNumberOfItems(&numItems);
    for (UInt32 i = 0; (ret == S_OK) && (i < numItems); i++)
    {
        NWindows::NCOM::CPropVariant prop;
        if ((ret = archive->GetProperty(i, kpidPath, &prop)) == S_OK)
        {
            if (prop.vt == VT_BSTR)
            {
#ifdef _WIN32
                string name(std::basic_string<TCHAR>(prop.bstrVal, SysStringLen(prop.bstrVal)));
#else
                std::string name = tstring(std::basic_string<TCHAR>(prop.bstrVal, SysStringLen(prop.bstrVal))).toutf8();
#endif
                printf("%s\n", name.c_str());
            }
            else if (prop.vt != VT_EMPTY)
            {
                ret = E_FAIL;
            }
        }
    }
    return ret;
}

HRESULT CommandWithArchive(const COMMAND_LINE_INFO& cli)
{
    HRESULT ret = S_OK;
    CComPtr<IInStream> file;

    CArchiveOpenCallback *openCallbackSpec = new CArchiveOpenCallback();
    CComPtr<IArchiveOpenCallback> openCallback(openCallbackSpec);

    ret = openCallbackSpec->GetStream(wstring(cli.archivePath).c_str(), &file);
    if (ret != S_OK)
    {
        return ret;
    }

    const UInt64 scanSize = 1 << 23;

    CComPtr<IInArchive> archive = new NArchive::NEgg::CInArchive();
    ret = archive->Open(file, &scanSize, openCallback);

    if (ret == S_FALSE)
    {
        archive = new NArchive::NAlz::CInArchive();
        ret = archive->Open(file, &scanSize, openCallback);
    }

    if (ret == S_OK)
    {
        switch (cli.command)
        {
        case COMMAND_LINE_INFO::clExtract:
            List(archive, cli);
            ret = Extract(archive, cli);
            break;
        case COMMAND_LINE_INFO::clList:         ret = List(archive, cli);           break;
        }
    }

    return ret;
}

void ShowVersion()
{
    printf("\nunegg v1.3\n");
    printf("Copyright(c) 2010 - present ESTsoft Corp. All rights reserved.\n\n");
}

void ShowUsage()
{
    printf("Usage : unegg [commands] [archive filename] [destination path].\n\n");
    printf("Available commands.\n");
    printf("-h\tDisplay this message.\n");
    printf("-v\tDisplay version.\n");
    printf("-l\tDisplay file list in archive.\n");
    printf("-x\tExtract all files to destination path.\n");
    printf("-pPwd\tSpecify password as 'Pwd'.\n");
}

int main(int argc, char* argv[])
{
    HRESULT ret = S_OK;

    COMMAND_LINE_INFO cli;
    if (1 < argc)
    {
        char currentPath[200];
#ifdef _WIN32
        GetCurrentDirectoryA(200, currentPath);
#else
        getcwd(currentPath, 200);
#endif

        for (int i = 1; (ret == S_OK) && (i < argc); i++)
        {
            if (argv[i][0] == '-')
            {
                switch (argv[i][1])
                {
                case 'h':
                case 'H':       cli.command = COMMAND_LINE_INFO::clHelp;        break;
                case 'v':
                case 'V':       cli.command = COMMAND_LINE_INFO::clVersion;     break;
                case 'l':
                case 'L':       cli.command = COMMAND_LINE_INFO::clList;        break;
                case 'x':
                case 'X':       cli.command = COMMAND_LINE_INFO::clExtract;     break;
                case 'p':
                case 'P':       cli.password = &argv[i][2];                     break;
                default:        ret = E_INVALIDARG;                             break;
                }
            }
            else
            {
                if (cli.archivePath.size() == 0)
                {
#ifdef _WIN32
                    cli.archivePath = argv[i];
#else
                    cli.archivePath.fromutf8(argv[i]);
#endif
                }
                else if (cli.destination.size() == 0)
                {
#ifdef _WIN32
                    cli.destination = argv[i];
#else
                    cli.destination.fromutf8(argv[i]);
#endif
                }
            }
        }

        if (cli.destination.size() == 0)
        {
#ifdef _WIN32
            cli.destination = currentPath;
#else
            cli.destination.fromutf8(currentPath);
#endif
        }
    }

    if (ret == S_OK)
    {
        switch (cli.command)
        {
        case COMMAND_LINE_INFO::clExtract:
        case COMMAND_LINE_INFO::clList:
            if (cli.archivePath.size() == 0)
            {
                ret = E_INVALIDARG;
            }
            break;
        case COMMAND_LINE_INFO::clVersion:
        case COMMAND_LINE_INFO::clHelp:
            break;
        default:
            ret = E_INVALIDARG;
            break;
        }
    }

    if (ret == S_OK)
    {
        switch (cli.command)
        {
        case COMMAND_LINE_INFO::clExtract:
        case COMMAND_LINE_INFO::clList:         ret = CommandWithArchive(cli);      break;
        case COMMAND_LINE_INFO::clVersion:      ShowVersion();                      break;
        case COMMAND_LINE_INFO::clHelp:         ShowUsage();                        break;
        }
    }
    else if (ret == E_INVALIDARG)
    {
        printf("Invalid argumets\n\n");
        ShowUsage();
        ret = S_OK;
    }

    switch (ret)
    {
    case S_FALSE:
    case HRESULT_FROM_WIN32(ERROR_BAD_FORMAT):          ret = ercBadFormat;         break;
    case E_NOINTERFACE:                                 ret = ercNoInterface;       break;
    case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):      ret = ercFileNotFound;      break;
    case E_OUTOFMEMORY:                                 ret = ercOutOfMemory;       break;
    case HRESULT_FROM_WIN32(ERROR_HANDLE_EOF):          ret = ercEndOfFile;         break;
    case HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED):       ret = ercUnsupported;       break;
    case HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER):   ret = ercInvalidParameters; break;
    case HRESULT_FROM_WIN32(ERROR_INVALID_FLAGS):       ret = ercInvalidFlags;      break;
    }

    if ((ret != 0) && ((ret < START_ERROR_CODE) || (ercLast < ret)))
    {
        ret = ercUndefined;
    }

    return ret;
}

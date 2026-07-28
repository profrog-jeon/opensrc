#include "pch.h"
#include "EggHelper.h"

int main(void)
{
    std::tstring strArchiveFile = TEXT("../../../tks/Build/Test/FileSignature/sample.alz");

    std::list< ST_EGG_ARCHIVE_INTERNAL_FILE_INFO> Internals;
    ReadEggFile(strArchiveFile, Internals);

    std::map<std::tstring, std::vector<BYTE> > mapData;
    ExtractEggFile(strArchiveFile, Internals, mapData);
    return 0;
}
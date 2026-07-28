#pragma once

struct ST_EGG_ARCHIVE_INTERNAL_FILE_INFO
{
	DWORD dwIndex;
	std::tstring strFilePath;
	QWORD qwCompressedSize;
	QWORD qwUncompressedSize;
};

ECODE ReadEggFile(std::tstring strFilePath, std::list< ST_EGG_ARCHIVE_INTERNAL_FILE_INFO>& outInternals);
ECODE ExtractEggFile(std::tstring strFilePath, const std::list< ST_EGG_ARCHIVE_INTERNAL_FILE_INFO>& Targets, std::map<std::tstring, std::vector<BYTE>>& outData);
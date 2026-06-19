/*
* FileOutputStream.cpp
*
*  Created on: 2011. 3. 25.
*      Author: Jehoon Shin(Revolution)
*
*  Copyright(c) 2010-Present ESTsoft Corp. All rights reserved.
*
*  For conditions of distribution and use, see copyright notice in license.txt
*/

#include "../../stdafx.h"
#include <sys/stat.h>
#include <errno.h>
#include "FileOutputStream.h"
#include <nest/Utils.h>

namespace nest
{

	FileOutputStream::FileOutputStream() :
file_(NULL)
{
	// TODO Auto-generated constructor stub

}

FileOutputStream::~FileOutputStream()
{
	// TODO Auto-generated destructor stub
	Close();
}

int FileOutputStream::Create(const char* virtualName)
{
	Close();

	int res = EnsureFilePath(virtualName);

	if (res == Result::Success)
	{
		file_ = fopen(virtualName, "wb");
		res = file_ != NULL ? Result::Success : Result::WriteError;

		if (res == Result::Success)
		{
			filename_ = virtualName;
		}
	}

	return res;
}

int FileOutputStream::CreateFolder(const char* virtualName)
{
	//return mkdir_r(virtualName, 0755);
	if(core::CreateDirectoryRecursively(virtualName) )
		return Result::Success;
	return Result::Invalid;
}

void FileOutputStream::Close()
{
	if (file_)
	{
		fclose(file_);
		file_ = NULL;
	}
}

size_t FileOutputStream::Write(void* buf, size_t size)
{
	return fwrite(buf, 1, size, file_);
}

int FileOutputStream::EnsureFilePath(const char* path)
{
	int res = Result::Success;
	char* tmp = const_cast<char*> (path);

	char* pos = strrchr(tmp, '/');
	if (pos)
	{
		char backup = *pos;
		*pos = 0;
		res = ForceDirectory(path);
		*pos = backup;
	}
	else
	{
		res = ForceDirectory(path);
	}

	return res;
}

int FileOutputStream::ForceDirectory(const char* path)
{
	if( !core::IsFileExist(path) )
		core::CreateDirectoryRecursively(path);

	return Result::Success;
}

}


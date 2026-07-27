#ifndef __EXPORTS_H__
#define __EXPORTS_H__

#include "common/IArchive.h"
#include "common/ErrorCodes.h"

HRESULT OpenArchive(LPCTSTR path, IInArchive** archive);
HRESULT ExtractArchive(IInArchive* archive, LPCTSTR destination, LPCTSTR password);

#endif

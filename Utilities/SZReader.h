#pragma once
#include "pch.h"
#include "Utilities/ArchiveReader.h"
#include "SevenZip/7z.h"
#include "SevenZip/7zAlloc.h"
#include "SevenZip/7zCrc.h"
#include "SevenZip/7zTypes.h"
#include "SevenZip/7zMemBuffer.h"

class SZReader : public ArchiveReader
{
private:
	CMemBufferInStream _memBufferStream;
	CLookToRead _lookStream;
	CSzArEx _archive;
	ISzAlloc _allocImp{ SzAlloc, SzFree };
	ISzAlloc _allocTempImp{ SzAllocTemp, SzFreeTemp };

protected:
	bool InternalLoadArchive(void* buffer, size_t size);
	vector<string> InternalGetFileList();

public:
	SZReader();
	virtual ~SZReader();

	bool ExtractFile(string filename, vector<uint8_t> &output);
};
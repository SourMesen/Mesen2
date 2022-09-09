#pragma once
#include "pch.h"
#include "miniz.h"
#include "ArchiveReader.h"

class ZipReader : public ArchiveReader
{
private:
	mz_zip_archive _zipArchive;

protected:
	bool InternalLoadArchive(void* buffer, size_t size);
	vector<string> InternalGetFileList();

public:
	ZipReader();
	virtual ~ZipReader();

	bool ExtractFile(string filename, vector<uint8_t> &output);
};
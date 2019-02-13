#pragma once
#include "stdafx.h"

class ArchiveReader
{
protected:
	bool _initialized = false;
	uint8_t* _buffer = nullptr;
	virtual bool InternalLoadArchive(void* buffer, size_t size) = 0;
	virtual vector<string> InternalGetFileList() = 0;
public:
	~ArchiveReader();

	bool LoadArchive(void* buffer, size_t size);
	bool LoadArchive(vector<uint8_t>& data);
	bool LoadArchive(string filename);
	bool LoadArchive(std::istream &in);

	bool GetStream(string filename, std::stringstream &stream);

	vector<string> GetFileList(std::initializer_list<string> extensions = {});
	bool CheckFile(string filename);

	virtual bool ExtractFile(string filename, vector<uint8_t> &output) = 0;

	static shared_ptr<ArchiveReader> GetReader(std::istream &in);
	static shared_ptr<ArchiveReader> GetReader(string filepath);
};
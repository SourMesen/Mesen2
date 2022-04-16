#pragma once
#include "stdafx.h"
#include <sstream>

class VirtualFile
{
private:
	string _path = "";
	string _innerFile = "";
	int32_t _innerFileIndex = -1;
	vector<uint8_t> _data;

	void FromStream(std::istream &input, vector<uint8_t> &output);

	void LoadFile();

public:
	static const std::initializer_list<string> RomExtensions;

	VirtualFile();
	VirtualFile(const string &archivePath, const string innerFile);
	VirtualFile(const string &file);
	VirtualFile(const void *buffer, size_t bufferSize, string fileName = "noname");
	VirtualFile(std::istream &input, string filePath);

	operator std::string() const;
	
	bool IsValid();
	bool IsArchive();
	string GetFilePath();
	string GetFolderPath();
	string GetFileName();
	string GetFileExtension();
	string GetSha1Hash();

	size_t GetSize();

	bool ReadFile(vector<uint8_t> &out);
	bool ReadFile(std::stringstream &out);
	bool ReadFile(uint8_t* out, uint32_t expectedSize);

	uint8_t ReadByte(uint32_t offset);

	bool ApplyPatch(VirtualFile &patch);
};
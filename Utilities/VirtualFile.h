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
	string GetFilePath();
	string GetFolderPath();
	string GetFileName();
	string GetSha1Hash();

	bool ReadFile(vector<uint8_t> &out);
	bool ReadFile(std::stringstream &out);

	bool ApplyPatch(VirtualFile &patch);
};
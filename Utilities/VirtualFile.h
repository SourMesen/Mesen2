#pragma once
#include "pch.h"
#include <sstream>

class VirtualFile
{
private:
	constexpr static int ChunkSize = 256 * 1024;

	string _path = "";
	string _innerFile = "";
	int32_t _innerFileIndex = -1;
	vector<uint8_t> _data;
	int64_t _fileSize = -1;

	vector<vector<uint8_t>> _chunks;
	bool _useChunks = false;

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
	uint32_t GetCrc32();

	size_t GetSize();
	bool CheckFileSignature(vector<string> signatures, bool loadArchives = false);
	void InitChunks();

	bool ReadFile(vector<uint8_t> &out);
	bool ReadFile(std::stringstream &out);
	bool ReadFile(uint8_t* out, uint32_t expectedSize);

	uint8_t ReadByte(uint32_t offset);

	bool ApplyPatch(VirtualFile &patch);

	template<typename T>
	bool ReadChunk(T& container, int start, int length)
	{
		InitChunks();
		if(start < 0 || start + length > GetSize()) {
			//Out of bounds
			return false;
		}

		for(int i = start, end = start + length; i < end; i++) {
			container.push_back(ReadByte(i));
		}

		return true;
	}
};
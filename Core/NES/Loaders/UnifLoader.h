#pragma once
#include "pch.h"
#include <unordered_map>
#include "NES/Loaders/BaseLoader.h"

struct RomData;

class UnifLoader : public BaseLoader
{
private:
	static std::unordered_map<string, int> _boardMappings;

	vector<uint8_t> _prgChunks[16];
	vector<uint8_t> _chrChunks[16];
	string _mapperName;

	void Read(uint8_t*& data, uint8_t& dest);
	void Read(uint8_t*& data, uint32_t& dest);
	void Read(uint8_t*& data, uint8_t* dest, size_t len);
	string ReadString(uint8_t*& data, uint8_t* chunkEnd);
	string ReadFourCC(uint8_t*& data);
	bool ReadChunk(uint8_t*& data, uint8_t* dataEnd, RomData& romData);

public:
	using BaseLoader::BaseLoader;

	static int32_t GetMapperID(string mapperName);
	void LoadRom(RomData& romData, vector<uint8_t>& romFile, bool databaseEnabled);
};
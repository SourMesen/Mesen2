#pragma once
#include "pch.h"
#include "NES/Loaders/BaseLoader.h"

struct NsfHeader;
struct RomData;

class NsfLoader : public BaseLoader
{
private:
	void Read(uint8_t* &data, uint8_t& dest);
	void Read(uint8_t* &data, uint16_t& dest);
	void Read(uint8_t* &data, char* dest, size_t len);

protected:
	void InitializeFromHeader(RomData& romData);
	void InitHeader(NsfHeader& header);

public:
	using BaseLoader::BaseLoader;

	void LoadRom(RomData& romData, vector<uint8_t>& romFile);
};
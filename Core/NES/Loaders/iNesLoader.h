#pragma once
#include "pch.h"
#include "NES/Loaders/BaseLoader.h"

struct RomData;
struct NesHeader;

class iNesLoader : public BaseLoader
{
public:
	using BaseLoader::BaseLoader;

	void LoadRom(RomData& romData, vector<uint8_t>& romFile, NesHeader *preloadedHeader, bool databaseEnabled);
};
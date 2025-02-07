#pragma once
#include "pch.h"
#include "NES/Loaders/BaseLoader.h"

struct RomData;

class FdsLoader : public BaseLoader
{
private:
	static constexpr size_t FdsDiskSideCapacity = 65500;
	static constexpr size_t QdDiskSideCapacity = 65536;

	bool _useQdFormat = false;

private:
	void AddGaps(vector<uint8_t>& diskSide, uint8_t* readBuffer, uint32_t bufferSize);

	int GetSideCapacity();

public:
	FdsLoader(bool useQdFormat = false);

	vector<uint8_t> RebuildFdsFile(vector<vector<uint8_t>> diskData, bool needHeader);
	void LoadDiskData(vector<uint8_t>& romFile, vector<vector<uint8_t>> &diskData, vector<vector<uint8_t>> &diskHeaders);
	void LoadRom(RomData& romData, vector<uint8_t>& romFile);
};
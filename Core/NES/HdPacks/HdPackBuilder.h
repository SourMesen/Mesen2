#pragma once
#include "pch.h"
#include "NES/HdPacks/HdData.h"
#include "NES/NesTypes.h"
#include "Shared/SettingTypes.h"
#include <map>

class Emulator;
class BaseMapper;

struct HdPackBuilderOptions
{
	char* SaveFolder;
	ScaleFilterType FilterType;
	uint32_t Scale;
	uint32_t ChrRamBankSize;

	bool UseLargeSprites;
	bool SortByUsageFrequency;
	bool GroupBlankTiles;
	bool IgnoreOverscan;
};

class HdPackBuilder
{
private:
	Emulator* _emu = nullptr;

	HdPackData _hdData;
	unordered_map<HdTileKey, uint32_t> _tileUsageCount;
	unordered_map<HdTileKey, HdPackTileInfo*> _tilesByKey;
	std::map<uint32_t, std::map<uint32_t, vector<HdPackTileInfo*>>> _tilesByChrBankByPalette;
	bool _isChrRam = false;
	string _saveFolder;
	string _romName;
	HdPackBuilderOptions _options = {};
	uint32_t _palette[512] = {};

	//Used to group blank tiles together
	uint32_t _blankTileIndex = 0;
	int _blankTilePalette = 0;

	void AddTile(HdPackTileInfo *tile, uint32_t usageCount);
	void GenerateHdTile(HdPackTileInfo *tile);
	void DrawTile(HdPackTileInfo *tile, int tileIndex, uint32_t* pngBuffer, int pageNumber, bool containsSpritesOnly);

public:
	HdPackBuilder(Emulator* emu, PpuModel ppuModel, bool isChrRam, HdPackBuilderOptions options);
	~HdPackBuilder();

	void ProcessTile(uint32_t x, uint32_t y, uint16_t tileAddr, HdPpuTileInfo& tile, BaseMapper* mapper, bool isSprite, uint32_t chrBankHash, bool transparencyRequired);
	void SaveHdPack();
	
	//static void GetChrBankList(uint32_t *banks);
	//static void GetBankPreview(uint32_t bankNumber, uint32_t pageNumber, uint32_t *rgbBuffer);
};
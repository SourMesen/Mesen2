#include "pch.h"
#include <algorithm>
#include "NES/HdPacks/HdPackBuilder.h"
#include "NES/HdPacks/HdNesPack.h"
#include "NES/BaseMapper.h"
#include "NES/BaseNesPpu.h"
#include "NES/NesConstants.h"
#include "NES/HdPacks/HdPackLoader.h"
#include "NES/NesDefaultVideoFilter.h"
#include "Utilities/xBRZ/xbrz.h"
#include "Utilities/HQX/hqx.h"
#include "Utilities/Scale2x/scalebit.h"
#include "Utilities/KreedSaiEagle/SaiEagle.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/PNGHelper.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/VirtualFile.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"

HdPackBuilder::HdPackBuilder(Emulator* emu, PpuModel ppuModel, bool isChrRam, HdPackBuilderOptions options)
{
	_emu = emu;
	_isChrRam = isChrRam;

	_saveFolder = options.SaveFolder;
	options.SaveFolder = nullptr;

	_options = options;

	NesDefaultVideoFilter::GetFullPalette(_palette, _emu->GetSettings()->GetNesConfig(), ppuModel);

	string existingPackDefinition = FolderUtilities::CombinePath(_saveFolder, "hires.txt");
	if(ifstream(existingPackDefinition)) {
		HdPackLoader::LoadHdNesPack(existingPackDefinition, _hdData);
		_hdData.LoadAsync();
		for(auto& tile : _hdData.Tiles) {
			tile->Init();
		}

		for(unique_ptr<HdPackTileInfo> &tile : _hdData.Tiles) {
			//Mark the tiles in the first PNGs as higher usage (preserves order when adding new tiles to an existing set)
			AddTile(tile.get(), 0xFFFFFFFF - tile->BitmapIndex);
		}

		if(_hdData.Scale != _options.Scale) {
			_options.FilterType = ScaleFilterType::Prescale;
		}
	} else {
		_hdData.Scale = _options.Scale;
	}

	_romName = FolderUtilities::GetFilename(_emu->GetRomInfo().RomFile.GetFileName(), false);
}

HdPackBuilder::~HdPackBuilder()
{
	SaveHdPack();
}

void HdPackBuilder::AddTile(HdPackTileInfo *tile, uint32_t usageCount)
{
	bool isTileBlank = _options.GroupBlankTiles ? tile->Blank : false;

	int chrBankId = isTileBlank ? 0xFFFFFFFF : tile->ChrBankId;
	int palette = isTileBlank ? _blankTilePalette : tile->PaletteColors;

	if(_tilesByChrBankByPalette.find(chrBankId) == _tilesByChrBankByPalette.end()) {
		_tilesByChrBankByPalette[chrBankId] = std::map<uint32_t, vector<HdPackTileInfo*>>();
	}

	std::map<uint32_t, vector<HdPackTileInfo*>> &paletteMap = _tilesByChrBankByPalette[chrBankId];
	if(paletteMap.find(palette) == paletteMap.end()) {
		paletteMap[palette] = vector<HdPackTileInfo*>(256, nullptr);
	}

	if(isTileBlank) {
		paletteMap[palette][_blankTileIndex] = tile;
		_blankTileIndex++;
		if(_blankTileIndex == _options.ChrRamBankSize / 16) {
			_blankTileIndex = 0;
			_blankTilePalette++;
		}
	} else {
		if(tile->TileIndex >= 0) {
			paletteMap[palette][tile->TileIndex % 256] = tile;
		} else {
			//FIXME: This will result in data loss if more than 256 tiles of the same palette exist in the hires.txt file
			//Currently this way to prevent issues when loading a CHR RAM HD pack into the recorder (because TileIndex is -1 in that case)
			for(int i = 0; i < 256; i++) {
				if(paletteMap[palette][i] == nullptr) {
					paletteMap[palette][i] = tile;
					break;
				}
			}
		}
	}

	_tilesByKey[tile->GetKey(false)] = tile;
	_tileUsageCount[tile->GetKey(false)] = usageCount;
}

void HdPackBuilder::ProcessTile(uint32_t x, uint32_t y, uint16_t tileAddr, HdPpuTileInfo &tile, BaseMapper *mapper, bool isSprite, uint32_t chrBankHash, bool transparencyRequired)
{
	if(_options.IgnoreOverscan) {
		OverscanDimensions overscan = _emu->GetSettings()->GetOverscan();
		if(x < overscan.Left || y < overscan.Top || (NesConstants::ScreenWidth - x - 1) < overscan.Right || (NesConstants::ScreenHeight - y - 1) < overscan.Bottom) {
			//Ignore tiles inside overscan
			return;
		}
	}

	auto result = _tileUsageCount.find(tile.GetKey(false));
	if(result == _tileUsageCount.end()) {
		//Check to see if a default tile matches
		result = _tileUsageCount.find(tile.GetKey(true));
	}

	if(result == _tileUsageCount.end()) {
		//First time seeing this tile/palette combination, store it
		HdPackTileInfo* hdTile = new HdPackTileInfo();
		hdTile->PaletteColors = tile.PaletteColors;
		hdTile->TileIndex = tile.TileIndex;
		hdTile->DefaultTile = false;
		hdTile->IsChrRamTile = _isChrRam;
		hdTile->Brightness = 255;
		hdTile->ChrBankId = _isChrRam ? chrBankHash : (tileAddr / 16 / 256);
		hdTile->TransparencyRequired = transparencyRequired;

		memcpy(hdTile->TileData, tile.TileData, 16);

		_hdData.Tiles.push_back(unique_ptr<HdPackTileInfo>(hdTile));
		AddTile(hdTile, 1);
	} else {
		if(transparencyRequired) {
			auto existingTile = _tilesByKey.find(tile.GetKey(false));
			if(existingTile != _tilesByKey.end()) {
				existingTile->second->TransparencyRequired = true;
			}
		}
		
		if(result->second < 0x7FFFFFFF) {
			//Increase usage count
			result->second++;
		}
	}
}

void HdPackBuilder::GenerateHdTile(HdPackTileInfo *tile)
{
	uint32_t hdScale = _hdData.Scale;

	vector<uint32_t> originalTile = tile->ToRgb(_palette);
	vector<uint32_t> hdTile(8 * 8 * hdScale*hdScale, 0);

	switch(_options.FilterType) {
		case ScaleFilterType::HQX: 
			hqx(hdScale, originalTile.data(), hdTile.data(), 8, 8);
			break;

		case ScaleFilterType::Prescale:
			hdTile.clear();
			for(uint8_t i = 0; i < 8 * hdScale; i++) {
				for(uint8_t j = 0; j < 8 * hdScale; j++) {
					hdTile.push_back(originalTile[i/hdScale*8+j/hdScale]);
				}
			}
			break;

		case ScaleFilterType::Scale2x:
			scale(hdScale, hdTile.data(), 8 * sizeof(uint32_t) * hdScale, originalTile.data(), 8 * sizeof(uint32_t), 4, 8, 8);
			break;

		case ScaleFilterType::_2xSai:
			twoxsai_generic_xrgb8888(8, 8, originalTile.data(), 8, hdTile.data(), 8 * hdScale);
			break;

		case ScaleFilterType::Super2xSai:
			supertwoxsai_generic_xrgb8888(8, 8, originalTile.data(), 8, hdTile.data(), 8 * hdScale);
			break;

		case ScaleFilterType::SuperEagle:
			supereagle_generic_xrgb8888(8, 8, originalTile.data(), 8, hdTile.data(), 8 * hdScale);
			break;

		case ScaleFilterType::xBRZ:
			xbrz::scale(hdScale, originalTile.data(), hdTile.data(), 8, 8, xbrz::ColorFormat::ARGB);
			break;
	}

	tile->HdTileData = hdTile;
}

void HdPackBuilder::DrawTile(HdPackTileInfo *tile, int tileNumber, uint32_t *pngBuffer, int pageNumber, bool containsSpritesOnly)
{
	if(tile->HdTileData.empty()) {
		GenerateHdTile(tile);
		tile->UpdateFlags();
	}

	if(containsSpritesOnly && _options.UseLargeSprites) {
		int row = tileNumber / 16;
		int column = tileNumber % 16;

		int newColumn = column / 2 + ((row & 1) ? 8 : 0);
		int newRow = (row & 0xFE) + ((column & 1) ? 1 : 0);

		tileNumber = newRow * 16 + newColumn;
	}

	tileNumber += pageNumber * (256 / (0x1000 / _options.ChrRamBankSize));

	int tileDimension = 8 * _hdData.Scale;
	int x = tileNumber % 16 * tileDimension;
	int y = tileNumber / 16 * tileDimension;

	tile->X = x;
	tile->Y = y;

	int pngWidth = 128 * _hdData.Scale;
	int pngPos = y * pngWidth + x;
	int tilePos = 0;
	for(uint8_t i = 0; i < tileDimension; i++) {
		for(uint8_t j = 0; j < tileDimension; j++) {
			pngBuffer[pngPos] = tile->HdTileData[tilePos++];
			pngPos++;
		}
		pngPos += pngWidth - tileDimension;
	}
}

void HdPackBuilder::SaveHdPack()
{
	FolderUtilities::CreateFolder(_saveFolder);

	stringstream pngRows;
	stringstream tileRows;
	stringstream ss;
	int pngIndex = 0;
	ss << "<ver>" << std::to_string(BaseHdNesPack::CurrentVersion) << std::endl;
	ss << "<scale>" << _hdData.Scale << std::endl;
	ss << "<supportedRom>" << _emu->GetRomInfo().RomFile.GetSha1Hash() << std::endl;
	if(_options.IgnoreOverscan) {
		OverscanDimensions overscan = _emu->GetSettings()->GetOverscan();
		ss << "<overscan>" << overscan.Top << "," << overscan.Right << "," << overscan.Bottom << "," << overscan.Left << std::endl;
	}

	int tileDimension = 8 * _hdData.Scale;
	int pngDimension = 16 * tileDimension;
	int pngBufferSize = pngDimension * pngDimension;
	uint32_t* pngBuffer = new uint32_t[pngBufferSize];

	int maxPageNumber = 0x1000 / _options.ChrRamBankSize;
	int pageNumber = 0;
	bool pngEmpty = true;
	int pngNumber = 0;

	for(int i = 0; i < pngBufferSize; i++) {
		pngBuffer[i] = 0xFFFF00FF;
	}

	auto savePng = [&tileRows, &pngRows, &ss, &pngBuffer, &pngDimension, &pngIndex, &pngBufferSize, &pngEmpty, &pngNumber, this](uint32_t chrBankId) {
		if(!pngEmpty) {
			string pngName;
			if(_isChrRam) {
				pngName = "Chr_" + std::to_string(pngNumber) + ".png";
			} else {
				pngName = "Chr_" + HexUtilities::ToHex(chrBankId) + "_" + std::to_string(pngNumber) + ".png";
			}

			tileRows << std::endl << "#" << pngName << std::endl;
			tileRows << pngRows.str();
			pngRows = stringstream();

			ss << "<img>" << pngName << std::endl;
			PNGHelper::WritePNG(FolderUtilities::CombinePath(_saveFolder, pngName), pngBuffer, pngDimension, pngDimension, 32);
			pngNumber++;
			pngIndex++;

			for(int i = 0; i < pngBufferSize; i++) {
				pngBuffer[i] = 0xFFFF00FF;
			}
			pngEmpty = true;
		}
	};

	for(std::pair<const uint32_t, std::map<uint32_t, vector<HdPackTileInfo*>>> &kvp : _tilesByChrBankByPalette) {
		if(_options.SortByUsageFrequency) {
			for(int i = 0; i < 256; i++) {
				vector<std::pair<uint32_t, HdPackTileInfo*>> tiles;
				for(std::pair<const uint32_t, vector<HdPackTileInfo*>> &paletteMap : kvp.second) {
					if(paletteMap.second[i]) {
						tiles.push_back({ _tileUsageCount[paletteMap.second[i]->GetKey(false)], paletteMap.second[i] });
					}
				}
				std::sort(tiles.begin(), tiles.end(), [=](std::pair<uint32_t, HdPackTileInfo*> &a, std::pair<uint32_t, HdPackTileInfo*> &b) {
					return a.first > b.first;
				});

				size_t j = 0;
				for(std::pair<const uint32_t, vector<HdPackTileInfo*>> &paletteMap : kvp.second) {
					if(j < tiles.size()) {
						paletteMap.second[i] = tiles[j].second;
						j++;
					} else {
						paletteMap.second[i] = nullptr;
					}
				}
			}
		}

		if(!_isChrRam) {
			pngNumber = 0;
		}

		for(std::pair<const uint32_t, vector<HdPackTileInfo*>> &tileKvp : kvp.second) {
			bool pageEmpty = true;
			bool spritesOnly = true;
			for(HdPackTileInfo* tileInfo : tileKvp.second) {
				if(tileInfo && !tileInfo->IsSpriteTile()) {
					spritesOnly = false;
				}
			}

			for(int i = 0; i < 256; i++) {
				HdPackTileInfo* tileInfo = tileKvp.second[i];
				if(tileInfo) {
					DrawTile(tileInfo, i, pngBuffer, pageNumber, spritesOnly);

					pngRows << tileInfo->ToString(pngIndex) << std::endl;

					pageEmpty = false;
					pngEmpty = false;
				}
			}

			if(!pageEmpty) {
				pageNumber++;

				if(pageNumber == maxPageNumber) {
					savePng(kvp.first);
					pageNumber = 0;
				}
			}
		}
	}
	savePng(-1);

	for(unique_ptr<HdPackCondition> &condition : _hdData.Conditions) {
		if(!condition->IsExcludedFromFile()) {
			ss << condition->ToString() << std::endl;
		}
	}

	for(int i = 0; i < HdPackData::BgLayerCount; i++) {
		for(HdBackgroundInfo& bgInfo : _hdData.BackgroundsByPriority[i]) {
			ss << bgInfo.ToString() << std::endl;
		}
	}

	for(auto &bgmInfo : _hdData.BgmFilesById) {
		ss << "<bgm>" << std::to_string(bgmInfo.first >> 8) << "," << std::to_string(bgmInfo.first & 0xFF) << "," << VirtualFile(bgmInfo.second.Filename).GetFileName();
		if(bgmInfo.second.LoopPosition > 0) {
			ss << "," << std::to_string(bgmInfo.second.LoopPosition);
		}
		ss << std::endl;
	}

	for(auto &sfxInfo : _hdData.SfxFilesById) {
		ss << "<sfx>" << std::to_string(sfxInfo.first >> 8) << "," << std::to_string(sfxInfo.first & 0xFF) << "," << VirtualFile(sfxInfo.second).GetFileName() << std::endl;
	}

	for(auto &patchInfo : _hdData.PatchesByHash) {
		ss << "<patch>" << VirtualFile(patchInfo.second).GetFileName() << "," << patchInfo.first << std::endl;
	}

	if(_hdData.OptionFlags != 0) {
		ss << "<options>";
		if(_hdData.OptionFlags & (int)HdPackOptions::NoSpriteLimit) {
			ss << "disableSpriteLimit,";
		}
		if(_hdData.OptionFlags & (int)HdPackOptions::AlternateRegisterRange) {
			ss << "alternateRegisterRange,";
		}
		if(_hdData.OptionFlags & (int)HdPackOptions::DisableCache) {
			ss << "disableCache,";
		}
		if(_hdData.OptionFlags & (int)HdPackOptions::DontRenderOriginalTiles) {
			ss << "disableOriginalTiles,";
		}
		if(_hdData.OptionFlags & (int)HdPackOptions::AutomaticFallbackTiles) {
			ss << "automaticFallbackTiles,";
		}
	}

	ss << tileRows.str();

	ofstream hiresFile(FolderUtilities::CombinePath(_saveFolder, "hires.txt"), ios::out);
	hiresFile << ss.str();
	hiresFile.close();

	delete[] pngBuffer;
}
/*
void HdPackBuilder::GetChrBankList(uint32_t *banks)
{
	ConsolePauseHelper helper(_instance->_console.get());
	for(std::pair<const uint32_t, std::map<uint32_t, vector<HdPackTileInfo*>>> &kvp : _instance->_tilesByChrBankByPalette) {
		*banks = kvp.first;
		banks++;
	}
	*banks = -1;
}

void HdPackBuilder::GetBankPreview(uint32_t bankNumber, uint32_t pageNumber, uint32_t *rgbBuffer)
{
	ConsolePauseHelper helper(_instance->_console.get());

	for(uint32_t i = 0; i < 128 * 128 * _instance->_hdData.Scale*_instance->_hdData.Scale; i++) {
		rgbBuffer[i] = 0xFF666666;
	}

	auto result = _instance->_tilesByChrBankByPalette.find(bankNumber);
	if(result != _instance->_tilesByChrBankByPalette.end()) {
		std::map<uint32_t, vector<HdPackTileInfo*>> bankData = result->second;

		if(_instance->_flags & HdPackRecordFlags::SortByUsageFrequency) {
			for(int i = 0; i < 256; i++) {
				vector<std::pair<uint32_t, HdPackTileInfo*>> tiles;
				for(std::pair<const uint32_t, vector<HdPackTileInfo*>> &pageData : bankData) {
					if(pageData.second[i]) {
						tiles.push_back({ _instance->_tileUsageCount[pageData.second[i]->GetKey(false)], pageData.second[i] });
					}
				}

				std::sort(tiles.begin(), tiles.end(), [=](std::pair<uint32_t, HdPackTileInfo*> &a, std::pair<uint32_t, HdPackTileInfo*> &b) {
					return a.first > b.first;
				});

				size_t j = 0;
				for(std::pair<const uint32_t, vector<HdPackTileInfo*>> &pageData : bankData) {
					if(j < tiles.size()) {
						pageData.second[i] = tiles[j].second;
						j++;
					} else {
						pageData.second[i] = nullptr;
					}
				}
			}
		}

		bool spritesOnly = true;
		for(HdPackTileInfo* tileInfo : (*bankData.begin()).second) {
			if(tileInfo && !tileInfo->IsSpriteTile()) {
				spritesOnly = false;
			}
		}

		for(int i = 0; i < 256; i++) {
			HdPackTileInfo* tileInfo = (*bankData.begin()).second[i];
			if(tileInfo) {
				_instance->DrawTile(tileInfo, i, (uint32_t*)rgbBuffer, 0, spritesOnly);
			}
		}
	}
}
*/
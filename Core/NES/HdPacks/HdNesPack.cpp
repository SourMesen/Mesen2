#include "pch.h"
#include <algorithm>
#include <unordered_map>
#include "NES/HdPacks/HdNesPack.h"
#include "NES/HdPacks/HdPackLoader.h"
#include "NES/NesConsole.h"
#include "NES/BaseMapper.h"
#include "NES/NesDefaultVideoFilter.h"
#include "Shared/MessageManager.h"
#include "Shared/EmuSettings.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/PNGHelper.h"

template<uint32_t scale>
HdNesPack<scale>::HdNesPack(NesConsole* console, EmuSettings* settings, HdPackData* hdData)
{
	_console = console;
	_settings = settings;
	_hdData = hdData;

	InitializeFallbackTiles();
	CleanupInvalidRules();
}

template<uint32_t scale>
HdNesPack<scale>::~HdNesPack()
{
}

template<uint32_t scale>
void HdNesPack<scale>::CleanupInvalidRules()
{
	//Cleanup invalid <addition> tags
	bool isChrRam = _console->GetMapper()->HasChrRam();
	if(_hdData->AdditionalSprites.size()) {
		vector<HdPackAdditionalSpriteInfo>& elems = _hdData->AdditionalSprites;
		elems.erase(std::remove_if(elems.begin(), elems.end(), [isChrRam](const HdPackAdditionalSpriteInfo& o) { return o.AdditionalTile.IsChrRamTile != isChrRam; }), elems.end());
	}
}

template<uint32_t scale>
void HdNesPack<scale>::InitializeFallbackTiles()
{
	BaseMapper* mapper = _console->GetMapper();
	uint8_t blankTile[16] = {};
	if(mapper->HasChrRom()) {
		if(_hdData->OptionFlags & (int)HdPackOptions::AutomaticFallbackTiles) {
			int32_t chrRomSize = (int32_t)mapper->GetChrRomSize();
			int32_t tileCount = chrRomSize / 16;

			HdTileKey tileKey = {};
			tileKey.IsChrRamTile = true;
			unordered_set<HdTileKey> usedTiles;
			for(auto& tile : _hdData->Tiles) {
				tileKey.TileIndex = tile->TileIndex;
				mapper->CopyChrTile(tile->TileIndex * 16, tileKey.TileData);

				usedTiles.emplace(tileKey);
			}

			for(int32_t i = 0; i < tileCount; i++) {
				mapper->CopyChrTile(i * 16, tileKey.TileData);
				if(memcmp(tileKey.TileData, blankTile, sizeof(blankTile)) == 0) {
					//Skip processing for blank/transparent tiles
					continue;
				}

				auto result = usedTiles.find(tileKey);
				if(result != usedTiles.end() && result->TileIndex != i) {
					_fallbackTiles[i] = result->TileIndex;
				}
			}
		}

		for(auto def : _hdData->FallbackTiles) {
			_fallbackTiles[def.TileIndex] = def.FallbackTileIndex;
		}
	}
}

template<uint32_t scale>
template<HdPackBlendMode blendMode>
void HdNesPack<scale>::BlendColors(uint8_t output[4], uint8_t input[4])
{
	if constexpr(blendMode == HdPackBlendMode::Alpha) {
		uint8_t invertedAlpha = 256 - input[3];
		output[0] = input[0] + (uint8_t)((invertedAlpha * output[0]) >> 8);
		output[1] = input[1] + (uint8_t)((invertedAlpha * output[1]) >> 8);
		output[2] = input[2] + (uint8_t)((invertedAlpha * output[2]) >> 8);
		output[3] = 0xFF;
	} else if constexpr(blendMode == HdPackBlendMode::Add) {
		output[0] = (uint8_t)std::min(255, (int)input[0] + (int)output[0]);
		output[1] = (uint8_t)std::min(255, (int)input[1] + (int)output[1]);
		output[2] = (uint8_t)std::min(255, (int)input[2] + (int)output[2]);
		output[3] = 0xFF;
	} else if constexpr(blendMode == HdPackBlendMode::Subtract) {
		output[0] = (uint8_t)std::max(0, (int)output[0] - (int)input[0]);
		output[1] = (uint8_t)std::max(0, (int)output[1] - (int)input[1]);
		output[2] = (uint8_t)std::max(0, (int)output[2] - (int)input[2]);
		output[3] = 0xFF;
	}
}

template<uint32_t scale>
uint32_t HdNesPack<scale>::AdjustBrightness(uint8_t input[4], int brightness)
{
	return (
		std::min(255, (brightness * ((int)input[0] + 1)) >> 8) |
		(std::min(255, (brightness * ((int)input[1] + 1)) >> 8) << 8) |
		(std::min(255, (brightness * ((int)input[2] + 1)) >> 8) << 16) |
		(input[3] << 24)
	);
}

template<uint32_t scale>
void HdNesPack<scale>::DrawColor(uint32_t color, uint32_t *outputBuffer, uint32_t screenWidth)
{
	if constexpr(scale == 1) {
		*outputBuffer = color;
	} else if constexpr(scale == 2) {
		outputBuffer[0] = color;
		outputBuffer[1] = color;
		outputBuffer[screenWidth] = color;
		outputBuffer[screenWidth+1] = color;
	} else {
		for(uint32_t i = 0; i < scale; i++) {
			std::fill(outputBuffer, outputBuffer + scale, color);
			outputBuffer += screenWidth;
		}
	}
}

template<uint32_t scale>
template<HdPackBlendMode blendMode>
void HdNesPack<scale>::DrawCustomBackground(HdBackgroundInfo& bgInfo, uint32_t *outputBuffer, uint32_t x, uint32_t y, uint32_t screenWidth)
{
	uint32_t width = bgInfo.Data->Width;
	uint32_t *pngData = bgInfo.data() + ((bgInfo.Top + y) * scale * width) + ((bgInfo.Left + x) * scale);

	if(bgInfo.Brightness == 255) {
		for(uint32_t i = 0; i < scale; i++) {
			for(uint32_t j = 0; j < scale; j++) {
				uint32_t pixelColor = pngData[j];
				if constexpr(blendMode == HdPackBlendMode::Alpha) {
					if(pixelColor >= 0xFF000000) {
						outputBuffer[j] = pixelColor;
					} else if(pixelColor >= 0x01000000) {
						BlendColors<blendMode>((uint8_t*)(outputBuffer + j), (uint8_t*)(&pixelColor));
					}
				} else {
					BlendColors<blendMode>((uint8_t*)(outputBuffer + j), (uint8_t*)(&pixelColor));
				}
			}
			outputBuffer += screenWidth;
			pngData += width;
		}
	} else {
		for(uint32_t i = 0; i < scale; i++) {
			for(uint32_t j = 0; j < scale; j++) {
				uint32_t pixelColor = AdjustBrightness((uint8_t*)(pngData + j), bgInfo.Brightness);
				if constexpr(blendMode == HdPackBlendMode::Alpha) {
					if(pixelColor >= 0xFF000000) {
						outputBuffer[j] = pixelColor;
					} else if(pixelColor >= 0x01000000) {
						BlendColors<blendMode>((uint8_t*)(outputBuffer + j), (uint8_t*)(&pixelColor));
					}
				} else {
					BlendColors<blendMode>((uint8_t*)(outputBuffer + j), (uint8_t*)(&pixelColor));
				}
			}
			outputBuffer += screenWidth;
			pngData += width;
		}
	}
}

template<uint32_t scale>
void HdNesPack<scale>::DrawTile(HdPpuTileInfo &tileInfo, HdPackTileInfo &hdPackTileInfo, uint32_t *outputBuffer, uint32_t screenWidth)
{
	if(hdPackTileInfo.IsFullyTransparent) {
		return;
	}

	uint32_t *bitmapData = hdPackTileInfo.HdTileData.data();
	uint32_t tileWidth = 8 * scale;
	uint8_t tileOffsetX = tileInfo.HorizontalMirroring ? 7 - tileInfo.OffsetX : tileInfo.OffsetX;
	uint32_t bitmapOffset = (tileInfo.OffsetY * scale) * tileWidth + tileOffsetX * scale;
	int32_t bitmapSmallInc = 1;
	int32_t bitmapLargeInc = tileWidth - scale;
	if(tileInfo.HorizontalMirroring) {
		bitmapOffset += scale - 1;
		bitmapSmallInc = -1;
		bitmapLargeInc = tileWidth + scale;
	}
	if(tileInfo.VerticalMirroring) {
		bitmapOffset += tileWidth * (scale - 1);
		bitmapLargeInc = (tileInfo.HorizontalMirroring ? (int32_t)scale : -(int32_t)scale) - (int32_t)tileWidth;
	}

	uint32_t rgbValue;
	if(hdPackTileInfo.HasTransparentPixels || hdPackTileInfo.Brightness != 255) {
		for(uint32_t y = 0; y < scale; y++) {
			for(uint32_t x = 0; x < scale; x++) {
				if(hdPackTileInfo.Brightness == 255) {
					rgbValue = *(bitmapData + bitmapOffset);
				} else {
					rgbValue = AdjustBrightness((uint8_t*)(bitmapData + bitmapOffset), hdPackTileInfo.Brightness);
				}

				if(!hdPackTileInfo.HasTransparentPixels || (bitmapData[bitmapOffset] & 0xFF000000) == 0xFF000000) {
					*outputBuffer = rgbValue;
				} else {
					if(bitmapData[bitmapOffset] & 0xFF000000) {
						BlendColors<HdPackBlendMode::Alpha>((uint8_t*)outputBuffer, (uint8_t*)&rgbValue);
					}
				}
				outputBuffer++;
				bitmapOffset += bitmapSmallInc;
			}
			bitmapOffset += bitmapLargeInc;
			outputBuffer += screenWidth - scale;
		}
	} else {
		for(uint32_t y = 0; y < scale; y++) {
			for(uint32_t x = 0; x < scale; x++) {
				*outputBuffer = *(bitmapData + bitmapOffset);
				outputBuffer++;
				bitmapOffset += bitmapSmallInc;
			}
			bitmapOffset += bitmapLargeInc;
			outputBuffer += screenWidth - scale;
		}
	}
}

template<uint32_t scale>
void HdNesPack<scale>::OnLineStart(HdPpuPixelInfo &lineFirstPixel, uint8_t y)
{
	_scrollX = ((lineFirstPixel.TmpVideoRamAddr & 0x1F) << 3) | lineFirstPixel.XScroll | ((lineFirstPixel.TmpVideoRamAddr & 0x400) ? 0x100 : 0);
	_useCachedTile = false;

	int32_t scrollY = (((lineFirstPixel.TmpVideoRamAddr & 0x3E0) >> 2) | ((lineFirstPixel.TmpVideoRamAddr & 0x7000) >> 12)) + ((lineFirstPixel.TmpVideoRamAddr & 0x800) ? 240 : 0);
	
	for(int layer = 0; layer < 4; layer++) {
		for(int i = 0; i < _activeBgCount[layer]; i++) {
			HdBgConfig& cfg = _bgConfig[layer * HdNesPack::PriorityLevelsPerLayer + i];
			if(cfg.BackgroundIndex < 0) {
				continue;
			}

			HdBackgroundInfo& bgInfo = _hdData->BackgroundsByPriority[cfg.BgPriority][cfg.BackgroundIndex];
			bgInfo.Data->Init();

			cfg.BgScrollX = (int32_t)(_scrollX * bgInfo.HorizontalScrollRatio);
			cfg.BgScrollY = (int32_t)(scrollY * bgInfo.VerticalScrollRatio);
			if(y >= -cfg.BgScrollY && (y + bgInfo.Top + cfg.BgScrollY + 1) * scale <= bgInfo.Data->Height) {
				cfg.BgMinX = -cfg.BgScrollX;
				cfg.BgMaxX = bgInfo.Data->Width / scale - bgInfo.Left - cfg.BgScrollX - 1;
			} else {
				cfg.BgMinX = -1;
				cfg.BgMaxX = -1;
			}
		}
	}
}

template<uint32_t scale>
int32_t HdNesPack<scale>::GetLayerIndex(uint8_t priority)
{
	for(size_t i = 0; i < _hdData->BackgroundsByPriority[priority].size(); i++) {
		bool isMatch = true;
		for(HdPackCondition* condition : _hdData->BackgroundsByPriority[priority][i].Conditions) {
			if(!condition->CheckCondition(0, 0, nullptr)) {
				isMatch = false;
				break;
			}
		}

		if(isMatch) {
			return (int32_t)i;
		}
	}
	return -1;
}

template<uint32_t scale>
void HdNesPack<scale>::OnBeforeApplyFilter()
{
	for(unique_ptr<HdPackCondition>& condition : _hdData->Conditions) {
		condition->Initialize(_hdScreenInfo, this);
	}

	if(_hdData->Palette.size() == 0x40) {
		memcpy(_palette, _hdData->Palette.data(), 0x40 * sizeof(uint32_t));
	} else {
		NesDefaultVideoFilter::GetFullPalette(_palette, _settings->GetNesConfig(), PpuModel::Ppu2C02);
	}
	_cacheEnabled = (_hdData->OptionFlags & (int)HdPackOptions::DisableCache) == 0;

	for(int layer = 0; layer < 4; layer++) {
		uint32_t activeCount = 0;
		for(int i = 0; i < HdNesPack::PriorityLevelsPerLayer; i++) {
			int32_t index = GetLayerIndex(layer * HdNesPack::PriorityLevelsPerLayer + i);
			if(index >= 0) {
				_bgConfig[layer*10+activeCount].BgPriority = layer * HdNesPack::PriorityLevelsPerLayer + i;
				_bgConfig[layer*10+activeCount].BackgroundIndex = index;
				activeCount++;
			}
		}
		_activeBgCount[layer] = activeCount;
	}

	ProcessAdditionalSprites();
}

template<uint32_t scale>
void HdNesPack<scale>::ProcessAdditionalSprites()
{
	if(_hdData->AdditionalSprites.empty()) {
		return;
	}

	bool checkFallbackTiles = _console->GetMapper()->HasChrRom() && _fallbackTiles.size() > 0;
	HdPpuPixelInfo& lineFirstPixel = _hdScreenInfo->ScreenTiles[0];
	uint32_t yScroll = (((lineFirstPixel.TmpVideoRamAddr & 0x3E0) >> 2) | ((lineFirstPixel.TmpVideoRamAddr & 0x7000) >> 12)) + ((lineFirstPixel.TmpVideoRamAddr & 0x800) ? 240 : 0);
	uint16_t tmpVramAddr = lineFirstPixel.TmpVideoRamAddr;
	bool processBgNextRow = true;

	for(int32_t y = 0; y < NesConstants::ScreenHeight; y++) {
		lineFirstPixel = _hdScreenInfo->ScreenTiles[y << 8];

		//Only process the first scanline for each row of tiles (if no additions are found)
		if(((yScroll + y) & 0x07) == 0 || tmpVramAddr != lineFirstPixel.TmpVideoRamAddr) {
			yScroll = (((lineFirstPixel.TmpVideoRamAddr & 0x3E0) >> 2) | ((lineFirstPixel.TmpVideoRamAddr & 0x7000) >> 12)) + ((lineFirstPixel.TmpVideoRamAddr & 0x800) ? 240 : 0);
			tmpVramAddr = lineFirstPixel.TmpVideoRamAddr;
			processBgNextRow = true;
		}

		if(processBgNextRow) {
			processBgNextRow = false;
			for(int32_t x = 0; x < NesConstants::ScreenWidth; x++) {
				HdPpuPixelInfo& pixelInfo = _hdScreenInfo->ScreenTiles[(y << 8) | x];
				for(uint8_t i = 0; i < pixelInfo.SpriteCount; i++) {
					DrawAdditionalTiles(x, y, pixelInfo.Sprite[i], checkFallbackTiles);
				}

				processBgNextRow |= DrawAdditionalTiles(x, y, pixelInfo.Tile, checkFallbackTiles);
			}
		} else {
			for(int32_t x = 0; x < NesConstants::ScreenWidth; x++) {
				HdPpuPixelInfo& pixelInfo = _hdScreenInfo->ScreenTiles[(y << 8) | x];
				for(uint8_t i = 0; i < pixelInfo.SpriteCount; i++) {
					DrawAdditionalTiles(x, y, pixelInfo.Sprite[i], checkFallbackTiles);
				}
			}
		}
	}
}

template<uint32_t scale>
bool HdNesPack<scale>::DrawAdditionalTiles(int32_t x, int32_t y, HdPpuTileInfo& tile, bool checkFallbackTiles)
{
	auto result = _additionalTilesByKey.find(tile);
	if(result == _additionalTilesByKey.end()) {
		BuildAdditionalTileCache(x, y, tile, checkFallbackTiles);
		result = _additionalTilesByKey.find(tile);
	}

	for(auto& additionalSprite : result->second) {
		InsertAdditionalSprite(x, y, tile, additionalSprite);
	}
	return result->second.size() > 0;
}

template<uint32_t scale>
void HdNesPack<scale>::BuildAdditionalTileCache(int32_t x, int32_t y, HdPpuTileInfo& tile, bool checkFallbackTiles)
{
	vector<HdPackAdditionalSpriteInfo> additions;
	for(HdPackAdditionalSpriteInfo& additionalSprite : _hdData->AdditionalSprites) {
		if(!additionalSprite.IgnorePalette && tile == additionalSprite.OriginalTile) {
			additions.push_back(additionalSprite);
		} else if(additionalSprite.IgnorePalette && tile.GetKey(true) == additionalSprite.OriginalTile.GetKey(true)) {
			additions.push_back(additionalSprite);
		} else if(checkFallbackTiles) {
			if(additionalSprite.IgnorePalette || tile.PaletteColors == additionalSprite.OriginalTile.PaletteColors) {
				if(GetFallbackTile(tile.TileIndex) == additionalSprite.OriginalTile.TileIndex) {
					additions.push_back(additionalSprite);
				}
			}
		}
	}

	//Cache list of additional tiles linked to this sprite
	_additionalTilesByKey.emplace(tile, additions);
}

template<uint32_t scale>
void HdNesPack<scale>::InsertAdditionalSprite(int32_t sourceX, int32_t sourceY, HdPpuTileInfo& sprite, HdPackAdditionalSpriteInfo& additionalSprite)
{
	int32_t targetX = sourceX + (sprite.HorizontalMirroring ? -additionalSprite.OffsetX : additionalSprite.OffsetX);
	int32_t targetY = sourceY + (sprite.VerticalMirroring ? -additionalSprite.OffsetY : additionalSprite.OffsetY);

	if(targetX >= 0 && targetX < NesConstants::ScreenWidth && targetY >= 0 && targetY < NesConstants::ScreenHeight) {
		HdPpuPixelInfo& pixelInfo = _hdScreenInfo->ScreenTiles[(targetY << 8) | targetX];
		if(pixelInfo.SpriteCount < 4) {
			HdPpuTileInfo& newSprite = pixelInfo.Sprite[pixelInfo.SpriteCount];
			memcpy(&newSprite, &sprite, sizeof(HdPpuTileInfo));
			newSprite.SpriteColorIndex = 0;
			newSprite.TileIndex = additionalSprite.AdditionalTile.TileIndex;
			newSprite.PaletteColors = additionalSprite.AdditionalTile.PaletteColors;
			memcpy(newSprite.TileData, additionalSprite.AdditionalTile.TileData, sizeof(newSprite.TileData));
			pixelInfo.SpriteCount++;
		}
	}
}

template<uint32_t scale>
HdPackTileInfo* HdNesPack<scale>::GetCachedMatchingTile(uint32_t x, uint32_t y, HdPpuTileInfo* tile)
{
	if(((_scrollX + x) & 0x07) == 0) {
		_useCachedTile = false;
	}

	bool disableCache = false;
	HdPackTileInfo* hdPackTileInfo;
	if(_useCachedTile) {
		hdPackTileInfo = _cachedTile;
	} else {
		hdPackTileInfo = GetMatchingTile(x, y, tile, &disableCache);

		if(!disableCache && _cacheEnabled) {
			//Use this tile for the next 8 horizontal pixels
			//Disable cache if a sprite condition is used, because sprites are not on a 8x8 grid
			_cachedTile = hdPackTileInfo;
			_useCachedTile = true;
		}
	}
	return hdPackTileInfo;
}

template<uint32_t scale>
HdPackTileInfo* HdNesPack<scale>::GetMatchingTile(uint32_t x, uint32_t y, HdPpuTileInfo* tile, bool* disableCache)
{
	auto hdTile = _hdData->TileByKey.find(*tile);
	if(hdTile == _hdData->TileByKey.end()) {
		int32_t fallbackTileIndex = GetFallbackTile(tile->TileIndex);
		if(fallbackTileIndex >= 0) {
			int32_t orgIndex = tile->TileIndex;
			tile->TileIndex = fallbackTileIndex;
			hdTile = _hdData->TileByKey.find(*tile);
			if(hdTile == _hdData->TileByKey.end()) {
				hdTile = _hdData->TileByKey.find(tile->GetKey(true));
				if(hdTile == _hdData->TileByKey.end()) {
					tile->TileIndex = orgIndex;
				}
			}
		}
	
		if(hdTile == _hdData->TileByKey.end()) {
			hdTile = _hdData->TileByKey.find(tile->GetKey(true));
		}
	}

	if(hdTile != _hdData->TileByKey.end()) {
		for(HdPackTileInfo* hdPackTile : hdTile->second) {
			if(disableCache != nullptr && hdPackTile->ForceDisableCache) {
				*disableCache = true;
			}

			if(hdPackTile->MatchesCondition(x, y, tile)) {
				if(hdPackTile->NeedInit()) {
					hdPackTile->Init();
				}
				return hdPackTile;
			}
		}
	}

	return nullptr;
}

template<uint32_t scale>
void HdNesPack<scale>::DrawBackgroundLayer(uint8_t priority, uint32_t x, uint32_t y, uint32_t* outputBuffer, uint32_t screenWidth)
{
	HdBgConfig bgConfig = _bgConfig[(int)priority];
	if((int32_t)x >= bgConfig.BgMinX && (int32_t)x <= bgConfig.BgMaxX) {
		HdBackgroundInfo& bgInfo = _hdData->BackgroundsByPriority[bgConfig.BgPriority][bgConfig.BackgroundIndex];
		switch(bgInfo.BlendMode) {
			case HdPackBlendMode::Alpha: DrawCustomBackground<HdPackBlendMode::Alpha>(bgInfo, outputBuffer, x + bgConfig.BgScrollX, y + bgConfig.BgScrollY, screenWidth); break;
			case HdPackBlendMode::Add: DrawCustomBackground<HdPackBlendMode::Add>(bgInfo, outputBuffer, x + bgConfig.BgScrollX, y + bgConfig.BgScrollY, screenWidth); break;
			case HdPackBlendMode::Subtract: DrawCustomBackground<HdPackBlendMode::Subtract>(bgInfo, outputBuffer, x + bgConfig.BgScrollX, y + bgConfig.BgScrollY, screenWidth); break;
		}
	}
}

template<uint32_t scale>
void HdNesPack<scale>::GetPixels(uint32_t x, uint32_t y, HdPpuPixelInfo &pixelInfo, uint32_t *outputBuffer, uint32_t screenWidth)
{
	HdPackTileInfo *hdPackTileInfo = nullptr;
	HdPackTileInfo *hdPackSpriteInfo = nullptr;

	bool hasSprite = pixelInfo.SpriteCount > 0;
	bool renderOriginalTiles = ((_hdData->OptionFlags & (int)HdPackOptions::DontRenderOriginalTiles) == 0);
	if(pixelInfo.Tile.TileIndex != HdPpuTileInfo::NoTile) {
		hdPackTileInfo = GetCachedMatchingTile(x, y, &pixelInfo.Tile);
	}

	int lowestBgSprite = 999;
	
	DrawColor(_palette[pixelInfo.Tile.PpuBackgroundColor], outputBuffer, screenWidth);

	for(int i = 0; i < _activeBgCount[0]; i++) {
		DrawBackgroundLayer(HdNesPack::BehindBgSpritesPriority+i, x, y, outputBuffer, screenWidth);
	}

	if(hasSprite) {
		for(int k = pixelInfo.SpriteCount - 1; k >= 0; k--) {
			if(pixelInfo.Sprite[k].BackgroundPriority) {
				if(pixelInfo.Sprite[k].SpriteColorIndex != 0) {
					lowestBgSprite = k;
				}

				hdPackSpriteInfo = GetMatchingTile(x, y, &pixelInfo.Sprite[k]);
				if(hdPackSpriteInfo) {
					DrawTile(pixelInfo.Sprite[k], *hdPackSpriteInfo, outputBuffer, screenWidth);
				} else if(pixelInfo.Sprite[k].SpriteColorIndex != 0) {
					DrawColor(_palette[pixelInfo.Sprite[k].SpriteColor], outputBuffer, screenWidth);
				}
			}
		}
	}
	
	for(int i = 0; i < _activeBgCount[1]; i++) {
		DrawBackgroundLayer(HdNesPack::BehindBgPriority+i, x, y, outputBuffer, screenWidth);
	}
	
	if(hdPackTileInfo) {
		DrawTile(pixelInfo.Tile, *hdPackTileInfo, outputBuffer, screenWidth);
	} else if(renderOriginalTiles) {
		//Draw regular SD background tile
		if(pixelInfo.Tile.BgColorIndex != 0) {
			DrawColor(_palette[pixelInfo.Tile.BgColor], outputBuffer, screenWidth);
		}
	}

	for(int i = 0; i < _activeBgCount[2]; i++) {
		DrawBackgroundLayer(HdNesPack::BehindFgSpritesPriority+i, x, y, outputBuffer, screenWidth);
	}

	if(hasSprite) {
		for(int k = pixelInfo.SpriteCount - 1; k >= 0; k--) {
			if(!pixelInfo.Sprite[k].BackgroundPriority && lowestBgSprite > k) {
				hdPackSpriteInfo = GetMatchingTile(x, y, &pixelInfo.Sprite[k]);
				if(hdPackSpriteInfo) {
					DrawTile(pixelInfo.Sprite[k], *hdPackSpriteInfo, outputBuffer, screenWidth);
				} else if(pixelInfo.Sprite[k].SpriteColorIndex != 0) {
					DrawColor(_palette[pixelInfo.Sprite[k].SpriteColor], outputBuffer, screenWidth);
				}
			}
		}
	}

	for(int i = 0; i < _activeBgCount[3]; i++) {
		DrawBackgroundLayer(HdNesPack::ForegroundPriority+i, x, y, outputBuffer, screenWidth);
	}
}

template<uint32_t scale>
void HdNesPack<scale>::Process(HdScreenInfo *hdScreenInfo, uint32_t* outputBuffer, OverscanDimensions &overscan)
{
	_hdScreenInfo = hdScreenInfo;
	uint32_t hdScale = GetScale();
	uint32_t screenWidth = (NesConstants::ScreenWidth - overscan.Left - overscan.Right) * hdScale;

	OnBeforeApplyFilter();
	for(uint32_t i = overscan.Top, iMax = 240 - overscan.Bottom; i < iMax; i++) {
		OnLineStart(hdScreenInfo->ScreenTiles[i << 8], i);
		uint32_t bufferIndex = (i - overscan.Top) * screenWidth * hdScale;
		uint32_t lineStartIndex = bufferIndex;
		for(uint32_t j = overscan.Left, jMax = 256 - overscan.Right; j < jMax; j++) {
			GetPixels(j, i, hdScreenInfo->ScreenTiles[i * 256 + j], outputBuffer + bufferIndex, screenWidth);
			bufferIndex += hdScale;
		}

		ProcessGrayscaleAndEmphasis(hdScreenInfo->ScreenTiles[i * 256], outputBuffer + lineStartIndex, screenWidth);
	}
}

template<uint32_t scale>
void HdNesPack<scale>::ProcessGrayscaleAndEmphasis(HdPpuPixelInfo &pixelInfo, uint32_t* outputBuffer, uint32_t hdScreenWidth)
{
	//Apply grayscale/emphasis bits on a scanline level (less accurate, but shouldn't cause issues and simpler to implement)
	if(pixelInfo.Grayscale) {
		uint32_t* out = outputBuffer;
		for(uint32_t y = 0; y < scale; y++) {
			for(uint32_t x = 0; x < hdScreenWidth; x++) {
				uint32_t &rgbValue = out[x];
				uint8_t average = (((rgbValue >> 16) & 0xFF) + ((rgbValue >> 8) & 0xFF) + (rgbValue & 0xFF)) / 3;
				rgbValue = (rgbValue & 0xFF000000) | (average << 16) | (average << 8) | average;
			}
			out += hdScreenWidth;
		}
	}

	if(pixelInfo.EmphasisBits) {
		uint8_t emphasisBits = pixelInfo.EmphasisBits;
		double red = 1.0, green = 1.0, blue = 1.0;
		if(emphasisBits & 0x01) {
			//Intensify red
			red *= 1.1;
			green *= 0.9;
			blue *= 0.9;
		}
		if(emphasisBits & 0x02) {
			//Intensify green
			green *= 1.1;
			red *= 0.9;
			blue *= 0.9;
		}
		if(emphasisBits & 0x04) {
			//Intensify blue
			blue *= 1.1;
			red *= 0.9;
			green *= 0.9;
		}

		uint32_t* out = outputBuffer;
		for(uint32_t y = 0; y < scale; y++) {
			for(uint32_t x = 0; x < hdScreenWidth; x++) {
				uint32_t &rgbValue = out[x];

				rgbValue = 0xFF000000 |
					(std::min<uint16_t>((uint16_t)(((rgbValue >> 16) & 0xFF) * red), 255) << 16) |
					(std::min<uint16_t>((uint16_t)(((rgbValue >> 8) & 0xFF) * green), 255) << 8) |
					std::min<uint16_t>((uint16_t)((rgbValue & 0xFF) * blue), 255);
			}
			out += hdScreenWidth;
		}
	}
}

template HdNesPack<1>::HdNesPack(NesConsole* console, EmuSettings* settings, HdPackData* hdData);
template HdNesPack<2>::HdNesPack(NesConsole* console, EmuSettings* settings, HdPackData* hdData);
template HdNesPack<3>::HdNesPack(NesConsole* console, EmuSettings* settings, HdPackData* hdData);
template HdNesPack<4>::HdNesPack(NesConsole* console, EmuSettings* settings, HdPackData* hdData);
template HdNesPack<5>::HdNesPack(NesConsole* console, EmuSettings* settings, HdPackData* hdData);
template HdNesPack<6>::HdNesPack(NesConsole* console, EmuSettings* settings, HdPackData* hdData);
template HdNesPack<7>::HdNesPack(NesConsole* console, EmuSettings* settings, HdPackData* hdData);
template HdNesPack<8>::HdNesPack(NesConsole* console, EmuSettings* settings, HdPackData* hdData);
template HdNesPack<9>::HdNesPack(NesConsole* console, EmuSettings* settings, HdPackData* hdData);
template HdNesPack<10>::HdNesPack(NesConsole* console, EmuSettings* settings, HdPackData* hdData);

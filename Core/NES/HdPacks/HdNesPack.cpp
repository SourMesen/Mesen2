#include "pch.h"
#include <algorithm>
#include <unordered_map>
#include "NES/HdPacks/HdNesPack.h"
#include "NES/HdPacks/HdPackLoader.h"
#include "NES/NesConsole.h"
#include "NES/NesDefaultVideoFilter.h"
#include "Shared/MessageManager.h"
#include "Shared/EmuSettings.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/PNGHelper.h"

HdNesPack::HdNesPack(EmuSettings* settings, HdPackData* hdData)
{
	_settings = settings;
	_hdData = hdData;
}

HdNesPack::~HdNesPack()
{
}

void HdNesPack::BlendColors(uint8_t output[4], uint8_t input[4])
{
	uint8_t invertedAlpha = 256 - input[3];
	output[0] = input[0] + (uint8_t)((invertedAlpha * output[0]) >> 8);
	output[1] = input[1] + (uint8_t)((invertedAlpha * output[1]) >> 8);
	output[2] = input[2] + (uint8_t)((invertedAlpha * output[2]) >> 8);
	output[3] = 0xFF;
}

uint32_t HdNesPack::AdjustBrightness(uint8_t input[4], int brightness)
{
	return (
		std::min(255, (brightness * ((int)input[0] + 1)) >> 8) |
		(std::min(255, (brightness * ((int)input[1] + 1)) >> 8) << 8) |
		(std::min(255, (brightness * ((int)input[2] + 1)) >> 8) << 16) |
		(input[3] << 24)
	);
}

void HdNesPack::DrawColor(uint32_t color, uint32_t *outputBuffer, uint32_t scale, uint32_t screenWidth)
{
	if(scale == 1) {
		*outputBuffer = color;
	} else {
		for(uint32_t i = 0; i < scale; i++) {
			std::fill(outputBuffer, outputBuffer + scale, color);
			outputBuffer += screenWidth;
		}
	}
}

void HdNesPack::DrawCustomBackground(HdBackgroundInfo& bgInfo, uint32_t *outputBuffer, uint32_t x, uint32_t y, uint32_t scale, uint32_t screenWidth)
{
	int brightness = bgInfo.Brightness;
	uint32_t left = bgInfo.Left;
	uint32_t top = bgInfo.Top;
	uint32_t width = bgInfo.Data->Width;
	uint32_t *pngData = bgInfo.data() + ((top + y) * _hdData->Scale * width) + ((left + x) * _hdData->Scale);
	uint32_t pixelColor;

	for(uint32_t i = 0; i < scale; i++) {
		for(uint32_t j = 0; j < scale; j++) {
			if(brightness == 255) {
				pixelColor = *pngData;
			} else {
				pixelColor = AdjustBrightness((uint8_t*)pngData, brightness);
			}

			if(((uint8_t*)pngData)[3] == 0xFF) {
				*outputBuffer = pixelColor;
			} else if(((uint8_t*)pngData)[3]) {
				BlendColors((uint8_t*)outputBuffer, (uint8_t*)(&pixelColor));
			}

			outputBuffer++;
			pngData++;
		}
		outputBuffer += screenWidth - scale;
		pngData += width - scale;
	}
}

void HdNesPack::DrawTile(HdPpuTileInfo &tileInfo, HdPackTileInfo &hdPackTileInfo, uint32_t *outputBuffer, uint32_t screenWidth)
{
	if(hdPackTileInfo.IsFullyTransparent) {
		return;
	}

	uint32_t scale = GetScale();
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
						BlendColors((uint8_t*)outputBuffer, (uint8_t*)&rgbValue);
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

uint32_t HdNesPack::GetScale()
{
	return _hdData->Scale;
}

void HdNesPack::OnLineStart(HdPpuPixelInfo &lineFirstPixel, uint8_t y)
{
	_scrollX = ((lineFirstPixel.TmpVideoRamAddr & 0x1F) << 3) | lineFirstPixel.XScroll | ((lineFirstPixel.TmpVideoRamAddr & 0x400) ? 0x100 : 0);
	_useCachedTile = false;

	int32_t scrollY = (((lineFirstPixel.TmpVideoRamAddr & 0x3E0) >> 2) | ((lineFirstPixel.TmpVideoRamAddr & 0x7000) >> 12)) + ((lineFirstPixel.TmpVideoRamAddr & 0x800) ? 240 : 0);
	
	for(int layer = 0; layer < 4; layer++) {
		for(int i = 0; i < _activeBgCount[layer]; i++) {
			HdBgConfig& cfg = _bgConfig[layer * HdNesPack::PriorityLevelsPerLayer + i];
			HdBackgroundInfo& bgInfo = _hdData->Backgrounds[cfg.BackgroundIndex];
			bgInfo.Data->Init();

			cfg.BgScrollX = (int32_t)(_scrollX * bgInfo.HorizontalScrollRatio);
			cfg.BgScrollY = (int32_t)(scrollY * bgInfo.VerticalScrollRatio);
			if(y >= -cfg.BgScrollY && (y + bgInfo.Top + cfg.BgScrollY + 1) * _hdData->Scale <= bgInfo.Data->Height) {
				cfg.BgMinX = -cfg.BgScrollX;
				cfg.BgMaxX = bgInfo.Data->Width / _hdData->Scale - bgInfo.Left - cfg.BgScrollX - 1;
			} else {
				cfg.BgMinX = -1;
				cfg.BgMaxX = -1;
			}
		}
	}
}

int32_t HdNesPack::GetLayerIndex(uint8_t priority)
{
	for(size_t i = 0; i < _hdData->Backgrounds.size(); i++) {
		if(_hdData->Backgrounds[i].Priority != priority) {
			continue;
		}

		bool isMatch = true;
		for(HdPackCondition* condition : _hdData->Backgrounds[i].Conditions) {
			if(!condition->CheckCondition(_hdScreenInfo, 0, 0, nullptr)) {
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

void HdNesPack::OnBeforeApplyFilter()
{
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
				_bgConfig[layer*10+activeCount].BackgroundIndex = index;
				activeCount++;
			}
		}
		_activeBgCount[layer] = activeCount;
	}

	for(unique_ptr<HdPackCondition> &condition : _hdData->Conditions) {
		condition->ClearCache();
	}
}

HdPackTileInfo* HdNesPack::GetCachedMatchingTile(uint32_t x, uint32_t y, HdPpuTileInfo* tile)
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

HdPackTileInfo* HdNesPack::GetMatchingTile(uint32_t x, uint32_t y, HdPpuTileInfo* tile, bool* disableCache)
{
	auto hdTile = _hdData->TileByKey.find(*tile);
	if(hdTile == _hdData->TileByKey.end()) {
		hdTile = _hdData->TileByKey.find(tile->GetKey(true));
	}

	if(hdTile != _hdData->TileByKey.end()) {
		for(HdPackTileInfo* hdPackTile : hdTile->second) {
			if(disableCache != nullptr && hdPackTile->ForceDisableCache) {
				*disableCache = true;
			}

			if(hdPackTile->MatchesCondition(_hdScreenInfo, x, y, tile)) {
				return hdPackTile;
			}
		}
	}

	return nullptr;
}

bool HdNesPack::DrawBackgroundLayer(uint8_t priority, uint32_t x, uint32_t y, uint32_t* outputBuffer, uint32_t screenWidth)
{
	HdBgConfig bgConfig = _bgConfig[(int)priority];
	if((int32_t)x >= bgConfig.BgMinX && (int32_t)x <= bgConfig.BgMaxX) {
		HdBackgroundInfo& bgInfo = _hdData->Backgrounds[bgConfig.BackgroundIndex];
		bgInfo.Data->Init();
		DrawCustomBackground(bgInfo, outputBuffer, x + bgConfig.BgScrollX, y + bgConfig.BgScrollY, _hdData->Scale, screenWidth);
		return true;
	}
	return false;
}

void HdNesPack::GetPixels(uint32_t x, uint32_t y, HdPpuPixelInfo &pixelInfo, uint32_t *outputBuffer, uint32_t screenWidth)
{
	HdPackTileInfo *hdPackTileInfo = nullptr;
	HdPackTileInfo *hdPackSpriteInfo = nullptr;

	bool hasSprite = pixelInfo.SpriteCount > 0;
	bool renderOriginalTiles = ((_hdData->OptionFlags & (int)HdPackOptions::DontRenderOriginalTiles) == 0);
	if(pixelInfo.Tile.TileIndex != HdPpuTileInfo::NoTile) {
		hdPackTileInfo = GetCachedMatchingTile(x, y, &pixelInfo.Tile);
	}

	int lowestBgSprite = 999;
	
	DrawColor(_palette[pixelInfo.Tile.PpuBackgroundColor], outputBuffer, _hdData->Scale, screenWidth);

	bool hasBackground = false;
	for(int i = 0; i < _activeBgCount[0]; i++) {
		hasBackground |= DrawBackgroundLayer(HdNesPack::BehindBgSpritesPriority+i, x, y, outputBuffer, screenWidth);
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
					DrawColor(_palette[pixelInfo.Sprite[k].SpriteColor], outputBuffer, _hdData->Scale, screenWidth);
				}
			}
		}
	}
	
	for(int i = 0; i < _activeBgCount[1]; i++) {
		hasBackground |= DrawBackgroundLayer(HdNesPack::BehindBgPriority+i, x, y, outputBuffer, screenWidth);
	}
	
	if(hdPackTileInfo) {
		DrawTile(pixelInfo.Tile, *hdPackTileInfo, outputBuffer, screenWidth);
	} else if(renderOriginalTiles) {
		//Draw regular SD background tile
		if(!hasBackground || pixelInfo.Tile.BgColorIndex != 0) {
			DrawColor(_palette[pixelInfo.Tile.BgColor], outputBuffer, _hdData->Scale, screenWidth);
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
					DrawColor(_palette[pixelInfo.Sprite[k].SpriteColor], outputBuffer, _hdData->Scale, screenWidth);
				}
			}
		}
	}

	for(int i = 0; i < _activeBgCount[3]; i++) {
		DrawBackgroundLayer(HdNesPack::ForegroundPriority+i, x, y, outputBuffer, screenWidth);
	}
}

void HdNesPack::Process(HdScreenInfo *hdScreenInfo, uint32_t* outputBuffer, OverscanDimensions &overscan)
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

void HdNesPack::ProcessGrayscaleAndEmphasis(HdPpuPixelInfo &pixelInfo, uint32_t* outputBuffer, uint32_t hdScreenWidth)
{
	//Apply grayscale/emphasis bits on a scanline level (less accurate, but shouldn't cause issues and simpler to implement)
	uint32_t scale = GetScale();
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
#pragma once
#include "pch.h"
#include "NES/HdPacks/HdData.h"

class EmuSettings;

class HdNesPack
{
private:
	struct HdBgConfig
	{
		int32_t BackgroundIndex = -1;
		int32_t BgScrollX = 0;
		int32_t BgScrollY = 0;
		int16_t BgMinX = -1;
		int16_t BgMaxX = -1;
	};

	static constexpr uint8_t PriorityLevelsPerLayer = 10;
	static constexpr uint8_t BehindBgSpritesPriority = 0 * PriorityLevelsPerLayer;
	static constexpr uint8_t BehindBgPriority = 1 * PriorityLevelsPerLayer;
	static constexpr uint8_t BehindFgSpritesPriority = 2 * PriorityLevelsPerLayer;
	static constexpr uint8_t ForegroundPriority = 3 * PriorityLevelsPerLayer;

	EmuSettings* _settings = nullptr;
	HdPackData* _hdData = nullptr;

	uint8_t _activeBgCount[4] = {};
	HdBgConfig _bgConfig[40] = {};

	HdScreenInfo *_hdScreenInfo = nullptr;
	uint32_t _palette[512] = {};
	HdPackTileInfo* _cachedTile = nullptr;
	bool _cacheEnabled = false;
	bool _useCachedTile = false;
	int32_t _scrollX = 0;

	__forceinline void BlendColors(uint8_t output[4], uint8_t input[4]);
	__forceinline uint32_t AdjustBrightness(uint8_t input[4], int brightness);
	__forceinline void DrawColor(uint32_t color, uint32_t* outputBuffer, uint32_t scale, uint32_t screenWidth);
	__forceinline void DrawTile(HdPpuTileInfo &tileInfo, HdPackTileInfo &hdPackTileInfo, uint32_t* outputBuffer, uint32_t screenWidth);
	
	__forceinline HdPackTileInfo* GetCachedMatchingTile(uint32_t x, uint32_t y, HdPpuTileInfo* tile);
	__forceinline HdPackTileInfo* GetMatchingTile(uint32_t x, uint32_t y, HdPpuTileInfo* tile, bool* disableCache = nullptr);

	__forceinline bool DrawBackgroundLayer(uint8_t priority, uint32_t x, uint32_t y, uint32_t* outputBuffer, uint32_t screenWidth);
	__forceinline void DrawCustomBackground(HdBackgroundInfo& bgInfo, uint32_t *outputBuffer, uint32_t x, uint32_t y, uint32_t scale, uint32_t screenWidth);

	void OnLineStart(HdPpuPixelInfo &lineFirstPixel, uint8_t y);
	int32_t GetLayerIndex(uint8_t priority);
	void OnBeforeApplyFilter();
	__forceinline void GetPixels(uint32_t x, uint32_t y, HdPpuPixelInfo &pixelInfo, uint32_t *outputBuffer, uint32_t screenWidth);
	__forceinline void ProcessGrayscaleAndEmphasis(HdPpuPixelInfo &pixelInfo, uint32_t* outputBuffer, uint32_t hdScreenWidth);

public:
	static constexpr uint32_t CurrentVersion = 106;

	HdNesPack(EmuSettings* settings, HdPackData* hdData);
	~HdNesPack();

	uint32_t GetScale();
	
	void Process(HdScreenInfo *hdScreenInfo, uint32_t *outputBuffer, OverscanDimensions &overscan);
};

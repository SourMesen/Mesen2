#pragma once
#include "stdafx.h"
#include "Debugger/DebugTypes.h"
#include "Shared/NotificationManager.h"
#include "Shared/Emulator.h"

class Debugger;
struct GbPpuState;
struct PpuState;

struct ViewerRefreshConfig
{
	uint16_t Scanline;
	uint16_t Cycle;
};

struct DebugSpriteInfo
{
	uint16_t SpriteIndex;
	uint16_t TileIndex;
	int16_t X;
	int16_t Y;

	uint8_t Palette;
	uint8_t Priority;
	uint8_t Width;
	uint8_t Height;
	bool HorizontalMirror;
	bool VerticalMirror;
	bool UseSecondTable;
	bool Visible;
};

class PpuTools
{
protected:
	Emulator* _emu;
	Debugger* _debugger;
	unordered_map<uint32_t, ViewerRefreshConfig> _updateTimings;

	uint8_t GetTilePixelColor(const uint8_t* ram, const uint32_t ramMask, const uint8_t bpp, const uint32_t pixelStart, const uint8_t shift, const int secondByteOffset);

	void BlendColors(uint8_t output[4], uint8_t input[4]);

	uint32_t GetRgbPixelColor(uint32_t* colors, uint8_t colorIndex, uint8_t palette, uint8_t bpp, bool directColorMode, uint16_t basePaletteOffset);

public:
	PpuTools(Debugger* debugger, Emulator *emu);

	void GetTileView(GetTileViewOptions options, uint8_t *source, uint32_t srcSize, uint32_t* palette, uint32_t *outBuffer);
	
	virtual FrameInfo GetTilemapSize(GetTilemapOptions options, BaseState& state) = 0;
	virtual void GetTilemap(GetTilemapOptions options, BaseState& state, uint8_t* vram, uint32_t* palette, uint32_t* outBuffer) = 0;
	
	virtual FrameInfo GetSpritePreviewSize(GetSpritePreviewOptions options, BaseState& state) = 0;
	virtual void GetSpritePreview(GetSpritePreviewOptions options, BaseState& state, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, uint32_t* outBuffer) = 0;
	virtual uint32_t GetSpriteList(GetSpritePreviewOptions options, BaseState& baseState, uint8_t* oamRam, DebugSpriteInfo outBuffer[]) = 0;

	void SetViewerUpdateTiming(uint32_t viewerId, uint16_t scanline, uint16_t cycle);
	void RemoveViewer(uint32_t viewerId);
	
	__forceinline void UpdateViewers(uint16_t scanline, uint16_t cycle)
	{
		if(_updateTimings.size() > 0) {
			for(auto updateTiming : _updateTimings) {
				ViewerRefreshConfig cfg = updateTiming.second;
				if(cfg.Cycle == cycle && cfg.Scanline == scanline) {
					_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::ViewerRefresh, (void*)(uint64_t)updateTiming.first);
				}
			}
		}
	}
};
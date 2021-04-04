#pragma once
#include "stdafx.h"
#include "DebugTypes.h"
#include "Emulator.h"
#include "NotificationManager.h"
#include "SNES/PpuTypes.h"

class Ppu;
struct GbPpuState;

struct ViewerRefreshConfig
{
	uint16_t Scanline;
	uint16_t Cycle;
	CpuType Type;
};

class PpuTools
{
private:
	Emulator *_emu;
	unordered_map<uint32_t, ViewerRefreshConfig> _updateTimings;

	uint8_t GetTilePixelColor(const uint8_t* ram, const uint32_t ramMask, const uint8_t bpp, const uint32_t pixelStart, const uint8_t shift);

	void BlendColors(uint8_t output[4], uint8_t input[4]);

	uint32_t GetRgbPixelColor(uint8_t* cgram, uint8_t colorIndex, uint8_t palette, uint8_t bpp, bool directColorMode, uint16_t basePaletteOffset);

public:
	PpuTools(Emulator *emu);

	void GetTileView(GetTileViewOptions options, uint8_t *source, uint32_t srcSize, uint8_t *cgram, uint32_t *outBuffer);
	void GetTilemap(GetTilemapOptions options, PpuState state, uint8_t* vram, uint8_t* cgram, uint32_t *outBuffer);
	void GetSpritePreview(GetSpritePreviewOptions options, PpuState state, uint8_t* vram, uint8_t* oamRam, uint8_t* cgram, uint32_t *outBuffer);

	void SetViewerUpdateTiming(uint32_t viewerId, uint16_t scanline, uint16_t cycle, CpuType cpuType);
	void RemoveViewer(uint32_t viewerId);
	
	__forceinline void UpdateViewers(uint16_t scanline, uint16_t cycle, CpuType cpuType)
	{
		if(_updateTimings.size() > 0) {
			for(auto updateTiming : _updateTimings) {
				ViewerRefreshConfig cfg = updateTiming.second;
				if(cfg.Cycle == cycle && cfg.Scanline == scanline && cfg.Type == cpuType) {
					_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::ViewerRefresh, (void*)(uint64_t)updateTiming.first);
				}
			}
		}
	}

	void GetGameboyTilemap(uint8_t* vram, GbPpuState& state, uint16_t offset, uint32_t* outBuffer);
	void GetGameboySpritePreview(GetSpritePreviewOptions options, GbPpuState state, uint8_t* vram, uint8_t* oamRam, uint32_t* outBuffer);
};
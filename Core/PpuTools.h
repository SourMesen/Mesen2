#pragma once
#include "stdafx.h"
#include "DebugTypes.h"

class Ppu;
class Console;

class PpuTools
{
private:
	Ppu *_ppu;
	Console *_console;
	unordered_map<uint32_t, uint32_t> _updateTimings;

	uint8_t GetTilePixelColor(const uint8_t* ram, const uint32_t ramMask, const uint8_t bpp, const uint32_t pixelStart, const uint8_t shift);

	void BlendColors(uint8_t output[4], uint8_t input[4]);

	uint32_t GetRgbPixelColor(uint8_t* cgram, uint8_t colorIndex, uint8_t palette, uint8_t bpp, bool directColorMode, uint16_t basePaletteOffset);

public:
	PpuTools(Console *console, Ppu *ppu);

	void GetTileView(GetTileViewOptions options, uint8_t *source, uint32_t srcSize, uint8_t *cgram, uint32_t *outBuffer);
	void GetTilemap(GetTilemapOptions options, PpuState state, uint8_t* vram, uint8_t* cgram, uint32_t *outBuffer);
	void GetSpritePreview(GetSpritePreviewOptions options, PpuState state, uint8_t* vram, uint8_t* oamRam, uint8_t* cgram, uint32_t *outBuffer);

	void SetViewerUpdateTiming(uint32_t viewerId, uint16_t scanline, uint16_t cycle);
	void RemoveViewer(uint32_t viewerId);
	void UpdateViewers(uint16_t scanline, uint16_t cycle);

	void GetGameboyTilemap(uint8_t* vram, uint16_t offset, uint32_t* outBuffer);
};
#pragma once
#include "stdafx.h"

class Ppu;
class Console;
struct GetTilemapOptions;

class PpuTools
{
private:
	Ppu *_ppu;
	Console *_console;
	unordered_map<uint32_t, uint32_t> _updateTimings;

	uint16_t GetTilePixelColor(const uint8_t bpp, const uint16_t pixelStart, const uint8_t shift);

	uint32_t ToArgb(uint16_t color);
	void BlendColors(uint8_t output[4], uint8_t input[4]);

public:
	PpuTools(Console *console, Ppu* ppu);

	void GetTilemap(GetTilemapOptions options, uint32_t *outBuffer);

	void SetViewerUpdateTiming(uint32_t viewerId, uint16_t scanline, uint16_t cycle);
	void RemoveViewer(uint32_t viewerId);
	void UpdateViewers(uint16_t scanline, uint16_t cycle);
};
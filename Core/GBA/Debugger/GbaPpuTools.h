#pragma once
#include "pch.h"
#include "Debugger/PpuTools.h"

class Debugger;
class Emulator;
struct GbaPpuState;

class GbaPpuTools final : public PpuTools
{
private:
	void GetSpriteInfo(DebugSpriteInfo& sprite, uint32_t* spritePreview, uint16_t spriteIndex, GetSpritePreviewOptions& options, GbaPpuState& state, uint8_t* vram, uint8_t* oamRam, uint32_t* palette);

	uint8_t _memoryAccess[228 * 308 * 4] = {};

	void GetSpritePreview(GetSpritePreviewOptions options, BaseState& state, DebugSpriteInfo* sprites, uint32_t* spritePreviews, uint32_t* palette, uint32_t* outBuffer);

public:
	GbaPpuTools(Debugger* debugger, Emulator *emu);

	void SetMemoryAccessData(uint16_t scanline, uint8_t* data);

	DebugTilemapInfo GetTilemap(GetTilemapOptions options, BaseState& state, BaseState& ppuToolsState, uint8_t* vram, uint32_t* palette, uint32_t *outBuffer) override;
	FrameInfo GetTilemapSize(GetTilemapOptions options, BaseState& state) override;
	DebugTilemapTileInfo GetTilemapTileInfo(uint32_t x, uint32_t y, uint8_t* vram, GetTilemapOptions options, BaseState& baseState, BaseState& ppuToolsState) override;

	DebugSpritePreviewInfo GetSpritePreviewInfo(GetSpritePreviewOptions options, BaseState& state, BaseState& ppuToolsState) override;
	void GetSpriteList(GetSpritePreviewOptions options, BaseState& baseState, BaseState& ppuToolsState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, DebugSpriteInfo outBuffer[], uint32_t* spritePreviews, uint32_t* screenPreview) override;

	DebugPaletteInfo GetPaletteInfo(GetPaletteInfoOptions options) override;
	void SetPaletteColor(int32_t colorIndex, uint32_t color) override;
};
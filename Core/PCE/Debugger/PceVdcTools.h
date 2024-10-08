#pragma once
#include "pch.h"
#include "Debugger/PpuTools.h"

class Debugger;
class Emulator;
class PceConsole;
struct PceVdcState;

class PceVdcTools final : public PpuTools
{
private:
	PceConsole* _console = nullptr;

	void GetSpriteInfo(PceVdcState& state, DebugSpriteInfo& sprite, uint32_t* spritePreview, uint16_t spriteIndex, GetSpritePreviewOptions& options, uint8_t* vram, uint8_t* oamRam, uint32_t* palette);

	template<TileFormat format>
	DebugTilemapInfo InternalGetTilemap(GetTilemapOptions options, PceVdcState& state, uint8_t* vram, uint32_t* palette, uint32_t* outBuffer);

	template<TileFormat format>
	void InternalGetSpriteInfo(DebugSpriteInfo& sprite, uint32_t* spritePreview, uint16_t spriteIndex, GetSpritePreviewOptions& options, uint8_t* vram, uint8_t* oamRam, uint32_t* palette);

	void GetSpritePreview(GetSpritePreviewOptions options, BaseState& state, DebugSpriteInfo* sprites, uint32_t* spritePreviews, uint32_t* palette, uint32_t* outBuffer);

public:
	PceVdcTools(Debugger* debugger, Emulator *emu, PceConsole* console);

	void SetViewerUpdateTiming(uint32_t viewerId, uint16_t scanline, uint16_t cycle) override;

	DebugTilemapInfo GetTilemap(GetTilemapOptions options, BaseState& state, BaseState& ppuToolsState, uint8_t* vram, uint32_t* palette, uint32_t *outBuffer) override;
	FrameInfo GetTilemapSize(GetTilemapOptions options, BaseState& state) override;
	DebugTilemapTileInfo GetTilemapTileInfo(uint32_t x, uint32_t y, uint8_t* vram, GetTilemapOptions options, BaseState& baseState, BaseState& ppuToolsState) override;

	DebugSpritePreviewInfo GetSpritePreviewInfo(GetSpritePreviewOptions options, BaseState& state, BaseState& ppuToolsState) override;
	void GetSpriteList(GetSpritePreviewOptions options, BaseState& baseState, BaseState& ppuToolsState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, DebugSpriteInfo outBuffer[], uint32_t* spritePreviews, uint32_t* screenPreview) override;

	DebugPaletteInfo GetPaletteInfo(GetPaletteInfoOptions options) override;
	void SetPaletteColor(int32_t colorIndex, uint32_t color) override;
};
#pragma once
#include "stdafx.h"
#include "Debugger/PpuTools.h"

class Debugger;
class Emulator;
class PceConsole;
struct PcePpuState;

class PcePpuTools final : public PpuTools
{
private:
	void GetSpriteInfo(DebugSpriteInfo& sprite, uint16_t spriteIndex, GetSpritePreviewOptions& options, PcePpuState& state, uint8_t* vram, uint8_t* oamRam, uint32_t* palette);

public:
	PcePpuTools(Debugger* debugger, Emulator *emu, PceConsole* console);

	DebugTilemapInfo GetTilemap(GetTilemapOptions options, BaseState& state, uint8_t* vram, uint32_t* palette, uint32_t *outBuffer) override;
	FrameInfo GetTilemapSize(GetTilemapOptions options, BaseState& state) override;
	DebugTilemapTileInfo GetTilemapTileInfo(uint32_t x, uint32_t y, uint8_t* vram, GetTilemapOptions options, BaseState& baseState) override;

	DebugSpritePreviewInfo GetSpritePreviewInfo(GetSpritePreviewOptions options, BaseState& state) override;
	void GetSpritePreview(GetSpritePreviewOptions options, BaseState& state, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, uint32_t *outBuffer) override;
	void GetSpriteList(GetSpritePreviewOptions options, BaseState& baseState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, DebugSpriteInfo outBuffer[]) override;

	DebugPaletteInfo GetPaletteInfo() override;
};
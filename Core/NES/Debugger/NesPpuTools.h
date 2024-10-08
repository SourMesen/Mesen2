#pragma once
#include "pch.h"
#include "Debugger/PpuTools.h"
#include "NES/Debugger/IExtModeMapperDebug.h"

class Debugger;
class Emulator;
class BaseMapper;
class NesConsole;
struct NesPpuState;

struct NesPpuToolsState
{
	ExtModeConfig ExtConfig;
};

class NesPpuTools final : public PpuTools
{
private:
	NesConsole* _console = nullptr;
	BaseMapper* _mapper = nullptr;
	void GetSpriteInfo(DebugSpriteInfo& sprite, uint32_t* spritePreview, uint32_t spriteIndex, GetSpritePreviewOptions& options, NesPpuState& state, NesPpuToolsState& ppuToolsState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette);
	void GetSpritePreview(GetSpritePreviewOptions options, BaseState& state, DebugSpriteInfo* sprites, uint32_t* spritePreviews, uint32_t* palette, uint32_t *outBuffer);

	DebugTilemapInfo GetWindowTilemap(GetTilemapOptions options, NesPpuState& state, IExtModeMapperDebug* exMode, ExtModeConfig& extCfg, uint8_t* vram, uint32_t* palette, uint32_t* outBuffer);

	void DrawNametable(uint8_t* ntSource, uint32_t ntBaseAddr, uint8_t ntIndex, GetTilemapOptions options, NesPpuState& state, IExtModeMapperDebug* exMode, ExtModeConfig& extCfg, uint8_t* vram, uint32_t* palette, uint32_t* outBuffer, uint32_t bufferWidth);
	void ApplyHighlights(GetTilemapOptions options, uint8_t nametableIndex, uint8_t* vram, uint32_t* outBuffer);

public:
	NesPpuTools(Debugger* debugger, Emulator *emu, NesConsole* console);

	void GetPpuToolsState(BaseState& state) override;

	DebugTilemapInfo GetTilemap(GetTilemapOptions options, BaseState& state, BaseState& ppuToolsState, uint8_t* vram, uint32_t* palette, uint32_t *outBuffer) override;
	FrameInfo GetTilemapSize(GetTilemapOptions options, BaseState& state) override;
	DebugTilemapTileInfo GetTilemapTileInfo(uint32_t x, uint32_t y, uint8_t* vram, GetTilemapOptions options, BaseState& baseState, BaseState& ppuToolsState) override;

	DebugSpritePreviewInfo GetSpritePreviewInfo(GetSpritePreviewOptions options, BaseState& state, BaseState& ppuToolsState) override;
	void GetSpriteList(GetSpritePreviewOptions options, BaseState& baseState, BaseState& ppuToolsState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, DebugSpriteInfo outBuffer[], uint32_t* spritePreview, uint32_t* screenPreview) override;

	DebugPaletteInfo GetPaletteInfo(GetPaletteInfoOptions options) override;
	void SetPaletteColor(int32_t colorIndex, uint32_t color) override;
};
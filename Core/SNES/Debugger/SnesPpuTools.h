#pragma once
#include "pch.h"
#include "Debugger/PpuTools.h"

class Debugger;
class Emulator;
struct BaseState;
struct LayerConfig;
struct FrameInfo;
struct SnesPpuState;

struct SnesPpuToolsState
{
	uint8_t ScanlineBgMode[239];
	int32_t Mode7StartX[239];
	int32_t Mode7StartY[239];
	int32_t Mode7EndX[239];
	int32_t Mode7EndY[239];
};

class SnesPpuTools final : public PpuTools
{
private:
	static constexpr int MainScreenViewLayer = 4;
	static constexpr int SubScreenViewLayer = 5;

	SnesPpuToolsState _state = {};
	uint16_t _mainScreenBuffer[256 * 239] = {};
	uint16_t _subScreenBuffer[256 * 239] = {};

	void GetSpriteInfo(DebugSpriteInfo& sprite, uint32_t* spritePreview, uint16_t spriteIndex, GetSpritePreviewOptions& options, SnesPpuState& state, uint8_t* vram, uint8_t* oamRam, uint32_t* palette);
	
	template<TileFormat format> void RenderMode7Tilemap(GetTilemapOptions& options, uint8_t* vram, uint32_t* outBuffer, const uint32_t* palette);
	template<TileFormat format> void RenderTilemap(GetTilemapOptions& options, int rowCount, LayerConfig& layer, int columnCount, uint8_t* vram, int tileHeight, int tileWidth, bool largeTileHeight, bool largeTileWidth, uint8_t bpp, uint32_t* outBuffer, FrameInfo outputSize, const uint32_t* palette, uint16_t basePaletteOffset);
	template<TileFormat format, bool largeTileHeight, bool largeTileWidth> void RenderTilemap(GetTilemapOptions& options, int rowCount, LayerConfig& layer, int columnCount, uint8_t* vram, int tileHeight, int tileWidth, uint8_t bpp, uint32_t* outBuffer, FrameInfo outputSize, const uint32_t* palette, uint16_t basePaletteOffset);
	
	DebugTilemapInfo RenderScreenView(uint8_t layer, uint32_t* outBuffer);
	void GetSpritePreview(GetSpritePreviewOptions options, BaseState& state, BaseState& ppuToolsState, DebugSpriteInfo* sprites, uint32_t* spritePreviews, uint32_t* palette, uint32_t* outBuffer);

public:
	SnesPpuTools(Debugger* debugger, Emulator *emu);

	void GetPpuToolsState(BaseState& state) override;

	void SetPpuScanlineState(uint16_t scanline, uint8_t mode, int32_t mode7startX, int32_t mode7startY, int32_t mode7endX, int32_t mode7endY);
	void SetPpuRowBuffers(uint16_t scanline, uint16_t xStart, uint16_t xEnd, uint16_t mainScreenRowBuffer[256], uint16_t subScreenRowBuffer[256]);

	DebugTilemapInfo GetTilemap(GetTilemapOptions options, BaseState& state, BaseState& ppuToolsState, uint8_t* vram, uint32_t* palette, uint32_t* outBuffer) override;
	FrameInfo GetTilemapSize(GetTilemapOptions options, BaseState& state) override;
	DebugTilemapTileInfo GetTilemapTileInfo(uint32_t x, uint32_t y, uint8_t* vram, GetTilemapOptions options, BaseState& baseState, BaseState& ppuToolsState) override;

	void GetSpriteList(GetSpritePreviewOptions options, BaseState& baseState, BaseState& ppuToolsState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, DebugSpriteInfo outBuffer[], uint32_t* spritePreviews, uint32_t* screenPreview) override;
	DebugSpritePreviewInfo GetSpritePreviewInfo(GetSpritePreviewOptions options, BaseState& state, BaseState& ppuToolsState) override;

	DebugPaletteInfo GetPaletteInfo(GetPaletteInfoOptions options) override;
	void SetPaletteColor(int32_t colorIndex, uint32_t color) override;
};
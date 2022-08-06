#pragma once
#include "stdafx.h"
#include "Debugger/DebugTypes.h"
#include "Shared/NotificationManager.h"
#include "Shared/Emulator.h"

class Debugger;
struct GbPpuState;
struct SnesPpuState;

struct ViewerRefreshConfig
{
	uint16_t Scanline;
	uint16_t Cycle;
};

enum class NullableBoolean
{
	Undefined = -1,
	False = 0,
	True = 1
};

enum class DebugSpritePriority
{
	Undefined = -1,
	Number0 = 0,
	Number1 = 1,
	Number2 = 2,
	Number3 = 3,
	Foreground = 4,
	Background = 5
};

struct DebugSpriteInfo
{
	int32_t TileIndex;
	int32_t TileAddress;
	int32_t PaletteAddress;
	TileFormat Format;

	int16_t SpriteIndex;

	int16_t X;
	int16_t Y;
	int16_t RawX;
	int16_t RawY;

	int16_t Bpp;
	int16_t Palette;
	DebugSpritePriority Priority;
	int16_t Width;
	int16_t Height;
	bool HorizontalMirror;
	bool VerticalMirror;
	bool Visible;
	bool UseExtendedVram;
	NullableBoolean UseSecondTable;

	uint32_t TileCount;
	uint32_t TileAddresses[8 * 8];
	uint32_t SpritePreview[64 * 64];

public:
	void Init()
	{
		TileIndex = -1;
		TileAddress = -1;
		PaletteAddress = -1;
		Format = {};
		SpriteIndex = -1;
		X = -1;
		Y = -1;
		RawX = -1;
		RawY = -1;
		Bpp = 2;
		Palette = -1;
		Priority = DebugSpritePriority::Undefined;
		Width = -1;
		Height = -1;
		HorizontalMirror = false;
		VerticalMirror = false;
		Visible = false;
		UseExtendedVram = false;
		UseSecondTable = NullableBoolean::Undefined;
		TileCount = 0;
	}
};

enum class TilemapMirroring
{
	None,
	Horizontal,
	Vertical,
	SingleScreenA,
	SingleScreenB,
	FourScreens,
};

struct DebugTilemapInfo
{
	uint32_t Bpp;
	TileFormat Format;
	TilemapMirroring Mirroring;
	
	uint32_t TileWidth;
	uint32_t TileHeight;

	uint32_t ScrollX;
	uint32_t ScrollWidth;
	uint32_t ScrollY;
	uint32_t ScrollHeight;

	uint32_t RowCount;
	uint32_t ColumnCount;
	uint32_t TilemapAddress;
	uint32_t TilesetAddress;
};

struct DebugTilemapTileInfo
{
	int32_t Row = -1;
	int32_t Column = -1;
	int32_t Width = -1;
	int32_t Height = -1;

	int32_t TileMapAddress = -1;

	int32_t TileIndex = -1;
	int32_t TileAddress = -1;
	
	int32_t PaletteIndex = -1;
	int32_t PaletteAddress = -1;

	int32_t AttributeAddress = -1;

	NullableBoolean HorizontalMirroring = NullableBoolean::Undefined;
	NullableBoolean VerticalMirroring = NullableBoolean::Undefined;
	NullableBoolean HighPriority = NullableBoolean::Undefined;
};

struct DebugSpritePreviewInfo
{
	uint32_t Width;
	uint32_t Height;
	uint32_t SpriteCount;
	int32_t CoordOffsetX;
	int32_t CoordOffsetY;
};

enum class RawPaletteFormat
{
	Indexed,
	Rgb555,
	Rgb333
};

struct DebugPaletteInfo
{
	MemoryType PaletteMemType;
	bool HasMemType;

	uint32_t ColorCount;
	uint32_t BgColorCount;
	uint32_t SpriteColorCount;
	uint32_t ColorsPerPalette;

	RawPaletteFormat RawFormat;
	uint32_t RawPalette[512];
	uint32_t RgbPalette[512];
};

class PpuTools
{
protected:
	Emulator* _emu;
	Debugger* _debugger;
	unordered_map<uint32_t, ViewerRefreshConfig> _updateTimings;

	uint8_t GetTilePixelColor(const uint8_t* ram, const uint32_t ramMask, uint32_t pixelStart, uint8_t shift, const TileFormat format);

	void BlendColors(uint8_t output[4], uint8_t input[4]);

	uint32_t GetRgbPixelColor(TileFormat format, const uint32_t* colors, uint8_t colorIndex, uint8_t palette);
	
	bool IsTileHidden(MemoryType memType, uint32_t addr, GetTileViewOptions& options);

public:
	PpuTools(Debugger* debugger, Emulator *emu);

	virtual void GetPpuToolsState(BaseState& state) {};

	virtual DebugPaletteInfo GetPaletteInfo(GetPaletteInfoOptions options) = 0;

	void GetTileView(GetTileViewOptions options, uint8_t *source, uint32_t srcSize, const uint32_t* palette, uint32_t *outBuffer);
	
	virtual DebugTilemapTileInfo GetTilemapTileInfo(uint32_t x, uint32_t y, uint8_t* vram, GetTilemapOptions options, BaseState& baseState) = 0;
	virtual FrameInfo GetTilemapSize(GetTilemapOptions options, BaseState& state) = 0;
	virtual DebugTilemapInfo GetTilemap(GetTilemapOptions options, BaseState& state, uint8_t* vram, uint32_t* palette, uint32_t* outBuffer) = 0;
	
	virtual DebugSpritePreviewInfo GetSpritePreviewInfo(GetSpritePreviewOptions options, BaseState& state) = 0;
	virtual void GetSpritePreview(GetSpritePreviewOptions options, BaseState& state, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, uint32_t* outBuffer) = 0;
	virtual void GetSpriteList(GetSpritePreviewOptions options, BaseState& baseState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, DebugSpriteInfo outBuffer[]) = 0;

	void SetTilePixel(AddressInfo tileAddress, TileFormat format, int32_t x, int32_t y, int32_t color);
	virtual void SetPaletteColor(int32_t colorIndex, uint32_t color) = 0;

	virtual void SetViewerUpdateTiming(uint32_t viewerId, uint16_t scanline, uint16_t cycle);
	void RemoveViewer(uint32_t viewerId);

	void UpdateViewers(uint16_t scanline, uint16_t cycle);
	
	__forceinline bool HasOpenedViewer()
	{
		return _updateTimings.size() > 0;
	}
};
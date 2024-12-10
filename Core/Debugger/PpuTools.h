#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"
#include "Shared/NotificationManager.h"
#include "Shared/Emulator.h"
#include "Shared/ColorUtilities.h"

class Debugger;

struct ViewerRefreshConfig
{
	uint16_t Scanline;
	uint16_t Cycle;
};

enum class SpriteVisibility : uint8_t
{
	Visible = 0,
	Offscreen = 1,
	Disabled = 2
};

enum class NullableBoolean : int8_t
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
	uint16_t Width;
	uint16_t Height;
	NullableBoolean HorizontalMirror;
	NullableBoolean VerticalMirror;
	NullableBoolean MosaicEnabled;
	NullableBoolean BlendingEnabled;
	NullableBoolean WindowMode;
	NullableBoolean TransformEnabled;
	NullableBoolean DoubleSize;
	int8_t TransformParamIndex;
	SpriteVisibility Visibility;
	bool UseExtendedVram;
	NullableBoolean UseSecondTable;

	uint32_t TileCount;
	uint32_t TileAddresses[8 * 8];

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
		Width = 0;
		Height = 0;
		HorizontalMirror = NullableBoolean::Undefined;
		VerticalMirror = NullableBoolean::Undefined;
		MosaicEnabled = NullableBoolean::Undefined;
		TransformEnabled = NullableBoolean::Undefined;
		BlendingEnabled = NullableBoolean::Undefined;
		WindowMode = NullableBoolean::Undefined;
		DoubleSize = NullableBoolean::Undefined;
		TransformParamIndex = -1;
		Visibility = SpriteVisibility::Offscreen;
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
	int8_t Priority = -1;
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

	int32_t PixelData = -1;

	int32_t PaletteIndex = -1;
	int32_t PaletteAddress = -1;
	int32_t BasePaletteIndex = -1;

	int32_t AttributeAddress = -1;
	int16_t AttributeData = -1;

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

	uint32_t VisibleX;
	uint32_t VisibleY;
	uint32_t VisibleWidth;
	uint32_t VisibleHeight;

	bool WrapBottomToTop;
	bool WrapRightToLeft;
};

enum class RawPaletteFormat
{
	Indexed,
	Rgb555,
	Rgb333,
	Rgb222,
	Rgb444,
	Bgr444
};

struct DebugPaletteInfo
{
	MemoryType PaletteMemType;
	uint32_t PaletteMemOffset;
	bool HasMemType;

	uint32_t ColorCount;
	uint32_t BgColorCount;
	uint32_t SpriteColorCount;
	uint32_t SpritePaletteOffset;

	uint32_t ColorsPerPalette;

	RawPaletteFormat RawFormat;
	uint32_t RawPalette[512];
	uint32_t RgbPalette[512];
};

class PpuTools
{
protected:
	static constexpr uint32_t _spritePreviewSize = 128*128;
	static constexpr uint32_t _grayscaleColorsBpp1[2] = { 0xFF000000, 0xFFFFFFFF };
	static constexpr uint32_t _grayscaleColorsBpp2[4] = { 0xFF000000, 0xFF666666, 0xFFBBBBBB, 0xFFFFFFFF };
	static constexpr uint32_t _grayscaleColorsBpp4[16] = {
		0xFF000000, 0xFF303030, 0xFF404040, 0xFF505050, 0xFF606060, 0xFF707070, 0xFF808080, 0xFF909090,
		0xFF989898, 0xFFA0A0A0, 0xFFAAAAAA, 0xFFBBBBBB, 0xFFCCCCCC, 0xFFDDDDDD, 0xFFEEEEEE, 0xFFFFFFFF
	};

	Emulator* _emu;
	Debugger* _debugger;
	unordered_map<uint32_t, ViewerRefreshConfig> _updateTimings;

	void BlendColors(uint8_t output[4], uint8_t input[4]);

	template<TileFormat format> __forceinline uint32_t GetRgbPixelColor(const uint32_t* colors, uint8_t colorIndex, uint8_t palette);
	template<TileFormat format> __forceinline uint8_t GetTilePixelColor(const uint8_t* ram, const uint32_t ramMask, uint32_t rowStart, uint8_t pixelIndex);
	
	bool IsTileHidden(MemoryType memType, uint32_t addr, GetTileViewOptions& options);

	uint32_t GetBackgroundColor(TileBackground bgColor, const uint32_t* colors, uint8_t paletteIndex = 0, uint8_t bpp = 0);
	uint32_t GetSpriteBackgroundColor(SpriteBackground bgColor, const uint32_t* colors, bool useDarkerColor);

	void GetSetTilePixel(AddressInfo tileAddress, TileFormat format, int32_t x, int32_t y, int32_t& color, bool forGet);

public:
	PpuTools(Debugger* debugger, Emulator *emu);

	virtual void GetPpuToolsState(BaseState& state) {};

	virtual DebugPaletteInfo GetPaletteInfo(GetPaletteInfoOptions options) = 0;

	void GetTileView(GetTileViewOptions options, uint8_t *source, uint32_t srcSize, const uint32_t* palette, uint32_t *outBuffer);

	virtual DebugTilemapTileInfo GetTilemapTileInfo(uint32_t x, uint32_t y, uint8_t* vram, GetTilemapOptions options, BaseState& baseState, BaseState& ppuToolsState) = 0;
	virtual FrameInfo GetTilemapSize(GetTilemapOptions options, BaseState& state) = 0;
	virtual DebugTilemapInfo GetTilemap(GetTilemapOptions options, BaseState& state, BaseState& ppuToolsState, uint8_t* vram, uint32_t* palette, uint32_t* outBuffer) = 0;
	
	virtual DebugSpritePreviewInfo GetSpritePreviewInfo(GetSpritePreviewOptions options, BaseState& state, BaseState& ppuToolsState) = 0;
	virtual void GetSpriteList(GetSpritePreviewOptions options, BaseState& baseState, BaseState& ppuToolsState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, DebugSpriteInfo outBuffer[], uint32_t* spritePreviews, uint32_t* screenPreview) = 0;

	int32_t GetTilePixel(AddressInfo tileAddress, TileFormat format, int32_t x, int32_t y);
	void SetTilePixel(AddressInfo tileAddress, TileFormat format, int32_t x, int32_t y, int32_t color);
	virtual void SetPaletteColor(int32_t colorIndex, uint32_t color) = 0;

	virtual void SetViewerUpdateTiming(uint32_t viewerId, uint16_t scanline, uint16_t cycle);
	void RemoveViewer(uint32_t viewerId);

	void UpdateViewers(uint16_t scanline, uint16_t cycle);
	
	__forceinline bool HasOpenedViewer()
	{
		return _updateTimings.size() > 0;
	}
	template<TileFormat format>
	void InternalGetTileView(GetTileViewOptions options, uint8_t* source, uint32_t srcSize, const uint32_t* colors, uint32_t* outBuffer);
};

template<TileFormat format> uint32_t PpuTools::GetRgbPixelColor(const uint32_t* colors, uint8_t colorIndex, uint8_t palette)
{
	switch(format) {
		case TileFormat::DirectColor:
			return ColorUtilities::Rgb555ToArgb(
				((((colorIndex & 0x07) << 1) | (palette & 0x01)) << 1) |
				(((colorIndex & 0x38) | ((palette & 0x02) << 1)) << 4) |
				(((colorIndex & 0xC0) | ((palette & 0x04) << 3)) << 7)
			);

		case TileFormat::NesBpp2:
		case TileFormat::Bpp2:
			return colors[palette * 4 + colorIndex];

		case TileFormat::Bpp4:
		case TileFormat::SmsBpp4:
		case TileFormat::GbaBpp4:
		case TileFormat::WsBpp4Packed:
		case TileFormat::PceSpriteBpp4:
		case TileFormat::PceSpriteBpp2Sp01:
		case TileFormat::PceSpriteBpp2Sp23:
		case TileFormat::PceBackgroundBpp2Cg0:
		case TileFormat::PceBackgroundBpp2Cg1:
			return colors[palette * 16 + colorIndex];

		case TileFormat::Bpp8:
		case TileFormat::GbaBpp8:
		case TileFormat::Mode7:
		case TileFormat::Mode7ExtBg:
			return colors[palette * 256 + colorIndex];

		case TileFormat::Mode7DirectColor:
			return ColorUtilities::Rgb555ToArgb(((colorIndex & 0x07) << 2) | ((colorIndex & 0x38) << 4) | ((colorIndex & 0xC0) << 7));

		case TileFormat::SmsSgBpp1:
			return colors[palette * 2 + colorIndex];

		default:
			throw std::runtime_error("unsupported format");
	}
}

template<TileFormat format> __forceinline uint8_t PpuTools::GetTilePixelColor(const uint8_t* ram, const uint32_t ramMask, uint32_t rowStart, uint8_t pixelIndex)
{
	uint8_t shift = (7 - pixelIndex);
	uint8_t color;
	switch(format) {
		case TileFormat::PceSpriteBpp4:
		case TileFormat::PceSpriteBpp2Sp01:
		case TileFormat::PceSpriteBpp2Sp23:
			shift = 15 - pixelIndex;
			if(shift >= 8) {
				shift -= 8;
				rowStart++;
			}
			break;

		default:
			break;
	}

	switch(format) {
		case TileFormat::PceSpriteBpp4:
			color = (((ram[(rowStart + 0) & ramMask] >> shift) & 0x01) << 0);
			color |= (((ram[(rowStart + 32) & ramMask] >> shift) & 0x01) << 1);
			color |= (((ram[(rowStart + 64) & ramMask] >> shift) & 0x01) << 2);
			color |= (((ram[(rowStart + 96) & ramMask] >> shift) & 0x01) << 3);
			return color;
	
		case TileFormat::PceSpriteBpp2Sp01:
			color = (((ram[(rowStart + 0) & ramMask] >> shift) & 0x01) << 0);
			color |= (((ram[(rowStart + 32) & ramMask] >> shift) & 0x01) << 1);
			return color;

		case TileFormat::PceSpriteBpp2Sp23:
			color = (((ram[(rowStart + 64) & ramMask] >> shift) & 0x01) << 0);
			color |= (((ram[(rowStart + 96) & ramMask] >> shift) & 0x01) << 1);
			return color;

		case TileFormat::PceBackgroundBpp2Cg0:
			color = (((ram[(rowStart + 0) & ramMask] >> shift) & 0x01) << 0);
			color |= (((ram[(rowStart + 1) & ramMask] >> shift) & 0x01) << 1);
			return color;

		case TileFormat::PceBackgroundBpp2Cg1:
			color = (((ram[(rowStart + 16) & ramMask] >> shift) & 0x01) << 0);
			color |= (((ram[(rowStart + 17) & ramMask] >> shift) & 0x01) << 1);
			return color;

		case TileFormat::Bpp2:
			color = (((ram[rowStart & ramMask] >> shift) & 0x01) << 0);
			color |= (((ram[(rowStart + 1) & ramMask] >> shift) & 0x01) << 1);
			return color;

		case TileFormat::NesBpp2:
			color = (((ram[(rowStart + 0) & ramMask] >> shift) & 0x01) << 0);
			color |= (((ram[(rowStart + 8) & ramMask] >> shift) & 0x01) << 1);
			return color;

		case TileFormat::Bpp4:
			color = (((ram[(rowStart + 0) & ramMask] >> shift) & 0x01) << 0);
			color |= (((ram[(rowStart + 1) & ramMask] >> shift) & 0x01) << 1);
			color |= (((ram[(rowStart + 16) & ramMask] >> shift) & 0x01) << 2);
			color |= (((ram[(rowStart + 17) & ramMask] >> shift) & 0x01) << 3);
			return color;

		case TileFormat::Bpp8:
		case TileFormat::DirectColor:
			color = (((ram[(rowStart + 0) & ramMask] >> shift) & 0x01) << 0);
			color |= (((ram[(rowStart + 1) & ramMask] >> shift) & 0x01) << 1);
			color |= (((ram[(rowStart + 16) & ramMask] >> shift) & 0x01) << 2);
			color |= (((ram[(rowStart + 17) & ramMask] >> shift) & 0x01) << 3);
			color |= (((ram[(rowStart + 32) & ramMask] >> shift) & 0x01) << 4);
			color |= (((ram[(rowStart + 33) & ramMask] >> shift) & 0x01) << 5);
			color |= (((ram[(rowStart + 48) & ramMask] >> shift) & 0x01) << 6);
			color |= (((ram[(rowStart + 49) & ramMask] >> shift) & 0x01) << 7);
			return color;

		case TileFormat::Mode7:
		case TileFormat::Mode7DirectColor:
			return ram[(rowStart + pixelIndex * 2 + 1) & ramMask];

		case TileFormat::Mode7ExtBg:
			return ram[(rowStart + pixelIndex * 2 + 1) & ramMask] & 0x7F;

		case TileFormat::SmsBpp4:
			color = (((ram[(rowStart + 0) & ramMask] >> shift) & 0x01) << 0);
			color |= (((ram[(rowStart + 1) & ramMask] >> shift) & 0x01) << 1);
			color |= (((ram[(rowStart + 2) & ramMask] >> shift) & 0x01) << 2);
			color |= (((ram[(rowStart + 3) & ramMask] >> shift) & 0x01) << 3);
			return color;

		case TileFormat::SmsSgBpp1:
			color = ((ram[rowStart & ramMask] >> shift) & 0x01);
			return color;

		case TileFormat::GbaBpp4: {
			uint8_t pixelOffset = (7 - shift);
			uint32_t addr = (rowStart + (pixelOffset >> 1));
			if(addr <= ramMask) {
				if(pixelOffset & 0x01) {
					return ram[addr] >> 4;
				} else {
					return ram[addr] & 0x0F;
				}
			} else {
				return 0;
			}
		}

		case TileFormat::GbaBpp8: {
			uint8_t pixelOffset = (7 - shift);
			uint32_t addr = rowStart + pixelOffset;
			if(addr <= ramMask) {
				return ram[addr];
			} else {
				return 0;
			}
		}

		case TileFormat::WsBpp4Packed: {
			uint8_t pixelOffset = (7 - shift);
			uint32_t addr = (rowStart + (pixelOffset >> 1));
			if(addr <= ramMask) {
				if(pixelOffset & 0x01) {
					return ram[addr] & 0x0F;
				} else {
					return ram[addr] >> 4;
				}
			} else {
				return 0;
			}
		}

		default:
			throw std::runtime_error("unsupported format");
	}
}

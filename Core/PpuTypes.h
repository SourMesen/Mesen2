#pragma once
#include "stdafx.h"

struct SpriteInfo
{
	int16_t X;
	uint8_t Y;
	uint8_t Index;
	uint8_t Width;
	uint8_t Height;
	bool HorizontalMirror;
	uint8_t Priority;

	uint8_t Palette;
	int8_t ColumnOffset;

	int16_t DrawX;
	uint16_t FetchAddress;
	uint16_t ChrData[2];

	bool IsVisible(uint16_t scanline)
	{
		if(X != -256 && (X + Width <= 0 || X > 255)) {
			//Sprite is not visible (and must be ignored for time/range flag calculations)
			//Sprites at X=-256 are always used when considering Time/Range flag calculations, but not actually drawn.
			return false;
		}

		uint8_t endY = (Y + Height) & 0xFF;
		return (scanline >= Y && scanline < endY) || (endY < Y && scanline < endY);
	}
};

struct TileData
{
	uint16_t TilemapData;
	uint16_t VScroll;
	uint16_t ChrData[4];
};

struct LayerData
{
	TileData Tiles[33];
	uint8_t HasPriorityTiles;
};

struct LayerConfig
{
	uint16_t TilemapAddress;
	uint16_t ChrAddress;

	uint16_t HScroll;
	uint16_t VScroll;

	bool DoubleWidth;
	bool DoubleHeight;

	bool LargeTiles;
};

struct Mode7Config
{
	int16_t Matrix[4];

	int16_t HScroll;
	int16_t VScroll;
	int16_t CenterX;
	int16_t CenterY;

	uint8_t ValueLatch;
	
	bool LargeMap;
	bool FillWithTile0;
	bool HorizontalMirroring;
	bool VerticalMirroring;
	bool ExtBgEnabled;

	//Holds the scroll values at the start of a scanline for the entire scanline
	int16_t HScrollLatch;
	int16_t VScrollLatch;
};

struct PpuState
{
	uint16_t Cycle;
	uint16_t Scanline;
	uint16_t HClock;
	uint32_t FrameCount;
	bool OverscanMode;

	uint8_t BgMode;
	bool DirectColorMode;
	bool HiResMode;
	bool ScreenInterlace;
	Mode7Config Mode7;
	LayerConfig Layers[4];

	uint8_t OamMode;
	uint16_t OamBaseAddress;
	uint16_t OamAddressOffset;
	bool EnableOamPriority;
	bool ObjInterlace;
};

struct WindowConfig
{
	bool ActiveLayers[6];
	bool InvertedLayers[6];
	uint8_t Left;
	uint8_t Right;

	template<uint8_t layerIndex>
	bool PixelNeedsMasking(int x)
	{
		if(InvertedLayers[layerIndex]) {
			if(Left > Right) {
				return true;
			} else {
				return x < Left || x > Right;
			}
		} else {
			if(Left > Right) {
				return false;
			} else {
				return x >= Left && x <= Right;
			}
		}
	}
};

enum class  WindowMaskLogic
{
	Or = 0,
	And = 1,
	Xor = 2,
	Xnor = 3
};

enum class ColorWindowMode
{
	Never = 0,
	OutsideWindow = 1,
	InsideWindow = 2,
	Always = 3
};

enum PixelFlags
{
	Filled = 0x01,
	AllowColorMath = 0x02,
};
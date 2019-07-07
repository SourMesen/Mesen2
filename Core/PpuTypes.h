#pragma once
#include "stdafx.h"

struct SpriteInfo
{
	uint8_t Index;
	int16_t X;
	uint8_t Y;
	bool HorizontalMirror;
	bool VerticalMirror;
	uint8_t Priority;

	uint8_t TileColumn;
	uint8_t TileRow;
	uint8_t Palette;
	bool UseSecondTable;
	uint8_t LargeSprite;
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
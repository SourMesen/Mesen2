#pragma once
#include "pch.h"
#include "Shared/BaseState.h"

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

	bool IsVisible(uint16_t scanline, bool interlace)
	{
		if(X != -256 && (X + Width <= 0 || X > 255)) {
			//Sprite is not visible (and must be ignored for time/range flag calculations)
			//Sprites at X=-256 are always used when considering Time/Range flag calculations, but not actually drawn.
			return false;
		}

		uint16_t endY = Y + (interlace ? (Height >> 1) : Height);
		return (
			(scanline >= Y && scanline < endY) ||
			((uint8_t)endY < Y && scanline < (uint8_t)endY) //wrap-around occurs after 256 scanlines
		);
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
};

struct LayerConfig
{
	uint16_t TilemapAddress = 0;
	uint16_t ChrAddress = 0;

	uint16_t HScroll = 0;
	uint16_t VScroll = 0;

	bool DoubleWidth = false;
	bool DoubleHeight = false;

	bool LargeTiles = false;
};

struct Mode7Config
{
	int16_t Matrix[4] = {};

	int16_t HScroll = 0;
	int16_t VScroll = 0;
	int16_t CenterX = 0;
	int16_t CenterY = 0;

	uint8_t ValueLatch = 0;
	
	bool LargeMap = false;
	bool FillWithTile0 = false;
	bool HorizontalMirroring = false;
	bool VerticalMirroring = false;

	//Holds the scroll values at the start of a scanline for the entire scanline
	int16_t HScrollLatch = 0;
	int16_t VScrollLatch = 0;
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

struct SnesPpuState : public BaseState
{
	uint16_t Cycle = 0;
	uint16_t Scanline = 0;
	uint16_t HClock = 0;
	uint32_t FrameCount = 0;

	bool ForcedBlank = false;
	uint8_t ScreenBrightness = 0;

	Mode7Config Mode7 = {};

	uint8_t BgMode = 0;
	bool Mode1Bg3Priority = false;

	uint8_t MainScreenLayers = 0;
	uint8_t SubScreenLayers = 0;
	LayerConfig Layers[4] = {};

	WindowConfig Window[2] = {};
	WindowMaskLogic MaskLogic[6] = {};
	bool WindowMaskMain[5] = {};
	bool WindowMaskSub[5] = {};

	uint16_t VramAddress = 0;
	uint8_t VramIncrementValue = 0;
	uint8_t VramAddressRemapping = 0;
	bool VramAddrIncrementOnSecondReg = false;
	uint16_t VramReadBuffer = 0;

	uint8_t Ppu1OpenBus = 0;
	uint8_t Ppu2OpenBus = 0;

	uint8_t CgramAddress = 0;
	uint8_t InternalCgramAddress = 0;
	uint8_t CgramWriteBuffer = 0;
	bool CgramAddressLatch = false;

	uint8_t MosaicSize = 0;
	uint8_t MosaicEnabled = 0;

	uint16_t OamRamAddress = 0;
	uint16_t InternalOamAddress = 0;

	uint8_t OamMode = 0;
	uint16_t OamBaseAddress = 0;
	uint16_t OamAddressOffset = 0;
	bool EnableOamPriority = false;

	bool ExtBgEnabled = false;
	bool HiResMode = false;
	bool ScreenInterlace = false;
	bool ObjInterlace = false;
	bool OverscanMode = false;
	bool DirectColorMode = false;

	ColorWindowMode ColorMathClipMode = ColorWindowMode::Never;
	ColorWindowMode ColorMathPreventMode = ColorWindowMode::Never;
	bool ColorMathAddSubscreen = false;
	uint8_t ColorMathEnabled = 0;
	bool ColorMathSubtractMode = false;
	bool ColorMathHalveResult = false;
	uint16_t FixedColor = 0;
};


enum PixelFlags
{
	AllowColorMath = 0x80,
};
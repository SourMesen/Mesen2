#pragma once
#include "stdafx.h"
#include "PpuTypes.h"

class Console;
class InternalRegisters;

struct SpriteInfo
{
	int16_t X;
	bool HorizontalMirror;
	bool VerticalMirror;
	uint8_t Priority;

	uint8_t TileColumn;
	uint8_t TileRow;
	uint8_t Palette;
	bool UseSecondTable;
	uint8_t LargeSprite;
};

class Ppu
{
public:
	constexpr static uint32_t SpriteRamSize = 544;
	constexpr static uint32_t CgRamSize = 512;
	constexpr static uint32_t VideoRamSize = 0x10000;

private:
	constexpr static int SpriteLayerIndex = 4;
	constexpr static int ColorWindowIndex = 5;

	constexpr static const uint8_t _oamSizes[8][2][2] = {
		{ { 1, 1 }, { 2, 2 } }, //8x8 + 16x16
		{ { 1, 1 }, { 4, 4 } }, //8x8 + 32x32
		{ { 1, 1 }, { 8, 8 } }, //8x8 + 64x64
		{ { 2, 2 }, { 4, 4 } }, //16x16 + 32x32
		{ { 2, 2 }, { 8, 8 } }, //16x16 + 64x64
		{ { 4, 4 }, { 8, 8 } }, //32x32 + 64x64
		{ { 2, 4 }, { 4, 8 } }, //16x32 + 32x64
		{ { 2, 4 }, { 4, 4 } }  //16x32 + 32x32
	};

	shared_ptr<Console> _console;
	shared_ptr<InternalRegisters> _regs;

	bool _forcedVblank = false;
	uint8_t _screenBrightness = 0;

	uint16_t _cycle = 0;
	uint16_t _scanline = 0;
	uint32_t _frameCount = 0;
	
	uint8_t _bgMode = 0;
	bool _mode1Bg3Priority = false;
	
	uint8_t _mainScreenLayers = 0;
	uint8_t _subScreenLayers = 0;
	LayerConfig _layerConfig[4];

	WindowConfig _window[2];
	WindowMaskLogic _maskLogic[6];
	bool _windowMaskMain[5];
	bool _windowMaskSub[5];
	
	uint8_t *_vram;
	uint16_t _vramAddress;
	uint8_t _vramIncrementValue;
	uint8_t _vramAddressRemapping;
	bool _vramAddrIncrementOnSecondReg;
	uint8_t _vramReadBuffer = 0;
	
	uint16_t _cgramAddress;
	uint8_t _cgram[Ppu::CgRamSize];

	uint16_t *_outputBuffers[2];

	SpriteInfo _sprites[32] = {};
	uint8_t _spriteCount = 0;
	uint8_t _spritePriority[256] = {};
	uint8_t _spritePalette[256] = {};
	uint16_t _spritePixels[256] = {};

	uint16_t _pixelsDrawn = 0;
	uint16_t _subPixelsDrawn = 0;

	uint8_t _rowPixelFlags[256];
	uint16_t *_currentBuffer;

	bool _subScreenFilled[256];
	uint16_t _subScreenBuffer[256];

	uint16_t _mosaicColor[256] = {};
	uint8_t _mosaicSize = 0;
	uint8_t _mosaicEnabled = 0;
	uint16_t _mosaicStartScanline = 0;

	uint8_t _oamMode = 0;
	uint16_t _oamBaseAddress = 0;
	uint16_t _oamAddressOffset = 0;

	uint8_t _oamRam[Ppu::SpriteRamSize];
	uint16_t _oamRamAddress = 0;
	bool _enableOamPriority = false;
	
	uint16_t _internalOamAddress = 0;
	uint8_t _oamWriteBuffer = 0;

	bool _timeOver = false;
	bool _rangeOver = false;

	bool _directColorMode = false;

	ColorWindowMode _colorMathClipMode = ColorWindowMode::Never;
	ColorWindowMode _colorMathPreventMode = ColorWindowMode::Never;
	bool _colorMathAddSubscreen = false;
	uint8_t _colorMathEnabled = 0;
	bool _colorMathSubstractMode = false;
	bool _colorMathHalveResult = false;
	uint16_t _fixedColor = 0;

	uint8_t _hvScrollLatchValue = 0;
	uint8_t _hScrollLatchValue = 0;

	uint16_t _horizontalLocation = 0;
	bool _horizontalLocToggle = false;
	uint16_t _verticalLocation = 0;
	bool _verticalLocationToggle = false;
	bool _locationLatched = false;

	uint16_t _mode7MatrixA = 0;
	uint16_t _mode7MatrixB = 0;
	uint8_t _mode7Latch = 0;

	void EvaluateNextLineSprites();
	
	template<uint8_t priority, bool forMainScreen>
	void RenderSprites();

	template<bool forMainScreen>
	void RenderMode0();

	template<bool forMainScreen>
	void RenderMode1();

	template<bool forMainScreen>
	void RenderMode2();

	template<bool forMainScreen>
	void RenderMode3();

	template<bool forMainScreen>
	void RenderMode4();

	void RenderScanline();

	template<bool forMainScreen>
	void RenderBgColor();

	template<uint8_t layerIndex, uint8_t bpp, bool processHighPriority, bool forMainScreen, uint16_t basePaletteOffset = 0>
	void RenderTilemap();

	template<uint8_t layerIndex, uint8_t bpp, bool processHighPriority, bool forMainScreen, bool largeTiles, uint16_t basePaletteOffset, uint8_t activeWindowCount, bool applyMosaic>
	void RenderTilemap();

	void ApplyColorMath();
	void ApplyBrightness();

	template<uint8_t layerIndex>
	bool ProcessMaskWindow(uint8_t activeWindowCount, int x);
	void ProcessWindowMaskSettings(uint8_t value, uint8_t offset);

	void SendFrame();

public:
	Ppu(shared_ptr<Console> console);
	~Ppu();

	uint32_t GetFrameCount();
	PpuState GetState();

	void Exec();

	uint8_t* GetVideoRam();
	uint8_t* GetCgRam();
	uint8_t* GetSpriteRam();

	void LatchLocationValues();

	uint8_t Read(uint16_t addr);
	void Write(uint32_t addr, uint8_t value);
};
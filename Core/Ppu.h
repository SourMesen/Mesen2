#pragma once
#include "stdafx.h"
#include "PpuTypes.h"
#include "../Utilities/ISerializable.h"
#include "../Utilities/Timer.h"

class Console;
class InternalRegisters;
class MemoryManager;
class Spc;
class EmuSettings;

class Ppu : public ISerializable
{
public:
	constexpr static uint32_t SpriteRamSize = 544;
	constexpr static uint32_t CgRamSize = 512;
	constexpr static uint32_t VideoRamSize = 0x10000;

private:
	constexpr static int SpriteLayerIndex = 4;
	constexpr static int ColorWindowIndex = 5;

	Console* _console;
	InternalRegisters* _regs;
	MemoryManager* _memoryManager;
	Spc* _spc;
	EmuSettings* _settings;

	//Temporary data used for the tilemap/tile fetching
	LayerData _layerData[4] = {};
	uint16_t _hOffset = 0;
	uint16_t _vOffset = 0;
	uint16_t _fetchBgStart = 0;
	uint16_t _fetchBgEnd = 0;

	//Temporary data used by the sprite evaluation/fetching
	SpriteInfo _currentSprite = {};
	uint8_t _oamEvaluationIndex = 0;
	uint8_t _oamTimeIndex = 0;
	uint16_t _fetchSpriteStart = 0;
	uint16_t _fetchSpriteEnd = 0;
	uint16_t _spriteEvalStart = 0;
	uint16_t _spriteEvalEnd = 0;
	bool _spriteFetchingDone = false;
	uint8_t _spriteIndexes[32] = {};
	uint8_t _spriteCount = 0;
	uint8_t _spriteTileCount = 0;
	bool _hasSpritePriority[4] = {};

	uint16_t _scanline = 0;
	uint32_t _frameCount = 0;

	uint16_t _vblankStartScanline;
	uint16_t _vblankEndScanline;
	uint16_t _baseVblankEndScanline;
	uint16_t _adjustedVblankEndScanline;
	uint16_t _nmiScanline;
	bool _overclockEnabled;

	uint8_t _oddFrame = 0;

	PpuState _state;

	uint16_t _drawStartX = 0;
	uint16_t _drawEndX = 0;
	
	uint16_t *_vram = nullptr;
	uint16_t _cgram[Ppu::CgRamSize >> 1] = {};
	uint8_t _oamRam[Ppu::SpriteRamSize] = {};

	uint16_t *_outputBuffers[2] = {};
	uint16_t *_currentBuffer = nullptr;
	bool _useHighResOutput = false;
	bool _interlacedFrame = false;

	uint8_t _mainScreenFlags[256] = {};
	uint16_t _mainScreenBuffer[256] = {};

	uint8_t _subScreenPriority[256] = {};
	uint16_t _subScreenBuffer[256] = {};

	uint32_t _mosaicColor[4] = {};
	uint32_t _mosaicPriority[4] = {};
	uint16_t _mosaicScanlineCounter = 0;
	
	uint16_t _internalOamAddress = 0;
	uint8_t _oamWriteBuffer = 0;

	bool _timeOver = false;
	bool _rangeOver = false;

	uint8_t _hvScrollLatchValue = 0;
	uint8_t _hScrollLatchValue = 0;

	uint16_t _horizontalLocation = 0;
	bool _horizontalLocToggle = false;
	uint16_t _verticalLocation = 0;
	bool _verticalLocationToggle = false;
	bool _locationLatched = false;
	bool _latchRequest = false;
	uint16_t _latchRequestX = 0;
	uint16_t _latchRequestY = 0;

	Timer _frameSkipTimer;
	bool _skipRender = false;
	uint8_t _configVisibleLayers = 0xFF;

	uint8_t _spritePriority[256] = {};
	uint8_t _spritePalette[256] = {};
	uint8_t _spriteColors[256] = {};
	uint8_t _spritePriorityCopy[256] = {};
	uint8_t _spritePaletteCopy[256] = {};
	uint8_t _spriteColorsCopy[256] = {};

	void RenderSprites(const uint8_t priorities[4]);

	template<bool hiResMode>
	void GetTilemapData(uint8_t layerIndex, uint8_t columnIndex);

	template<bool hiResMode, uint8_t bpp, bool secondTile = false>
	void GetChrData(uint8_t layerIndex, uint8_t column, uint8_t plane);

	void GetHorizontalOffsetByte(uint8_t columnIndex);
	void GetVerticalOffsetByte(uint8_t columnIndex);
	void FetchTileData();

	void RenderMode0();
	void RenderMode1();
	void RenderMode2();
	void RenderMode3();
	void RenderMode4();
	void RenderMode5();
	void RenderMode6();
	void RenderMode7();

	void RenderBgColor();

	template<uint8_t layerIndex, uint8_t bpp, uint8_t normalPriority, uint8_t highPriority, uint16_t basePaletteOffset = 0>
	__forceinline void RenderTilemap();
	
	template<uint8_t layerIndex, uint8_t bpp, uint8_t normalPriority, uint8_t highPriority, uint16_t basePaletteOffset, bool hiResMode>
	__forceinline void RenderTilemap();

	template<uint8_t layerIndex, uint8_t bpp, uint8_t normalPriority, uint8_t highPriority, uint16_t basePaletteOffset, bool hiResMode, bool applyMosaic>
	__forceinline void RenderTilemap();

	template<uint8_t layerIndex, uint8_t bpp, uint8_t normalPriority, uint8_t highPriority, uint16_t basePaletteOffset, bool hiResMode, bool applyMosaic, bool directColorMode>
	void RenderTilemap();

	template<uint8_t bpp, bool directColorMode, uint8_t basePaletteOffset>
	__forceinline uint16_t GetRgbColor(uint8_t paletteIndex, uint8_t colorIndex);

	__forceinline bool IsRenderRequired(uint8_t layerIndex);

	template<uint8_t bpp>
	__forceinline uint8_t GetTilePixelColor(const uint16_t chrData[4], const uint8_t shift);

	template<uint8_t layerIndex, uint8_t normalPriority, uint8_t highPriority>
	__forceinline void RenderTilemapMode7();

	template<uint8_t layerIndex, uint8_t normalPriority, uint8_t highPriority, bool applyMosaic>
	__forceinline void RenderTilemapMode7();

	template<uint8_t layerIndex, uint8_t normalPriority, uint8_t highPriority, bool applyMosaic, bool directColorMode>
	void RenderTilemapMode7();

	__forceinline void DrawMainPixel(uint8_t x, uint16_t color, uint8_t flags);
	__forceinline void DrawSubPixel(uint8_t x, uint16_t color, uint8_t priority);

	void ApplyColorMath();
	void ApplyColorMathToPixel(uint16_t &pixelA, uint16_t pixelB, int x, bool isInsideWindow);
	
	template<bool forMainScreen>
	void ApplyBrightness();

	void ConvertToHiRes();
	void ApplyHiResMode();

	template<uint8_t layerIndex>
	bool ProcessMaskWindow(uint8_t activeWindowCount, int x);

	void ProcessWindowMaskSettings(uint8_t value, uint8_t offset);

	void UpdateVramReadBuffer();
	uint16_t GetVramAddress();

	void SendFrame();

	bool IsDoubleHeight();
	bool IsDoubleWidth();

	void EvaluateNextLineSprites();
	void FetchSpriteData();
	__forceinline void FetchSpritePosition(uint16_t oamAddress);
	void FetchSpriteAttributes(uint16_t oamAddress);
	void FetchSpriteTile(bool secondCycle);

	void UpdateOamAddress();
	uint16_t GetOamAddress();
	
	void RandomizeState();

public:
	Ppu(Console* console);
	virtual ~Ppu();

	void PowerOn();
	void Reset();

	void RenderScanline();

	uint32_t GetFrameCount();
	uint16_t GetRealScanline();
	uint16_t GetVblankEndScanline();
	uint16_t GetScanline();
	uint16_t GetCycle();
	uint16_t GetNmiScanline();
	uint16_t GetVblankStart();

	PpuState GetState();
	void GetState(PpuState &state, bool returnPartialState);

	bool ProcessEndOfScanline(uint16_t hClock);
	void UpdateSpcState();
	void UpdateNmiScanline();
	uint16_t GetLastScanline();

	bool IsHighResOutput();
	uint16_t* GetScreenBuffer();
	uint16_t* GetPreviousScreenBuffer();
	uint8_t* GetVideoRam();
	uint8_t* GetCgRam();
	uint8_t* GetSpriteRam();

	void SetLocationLatchRequest(uint16_t x, uint16_t y);
	void ProcessLocationLatchRequest();
	void LatchLocationValues();
	
	uint8_t Read(uint16_t addr);
	void Write(uint32_t addr, uint8_t value);

	void Serialize(Serializer &s) override;
};
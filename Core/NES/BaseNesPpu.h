#pragma once 
#include "pch.h"
#include "NES/INesMemoryHandler.h"
#include "Utilities/ISerializable.h"
#include "NES/NesTypes.h"

enum class ConsoleRegion;

class Emulator;
class BaseMapper;
class SnesControlManager;
class NesConsole;
class EmuSettings;

class BaseNesPpu : public INesMemoryHandler, public ISerializable
{
protected:
	uint64_t _masterClock = 0;
	uint32_t _cycle = 0;
	int16_t _scanline = 0;
	bool _emulatorBgEnabled = false;
	bool _emulatorSpritesEnabled = false;
	//16
	uint16_t _videoRamAddr = 0;
	uint16_t _tmpVideoRamAddr = 0;
	uint16_t _highBitShift = 0;
	uint16_t _lowBitShift = 0;
	uint8_t _masterClockDivider = 0;
	uint8_t _spriteRamAddr = 0;
	uint8_t _openBus = 0;
	uint8_t _xScroll = 0;
	bool _enableOamDecay = false;
	bool _needStateUpdate = false;
	bool _renderingEnabled = false;
	bool _prevRenderingEnabled = false;
	//32
	bool _sprite0Visible = false;
	uint8_t _spriteCount = 0;
	uint8_t _secondaryOamAddr = 0;
	uint8_t _oamCopybuffer = 0;
	bool _spriteInRange = false;
	bool _sprite0Added = false;
	uint8_t _spriteAddrH = 0;
	uint8_t _spriteAddrL = 0;
	uint8_t _overflowBugCounter = 0;
	bool _oamCopyDone = false;
	uint16_t _minimumDrawBgCycle = 0;
	uint16_t _minimumDrawSpriteCycle = 0;
	uint16_t _minimumDrawSpriteStandardCycle = 0;
	//48
	BaseMapper* _mapper = nullptr;
	uint16_t* _currentOutputBuffer = nullptr;
	////////////////////////
	//64 : end of cache line
	////////////////////////
	uint8_t _paletteRam[0x20] = {};
	uint8_t _secondarySpriteRam[0x20] = {};
	////////////////////////
	//128 : end of cache line
	////////////////////////
	TileInfo _tile = {};
	uint16_t _ppuBusAddress = 0;
	uint16_t _nmiScanline = 0;
	uint8_t _currentTilePalette = 0;
	uint8_t _previousTilePalette = 0;
	uint16_t _intensifyColorBits = 0;
	uint8_t _paletteRamMask = 0;
	uint8_t _updateVramAddrDelay = 0;
	//144
	uint32_t _spriteIndex = 0;
	int32_t _lastUpdatedPixel = 0;
	uint32_t _frameCount = 0;
	uint16_t _updateVramAddr = 0;
	bool _preventVblFlag = false;
	bool _writeToggle = false; //not used in rendering
	//160
	NesSpriteInfo* _lastSprite = nullptr; //used by HD ppu
	NesConsole* _console = nullptr;
	//176
	PpuControlFlags _control = {}; // 8 bytes
	PpuMaskFlags _mask = {}; // 8 bytes
  ////////////////////////
	//192 : end of cache line
	////////////////////////
	uint8_t _spriteRam[0x100] = {};
	////////////////////////
	//448 : end of cache line
	////////////////////////
	bool _hasSprite[257] = {};
	//705
	NesSpriteInfo _spriteTiles[64] = {};

	Emulator* _emu = nullptr;
	EmuSettings* _settings = nullptr;
	uint16_t* _outputBuffers[2] = {};

	ConsoleRegion _region = {};
	uint16_t _standardVblankEnd = 0;
	uint16_t _standardNmiScanline = 0;
	uint16_t _vblankEnd = 0;
	uint16_t _palSpriteEvalScanline = 0;

	bool _needVideoRamIncrement = false;
	bool _allowFullPpuAccess = false;

	uint8_t _memoryReadBuffer = 0;
	PPUStatusFlags _statusFlags = {};

	uint8_t _firstVisibleSpriteAddr = 0; //For extra sprites
	uint8_t _lastVisibleSpriteAddr = 0; //For extra sprites

	uint32_t _ignoreVramRead = 0;
	int32_t _openBusDecayStamp[8] = {};

	uint64_t _oamDecayCycles[0x40] = {};
	bool _corruptOamRow[32] = {};
	
	bool IsRenderingEnabled();
	void UpdateGrayscaleAndIntensifyBits();
	void UpdateColorBitMasks();
	void UpdateMinimumDrawCycles();

public:
	virtual void Reset(bool softReset) = 0;
	virtual void Run(uint64_t runTo) = 0;

	uint32_t GetFrameCount() { return _frameCount; }
	uint32_t GetCurrentCycle() { return _cycle; }
	int32_t GetCurrentScanline() { return _scanline; }
	int32_t GetScanlineCount() { return _vblankEnd + 2; }
	uint32_t GetFrameCycle() { return ((_scanline + 1) * 341) + _cycle; }

	virtual uint16_t* GetScreenBuffer(bool previousBuffer, bool processGrayscaleEmphasisBits = false) = 0;
	virtual void UpdateTimings(ConsoleRegion region, bool overclockAllowed = true) = 0;

	void GetState(NesPpuState& state);
	void SetState(NesPpuState& state);

	uint16_t GetCurrentBgColor();

	uint8_t ReadPaletteRam(uint16_t addr);
	void WritePaletteRam(uint16_t addr, uint8_t value);

	void DebugSendFrame();

	virtual PpuModel GetPpuModel() = 0;
	virtual uint32_t GetPixelBrightness(uint8_t x, uint8_t y) = 0;

	virtual void GetMemoryRanges(MemoryRanges& ranges) override {}
	virtual uint8_t ReadRam(uint16_t addr) override { return 0; }
	virtual void WriteRam(uint16_t addr, uint8_t value) override {}

	virtual void Serialize(Serializer& s) override {}
};

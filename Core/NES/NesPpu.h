#pragma once

#include "stdafx.h"
#include "Utilities/ISerializable.h"
#include "NES/NesTypes.h"
#include "NES/BaseNesPpu.h"
#include "NES/BaseMapper.h"
#include "NES/Debugger/NesDebuggerTypes.h"
#include "NES/INesMemoryHandler.h"
#include "MemoryOperationType.h"

enum class ConsoleRegion;

class Emulator;
class ControlManager;
class NesConsole;
class EmuSettings;

enum PPURegisters
{
	Control = 0x00,
	Mask = 0x01,
	Status = 0x02,
	SpriteAddr = 0x03,
	SpriteData = 0x04,
	ScrollOffsets = 0x05,
	VideoMemoryAddr = 0x06,
	VideoMemoryData = 0x07,
	SpriteDMA = 0x4014,
};

template<class T>
class NesPpu : public BaseNesPpu
{
private:
	static constexpr int32_t OamDecayCycleCount = 3000;

protected:
	uint16_t* _currentOutputBuffer;
	uint8_t _paletteRAM[0x20];

	NesConsole* _console;
	BaseMapper* _mapper;
	Emulator* _emu;
	EmuSettings* _settings;

	PPUState _state;
	uint64_t _masterClock;
	uint8_t _masterClockDivider;
	uint8_t _memoryReadBuffer;

	uint8_t _spriteRAM[0x100];
	uint8_t _secondarySpriteRAM[0x20];
	bool _hasSprite[257];

	uint16_t* _outputBuffers[2];

	ConsoleRegion _region;
	uint16_t _standardVblankEnd;
	uint16_t _standardNmiScanline;
	uint16_t _vblankEnd;
	uint16_t _nmiScanline;
	uint16_t _palSpriteEvalScanline;

	PPUControlFlags _flags;
	PPUStatusFlags _statusFlags;

	uint16_t _intensifyColorBits;
	uint8_t _paletteRamMask;
	int32_t _lastUpdatedPixel;

	NesSpriteInfo* _lastSprite; //used by HD ppu

	uint16_t _ppuBusAddress;
	TileInfo _currentTile;
	TileInfo _nextTile;
	TileInfo _previousTile;

	NesSpriteInfo _spriteTiles[64];
	uint32_t _spriteCount;
	uint32_t _secondaryOAMAddr;
	bool _sprite0Visible;

	uint8_t _firstVisibleSpriteAddr;
	uint8_t _lastVisibleSpriteAddr;
	uint32_t _spriteIndex;

	uint8_t _openBus;
	int32_t _openBusDecayStamp[8];
	uint32_t _ignoreVramRead;

	uint8_t _oamCopybuffer;
	bool _spriteInRange;
	bool _sprite0Added;
	uint8_t _spriteAddrH;
	uint8_t _spriteAddrL;
	bool _oamCopyDone;
	uint8_t _overflowBugCounter;

	bool _needStateUpdate;
	bool _renderingEnabled;
	bool _prevRenderingEnabled;
	bool _preventVblFlag;

	uint16_t _updateVramAddr;
	uint8_t _updateVramAddrDelay;

	uint32_t _minimumDrawBgCycle;
	uint32_t _minimumDrawSpriteCycle;
	uint32_t _minimumDrawSpriteStandardCycle;

	uint64_t _oamDecayCycles[0x40];
	bool _enableOamDecay;
	bool _corruptOamRow[32];

	void UpdateStatusFlag();

	void SetControlRegister(uint8_t value);
	void SetMaskRegister(uint8_t value);

	__forceinline bool IsRenderingEnabled();

	void ProcessTmpAddrScrollGlitch(uint16_t normalAddr, uint16_t value, uint16_t mask);

	void SetOpenBus(uint8_t mask, uint8_t value);
	uint8_t ApplyOpenBus(uint8_t mask, uint8_t value);

	void ProcessStatusRegOpenBus(uint8_t& openBusMask, uint8_t& returnValue);

	__forceinline void UpdateVideoRamAddr();
	__forceinline void IncVerticalScrolling();
	__forceinline void IncHorizontalScrolling();
	__forceinline uint16_t GetNameTableAddr();
	__forceinline uint16_t GetAttributeAddr();

	void ProcessScanlineFirstCycle();
	__forceinline void ProcessScanlineImpl();
	__forceinline void ProcessSpriteEvaluation();

	void BeginVBlank();
	void TriggerNmi();

	__forceinline void LoadTileInfo();
	void LoadSprite(uint8_t spriteY, uint8_t tileIndex, uint8_t attributes, uint8_t spriteX, bool extraSprite);
	void LoadSpriteTileInfo();
	void LoadExtraSprites();
	__forceinline void ShiftTileRegisters();

	__forceinline uint8_t ReadSpriteRam(uint8_t addr);
	__forceinline void WriteSpriteRam(uint8_t addr, uint8_t value);

	void SetOamCorruptionFlags();
	void ProcessOamCorruption();

	void UpdateMinimumDrawCycles();

	__forceinline uint8_t GetPixelColor();

	void UpdateGrayscaleAndIntensifyBits();
	virtual void SendFrame();

	void SendFrameVsDualSystem();

	void UpdateState();

	void UpdateApuStatus();

	PPURegisters GetRegisterID(uint16_t addr)
	{
		if(addr == 0x4014) {
			return PPURegisters::SpriteDMA;
		} else {
			return (PPURegisters)(addr & 0x07);
		}
	}

	__forceinline void SetBusAddress(uint16_t addr);
	__forceinline uint8_t ReadVram(uint16_t addr, MemoryOperationType type = MemoryOperationType::PpuRenderingRead);
	__forceinline void WriteVram(uint16_t addr, uint8_t value);

	void Serialize(Serializer& s) override;

public:
	NesPpu(NesConsole* console);
	virtual ~NesPpu();

	void Reset() override;

	void DebugSendFrame();
	uint16_t* GetScreenBuffer(bool previousBuffer) override;
	void DebugCopyOutputBuffer(uint16_t* target);
	void DebugUpdateFrameBuffer(bool toGrayscale);
	void GetState(NesPpuState& state);
	void SetState(NesPpuState& state);

	void GetMemoryRanges(MemoryRanges& ranges) override
	{
		ranges.AddHandler(MemoryOperation::Read, 0x2000, 0x3FFF);
		ranges.AddHandler(MemoryOperation::Write, 0x2000, 0x3FFF);
		ranges.AddHandler(MemoryOperation::Write, 0x4014);
	}

	PpuModel GetPpuModel() override;

	uint8_t ReadPaletteRAM(uint16_t addr);
	void WritePaletteRAM(uint16_t addr, uint8_t value);

	uint8_t ReadRam(uint16_t addr) override;
	uint8_t PeekRam(uint16_t addr) override;
	void WriteRam(uint16_t addr, uint8_t value) override;

	void SetRegion(ConsoleRegion region) override;
	double GetOverclockRate();

	__forceinline void Exec();
	void Run(uint64_t runTo) override;


	PPUControlFlags GetControlFlags()
	{
		return _flags;
	}

	uint8_t* GetSpriteRam();

	uint8_t* GetSecondarySpriteRam()
	{
		return _secondarySpriteRAM;
	}

	uint32_t GetPixelBrightness(uint8_t x, uint8_t y) override;
	uint16_t GetCurrentBgColor();

	uint16_t GetPixel(uint8_t x, uint8_t y)
	{
		return _currentOutputBuffer[y << 8 | x];
	}
};

template<class T>
void NesPpu<T>::Run(uint64_t runTo)
{
	do {
		//Always need to run at least once, check condition at the end of the loop (slightly faster)
		Exec();
		_masterClock += _masterClockDivider;
	} while(_masterClock + _masterClockDivider <= runTo);
}

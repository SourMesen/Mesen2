#pragma once

#include "stdafx.h"
#include "Utilities/ISerializable.h"
#include "NES/NesTypes.h"
#include "NES/Debugger/NesDebuggerTypes.h"
#include "NES/INesMemoryHandler.h"
#include "MemoryOperationType.h"

enum class ConsoleRegion;

class Emulator;
class BaseMapper;
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

class NesPpu : public INesMemoryHandler, public ISerializable
{
	protected:
		NesConsole* _console;
		Emulator* _emu;
		EmuSettings* _settings;

		PPUState _state;
		int32_t _scanline;
		uint32_t _cycle;
		uint32_t _frameCount;
		uint64_t _masterClock;
		uint8_t _masterClockDivider;
		uint8_t _memoryReadBuffer;

		uint8_t _paletteRAM[0x20];

		uint8_t _spriteRAM[0x100];
		uint8_t _secondarySpriteRAM[0x20];
		bool _hasSprite[257];

		uint16_t *_currentOutputBuffer;
		uint16_t *_outputBuffers[2];

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

		NesSpriteInfo *_lastSprite; //used by HD ppu

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

		bool _emulatorBgEnabled = true;
		bool _emulatorSpritesEnabled = true;

		uint32_t _minimumDrawBgCycle;
		uint32_t _minimumDrawSpriteCycle;
		uint32_t _minimumDrawSpriteStandardCycle;

		uint64_t _oamDecayCycles[0x40];
		bool _enableOamDecay;
		bool _corruptOamRow[32];

		void UpdateStatusFlag();

		void SetControlRegister(uint8_t value);
		void SetMaskRegister(uint8_t value);

		bool IsRenderingEnabled();

		void ProcessTmpAddrScrollGlitch(uint16_t normalAddr, uint16_t value, uint16_t mask);

		void SetOpenBus(uint8_t mask, uint8_t value);
		uint8_t ApplyOpenBus(uint8_t mask, uint8_t value);

		void ProcessStatusRegOpenBus(uint8_t & openBusMask, uint8_t & returnValue);

		void UpdateVideoRamAddr();
		void IncVerticalScrolling();
		void IncHorizontalScrolling();
		uint16_t GetNameTableAddr();
		uint16_t GetAttributeAddr();

		__forceinline void ProcessScanline();
		__forceinline void ProcessSpriteEvaluation();

		void BeginVBlank();
		void TriggerNmi();

		void LoadTileInfo();
		void LoadSprite(uint8_t spriteY, uint8_t tileIndex, uint8_t attributes, uint8_t spriteX, bool extraSprite);
		void LoadSpriteTileInfo();
		void LoadExtraSprites();
		__forceinline void ShiftTileRegisters();

		__forceinline uint8_t ReadSpriteRam(uint8_t addr);
		__forceinline void WriteSpriteRam(uint8_t addr, uint8_t value);
		
		void SetOamCorruptionFlags();
		void ProcessOamCorruption();

		void UpdateMinimumDrawCycles();

		uint8_t GetPixelColor();
		__forceinline virtual void DrawPixel();
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
		static constexpr int32_t ScreenWidth = 256;
		static constexpr int32_t ScreenHeight = 240;
		static constexpr int32_t PixelCount = 256*240;
		static constexpr int32_t OutputBufferSize = 256*240*2;
		static constexpr int32_t OamDecayCycleCount = 3000;

		NesPpu(NesConsole* console);
		virtual ~NesPpu();

		void Reset();

		void DebugSendFrame();
		uint16_t* GetScreenBuffer(bool previousBuffer);
		void DebugCopyOutputBuffer(uint16_t *target);
		void DebugUpdateFrameBuffer(bool toGrayscale);
		void GetState(NesPpuState &state);
		void SetState(NesPpuState &state);

		void GetMemoryRanges(MemoryRanges &ranges) override
		{
			ranges.AddHandler(MemoryOperation::Read, 0x2000, 0x3FFF);
			ranges.AddHandler(MemoryOperation::Write, 0x2000, 0x3FFF);
			ranges.AddHandler(MemoryOperation::Write, 0x4014);
		}

		PpuModel GetPpuModel();

		uint8_t ReadPaletteRAM(uint16_t addr);
		void WritePaletteRAM(uint16_t addr, uint8_t value);

		uint8_t ReadRam(uint16_t addr) override;
		uint8_t PeekRam(uint16_t addr) override;
		void WriteRam(uint16_t addr, uint8_t value) override;

		void SetRegion(ConsoleRegion region);
		double GetOverclockRate();
		
		void Exec();
		__forceinline void Run(uint64_t runTo);

		uint32_t GetFrameCount()
		{
			return _frameCount;
		}

		uint32_t GetFrameCycle()
		{
			return ((_scanline + 1) * 341) + _cycle;
		}

		PPUControlFlags GetControlFlags()
		{
			return _flags;
		}

		uint32_t GetCurrentCycle()
		{
			return _cycle;
		}

		int32_t GetCurrentScanline()
		{
			return _scanline;
		}
		
		uint8_t* GetSpriteRam();

		uint8_t* GetSecondarySpriteRam()
		{
			return _secondarySpriteRAM;
		}
		
		uint32_t GetPixelBrightness(uint8_t x, uint8_t y);
		uint16_t GetCurrentBgColor();

		uint16_t GetPixel(uint8_t x, uint8_t y)
		{
			return _currentOutputBuffer[y << 8 | x];
		}
};

void NesPpu::Run(uint64_t runTo)
{
	while(_masterClock + _masterClockDivider <= runTo) {
		Exec();
		_masterClock += _masterClockDivider;
	}
}

#pragma once
#include "stdafx.h"
#include "Shared/Emulator.h"
#include "PCE/PceTypes.h"
#include "PCE/PceConstants.h"

class PceConsole;

enum class PcePpuModeH
{
	Hds,
	Hdw,
	Hde,
	Hsw,
};

enum class PcePpuModeV
{
	Vds,
	Vdw,
	Vde,
	Vsw,
};

enum class PceVdcEvent
{
	None,
	LatchScrollY,
	LatchScrollX,
	HdsIrqTrigger,
	IncRcrCounter,
};

struct PceTileInfo
{
	uint16_t TileData[2];
	uint16_t TileAddr;
	uint8_t Palette;
};

struct PceSpriteInfo
{
	uint16_t TileData[4];
	int16_t X;
	uint16_t TileAddress;
	uint8_t Index;
	uint8_t Palette;
	bool HorizontalMirroring;
	bool ForegroundPriority;
	bool LoadSp23;
};

class PcePpu
{
private:
	PcePpuState _state = {};
	Emulator* _emu = nullptr;
	PceConsole* _console = nullptr;
	uint16_t* _vram = nullptr;
	uint16_t* _paletteRam = nullptr;
	uint16_t* _spriteRam = nullptr;

	uint16_t* _outBuffer[2] = {};
	uint16_t* _currentOutBuffer = nullptr;
	
	uint8_t _rowVceClockDivider[2][PceConstants::ScreenHeight] = {};
	uint8_t* _currentClockDividers = nullptr;

	uint16_t _rowBuffer[PceConstants::MaxScreenWidth] = {};

	uint16_t _xStart = 0;

	PcePpuModeH _hMode = PcePpuModeH::Hds;
	int16_t _hModeCounter = 0;
	
	PcePpuModeV _vMode = PcePpuModeV::Vds;
	int16_t _vModeCounter = 0;

	uint16_t _screenOffsetX = 0;
	bool _needRcrIncrement = false;
	bool _needBgScrollYInc = false;
	bool _needVertBlankIrq = false;
	bool _verticalBlankDone = false;

	uint8_t _spriteCount = 0;
	uint16_t _spriteRow = 0;
	PceSpriteInfo _sprites[16] = {};
	uint16_t _evalStartCycle = 0;
	uint16_t _evalEndCycle = 0;
	int16_t _evalLastCycle = 0;
	bool _hasSpriteOverflow = false;

	PceSpriteInfo _drawSprites[16] = {};
	uint8_t _drawSpriteCount = 0;
	bool _rowHasSprite0 = false;
	uint16_t _loadSpriteStart = 0;

	uint16_t _loadBgStart = 0;
	uint16_t _loadBgEnd = 0;
	int16_t _loadBgLastCycle = 0;
	uint8_t _tileCount = 0;
	PceTileInfo _tiles[100] = {};
	bool _allowVramAccess = false;

	bool _pendingMemoryRead = false;

	PceVdcEvent _nextEvent = PceVdcEvent::None;
	uint16_t _nextEventCounter = 0;

	template<uint16_t bitMask = 0xFFFF>
	void UpdateReg(uint16_t& reg, uint8_t value, bool msb)
	{
		if(msb) {
			reg = ((reg & 0xFF) | (value << 8)) & bitMask;
		} else {
			reg = ((reg & 0xFF00) | value) & bitMask;
		}
	}

	void LoadReadBuffer();
	void DrawScanline();
	void SendFrame();

	uint16_t DotsToClocks(int dots);
	void TriggerHdsIrqs();

	__declspec(noinline) void IncrementRcrCounter();
	__declspec(noinline) void IncScrollY();
	__declspec(noinline) void ProcessEndOfScanline();
	__declspec(noinline) void ProcessEndOfVisibleFrame();
	__declspec(noinline) void ProcessSatbTransfer();
	__declspec(noinline) void SetHorizontalMode(PcePpuModeH hMode);

	__declspec(noinline) void ProcessVdcEvents();
	__declspec(noinline) void ProcessEvent();

	__declspec(noinline) void ProcessHorizontalSyncStart();
	__declspec(noinline) void ProcessVerticalSyncStart();

	__forceinline uint8_t GetTilePixelColor(const uint16_t chrData[2], const uint8_t shift);
	__forceinline uint8_t GetSpritePixelColor(const uint16_t chrData[4], const uint8_t shift);

	__declspec(noinline) void ProcessSpriteEvaluation();
	__declspec(noinline) void LoadSpriteTiles();
	
	__declspec(noinline) void LoadBackgroundTiles();
	__declspec(noinline) void LoadBackgroundTilesWidth2(uint16_t end, uint16_t scrollOffset, uint16_t columnMask, uint16_t row);
	__declspec(noinline) void LoadBackgroundTilesWidth4(uint16_t end, uint16_t scrollOffset, uint16_t columnMask, uint16_t row);
	
	__forceinline void LoadBatEntry(uint16_t scrollOffset, uint16_t columnMask, uint16_t row);
	__forceinline void LoadTileDataCg0(uint16_t row);
	__forceinline void LoadTileDataCg1(uint16_t row);

	void WaitForVramAccess();
	bool IsVramAccessBlocked();

public:
	PcePpu(Emulator* emu, PceConsole* console);
	~PcePpu();

	PcePpuState& GetState();
	uint16_t* GetScreenBuffer();
	uint16_t* GetPreviousScreenBuffer();
	uint8_t* GetRowClockDividers() { return _currentClockDividers; }
	uint8_t* GetPreviousRowClockDividers() { return _currentClockDividers == _rowVceClockDivider[0] ? _rowVceClockDivider[1] : _rowVceClockDivider[0]; }

	uint16_t GetHClock() { return _state.HClock; }
	uint16_t GetScanline() { return _state.Scanline; }
	uint16_t* GetRowBuffer() { return _rowBuffer; }
	uint16_t GetFrameCount() { return _state.FrameCount; }

	void Exec();

	uint8_t ReadVdc(uint16_t addr);
	void WriteVdc(uint16_t addr, uint8_t value);

	uint8_t ReadVce(uint16_t addr);
	void WriteVce(uint16_t addr, uint8_t value);
};

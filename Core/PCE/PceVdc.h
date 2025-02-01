#pragma once
#include "pch.h"
#include "Shared/Emulator.h"
#include "Shared/MemoryType.h"
#include "PCE/PceTypes.h"
#include "PCE/PceConstants.h"
#include "Utilities/ISerializable.h"

class PceConsole;
class PceVce;
class PceVpc;

enum class PceVdcModeH
{
	Hds,
	Hdw,
	Hde,
	Hsw,
};

enum class PceVdcModeV
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

class PceVdc final : public ISerializable
{
private:
	PceVdcState _state = {};
	Emulator* _emu = nullptr;
	PceConsole* _console = nullptr;
	PceVce* _vce = nullptr;
	PceVpc* _vpc = nullptr;
	uint16_t* _vram = nullptr;
	uint16_t* _spriteRam = nullptr;
	
	uint16_t _rowBuffer[PceConstants::MaxScreenWidth] = {};

	uint16_t _vramOpenBus = 0;

	uint16_t _lastDrawHClock = 0;
	uint16_t _xStart = 0;

	PceVdcModeH _hMode = PceVdcModeH::Hds;
	int16_t _hModeCounter = 0;
	
	PceVdcModeV _vMode = PceVdcModeV::Vds;
	int16_t _vModeCounter = 0;

	uint16_t _screenOffsetX = 0;
	bool _needRcrIncrement = false;
	bool _needVCounterClock = false;
	bool _needVertBlankIrq = false;
	bool _verticalBlankDone = false;

	uint16_t _latchClockY = UINT16_MAX;
	uint16_t _latchClockX = UINT16_MAX;

	uint8_t _spriteCount = 0;
	uint16_t _spriteRow = 0;
	uint16_t _evalStartCycle = 0;
	uint16_t _evalEndCycle = 0;
	int16_t _evalLastCycle = 0;
	bool _hasSpriteOverflow = false;

	uint16_t _loadBgStart = 0;
	uint16_t _loadBgEnd = 0;
	int16_t _loadBgLastCycle = 0;
	uint8_t _tileCount = 0;
	bool _allowVramAccess = false;

	bool _pendingMemoryRead = false;
	bool _pendingMemoryWrite = false;
	int8_t _transferDelay = 0;

	bool _vramDmaRunning = false;
	bool _vramDmaReadCycle = false;
	uint16_t _vramDmaBuffer = 0;
	uint16_t _vramDmaPendingCycles = 0;

	PceVdcEvent _nextEvent = PceVdcEvent::None;
	uint16_t _nextEventCounter = 0;
	uint64_t _hSyncStartClock = 0;
	bool _allowDma = true;

	bool _isVdc2 = false;
	MemoryType _vramType = MemoryType::PceVideoRam;
	MemoryType _spriteRamType = MemoryType::PceSpriteRam;

	bool _xPosHasSprite[1024 + 32] = {};
	uint8_t _drawSpriteCount = 0;
	uint8_t _totalSpriteCount = 0;
	bool _rowHasSprite0 = false;
	uint16_t _loadSpriteStart = 0;
	PceSpriteInfo _sprites[64] = {};
	PceSpriteInfo _drawSprites[64] = {};
	PceTileInfo _tiles[100] = {};

	template<uint16_t bitMask = 0xFFFF>
	void UpdateReg(uint16_t& reg, uint8_t value, bool msb)
	{
		if(msb) {
			reg = ((reg & 0xFF) | (value << 8)) & bitMask;
		} else {
			reg = ((reg & 0xFF00) | value) & bitMask;
		}
	}

	void ProcessVramRead();
	void ProcessVramWrite();
	__noinline void ProcessVramAccesses();

	void QueueMemoryRead();
	void QueueMemoryWrite();
	void WaitForVramAccess();

	uint8_t GetClockDivider();
	uint16_t GetScanlineCount();
	uint16_t DotsToClocks(int dots);
	void TriggerHdsIrqs();

	__noinline void IncrementRcrCounter();
	__noinline void IncScrollY();
	__noinline void ProcessEndOfScanline();
	
	void TriggerDmaStart();
	__noinline void TriggerVerticalBlank();
	__noinline void ProcessSatbTransfer();
	__noinline void ProcessVramDmaTransfer();
	__noinline void SetVertMode(PceVdcModeV vMode);
	__noinline void SetHorizontalMode(PceVdcModeH hMode);
	void ClockVCounter();

	__noinline void ProcessVdcEvents();
	__noinline void ProcessEvent();

	__noinline void ProcessHorizontalSyncStart();
	__noinline void ProcessVerticalSyncStart();

	__forceinline uint8_t GetTilePixelColor(const uint16_t chr0, const uint16_t chr1, const uint8_t shift);
	__forceinline uint8_t GetSpritePixelColor(const uint16_t chrData[4], const uint8_t shift);

	__forceinline void ProcessSpriteEvaluation();
	__noinline void LoadSpriteTiles();

	bool IsDmaAllowed();
	
	template<bool skipRender>
	__forceinline void LoadBackgroundTiles();

	__noinline void LoadBackgroundTilesWidth2(uint16_t end, uint16_t scrollOffset, uint16_t columnMask, uint16_t row);
	__noinline void LoadBackgroundTilesWidth4(uint16_t end, uint16_t scrollOffset, uint16_t columnMask, uint16_t row);
	
	__forceinline void LoadBatEntry(uint16_t scrollOffset, uint16_t columnMask, uint16_t row);
	__forceinline void LoadTileDataCg0(uint16_t row);
	__forceinline void LoadTileDataCg1(uint16_t row);

	__forceinline uint16_t ReadVram(uint16_t addr);

	template<bool hasSprites, bool hasSprite0, bool skipRender> __forceinline void InternalDrawScanline();

public:
	PceVdc(Emulator* emu, PceConsole* console, PceVpc* vpc, PceVce* vce, bool isVdc2);
	~PceVdc();

	PceVdcState& GetState();

	uint16_t GetHClock() { return _state.HClock; }
	uint16_t GetScanline() { return _state.Scanline; }
	uint16_t* GetRowBuffer() { return _rowBuffer; }
	uint16_t GetFrameCount() { return _state.FrameCount; }

	void Exec();
	void DrawScanline();

	uint8_t ReadRegister(uint16_t addr);
	void WriteRegister(uint16_t addr, uint8_t value);

	void Serialize(Serializer& s) override;
};

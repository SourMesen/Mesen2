#pragma once
#include "stdafx.h"
#include "Shared/Emulator.h"
#include "PCE/PceTypes.h"
#include "PCE/PceConstants.h"

class PceConsole;

enum class PpuFetchMode
{
	None,
	BgFetch,
	SpriteFetch
};

enum class PcePpuModeH
{
	Hds,
	Hds_VerticalBlankIrq,
	Hds_ScrollYLatch,
	Hds_ScrollXLatch,
	Hdw,
	Hdw_RcrIrq,
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

	uint32_t _screenWidth = 256;
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

	struct PceSpriteInfo
	{
		uint16_t TileData[4];
		int16_t X;
		uint16_t TileAddress;
		uint8_t Index;
		uint8_t Palette;
		bool HorizontalMirroring;
		bool ForegroundPriority;
	};

	bool _inSpriteEval = false;
	uint8_t _spriteCount = 0;
	PceSpriteInfo _sprites[16] = {};
	uint16_t _evalStartCycle = 0;
	uint16_t _evalLastCycle = 0;
	bool _hasSpriteOverflow = false;

	PceSpriteInfo _drawSprites[16] = {};
	uint8_t _drawSpriteCount = 0;
	bool _rowHasSprite0 = false;
	uint16_t _loadSpriteStart = 0;

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

	void UpdateFrameTimings();
	
	uint16_t DotsToClocks(int dots);
	void TriggerHdsIrqs();

	__declspec(noinline) void IncrementRcrCounter();
	void IncScrollY();
	__declspec(noinline) void ProcessEndOfScanline();
	__declspec(noinline) void ProcessEndOfVisibleFrame();
	__declspec(noinline) void ProcessSatbTransfer();
	__declspec(noinline) void ProcessEvent();

	void ProcessSpriteEvaluation();
	void LoadSpriteTiles();

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

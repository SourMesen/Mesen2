#include "pch.h"
#include "PCE/PceVdc.h"
#include "PCE/PceVce.h"
#include "PCE/PceVpc.h"
#include "PCE/PceMemoryManager.h"
#include "PCE/PceConstants.h"
#include "PCE/PceConsole.h"
#include "Shared/EmuSettings.h"
#include "Shared/EventType.h"
#include "Shared/MessageManager.h"
#include "Utilities/Serializer.h"

PceVdc::PceVdc(Emulator* emu, PceConsole* console, PceVpc* vpc, PceVce* vce, bool isVdc2)
{
	_emu = emu;
	_console = console;
	_vpc = vpc;
	_vce = vce;

	_vram = new uint16_t[0x8000];
	_spriteRam = new uint16_t[0x100];

	console->InitializeRam(_vram, 0x10000);
	console->InitializeRam(_spriteRam, 0x200);

	//These values can't ever be 0, init them to a possible value
	_state.HvLatch.ColumnCount = 32;
	_state.HvReg.ColumnCount = 32;
	_state.HvLatch.RowCount = 32;
	_state.HvReg.RowCount = 32;
	_state.VramAddrIncrement = 1;

	_state.HvReg.HorizDisplayWidth = 0x1F;
	_state.HvLatch.HorizDisplayWidth = 0x1F;
	_state.HvReg.VertDisplayWidth = 239;
	_state.HvLatch.VertDisplayWidth = 239;

	_isVdc2 = isVdc2;
	_vramType = isVdc2 ? MemoryType::PceVideoRamVdc2 : MemoryType::PceVideoRam;
	_spriteRamType = isVdc2 ? MemoryType::PceSpriteRamVdc2 : MemoryType::PceSpriteRam;
	_emu->RegisterMemory(_vramType, _vram, 0x8000 * sizeof(uint16_t));
	_emu->RegisterMemory(_spriteRamType, _spriteRam, 0x100 * sizeof(uint16_t));
}

PceVdc::~PceVdc()
{
	delete[] _vram;
	delete[] _spriteRam;
}

PceVdcState& PceVdc::GetState()
{
	return _state;
}

uint8_t PceVdc::GetClockDivider()
{
	return _vce->GetClockDivider();
}

uint16_t PceVdc::GetScanlineCount()
{
	return _vce->GetScanlineCount();
}

uint16_t PceVdc::DotsToClocks(int dots)
{
	return dots * GetClockDivider();
}

void PceVdc::Exec()
{
	if(_state.SatbTransferRunning) {
		ProcessSatbTransfer();
	} else if(_vramDmaRunning) {
		ProcessVramDmaTransfer();
	}

	if(_rowHasSprite0 || _pendingMemoryWrite || _pendingMemoryRead) {
		DrawScanline();

		if(_pendingMemoryRead || _pendingMemoryWrite) {
			//TODO timing, this isn't quite right - should probably be aligned with VDC clocks?
			ProcessVramAccesses();
		}
	}

	if(_hModeCounter <= 3 || _nextEventCounter <= 3) {
		ProcessVdcEvents();
	} else {
		_hModeCounter -= 3;
		if(_nextEventCounter > 0) {
			_nextEventCounter -= 3;
		}
		_state.HClock += 3;
	}

	if(_state.HClock == 1365) {
		ProcessEndOfScanline();
	}

	if(!_isVdc2) {
		_emu->ProcessPpuCycle<CpuType::Pce>();
	}
}

void PceVdc::TriggerHdsIrqs()
{
	if(_needVertBlankIrq) {
		TriggerVerticalBlank();
	}
	if(_hasSpriteOverflow && _state.EnableOverflowIrq) {
		_state.SpriteOverflow = true;
		_vpc->SetIrq(this);
	}
	_hasSpriteOverflow = false;
}

void PceVdc::ProcessVdcEvents()
{
	for(int i = 0; i < 3; i++) {
		_state.HClock++;

		if(--_hModeCounter == 0) {
			DrawScanline();
			SetHorizontalMode((PceVdcModeH)(((int)_hMode + 1) % 4));
		}

		if(_nextEventCounter && --_nextEventCounter == 0) {
			ProcessEvent();
		}
	}
}

void PceVdc::ProcessEvent()
{
	DrawScanline();

	switch(_nextEvent) {
		case PceVdcEvent::LatchScrollY:
			_needVCounterClock = true;
			_latchClockY = _state.HClock;
			IncScrollY();
			if(!_state.BurstModeEnabled) {
				_state.BackgroundEnabled = _state.NextBackgroundEnabled;
				_state.SpritesEnabled = _state.NextSpritesEnabled;
			}
			_nextEvent = PceVdcEvent::LatchScrollX;
			_nextEventCounter = DotsToClocks(2);
			break;

		case PceVdcEvent::LatchScrollX:
			_state.HvLatch.BgScrollX = _state.HvReg.BgScrollX;
			_latchClockX = _state.HClock;
			_nextEvent = PceVdcEvent::HdsIrqTrigger;
			_nextEventCounter = DotsToClocks(6);
			break;

		case PceVdcEvent::HdsIrqTrigger:
			if(_evalStartCycle >= 1365) {
				//New row was about to start, but no time to start sprite eval, next row will have no sprites
				_spriteCount = 0;
			}
			TriggerHdsIrqs();
			_nextEvent = PceVdcEvent::None;
			_nextEventCounter = UINT16_MAX;
			break;

		case PceVdcEvent::IncRcrCounter:
			IncrementRcrCounter();
			_nextEvent = PceVdcEvent::None;
			_nextEventCounter = UINT16_MAX;
			break;
	}
}

void PceVdc::SetHorizontalMode(PceVdcModeH hMode)
{
	_hMode = hMode;
	switch(_hMode) {
		case PceVdcModeH::Hds:
			_hModeCounter = DotsToClocks((_state.HvLatch.HorizDisplayStart + 1) * 8);
			//LogDebug("H: " + std::to_string(_state.HClock) + " - HDS");
			break;

		case PceVdcModeH::Hdw:
			_needVCounterClock = true;
			_needRcrIncrement = true;
			_nextEvent = PceVdcEvent::IncRcrCounter;
			_nextEventCounter = DotsToClocks((_state.HvLatch.HorizDisplayWidth - 1) * 8) + 2;
			_hModeCounter = DotsToClocks((_state.HvLatch.HorizDisplayWidth + 1) * 8);
			//LogDebug("H: " + std::to_string(_state.HClock) + " - HDW start");
			break;

		case PceVdcModeH::Hde:
			_loadBgStart = UINT16_MAX;
			_evalStartCycle = UINT16_MAX;
			_loadSpriteStart = _state.HClock;

			_hModeCounter = DotsToClocks((_state.HvLatch.HorizDisplayEnd + 1) * 8);
			//LogDebug("H: " + std::to_string(_state.HClock) + " - HDE");
			break;

		case PceVdcModeH::Hsw:
			_loadBgStart = UINT16_MAX;
			_evalStartCycle = UINT16_MAX;
			_hModeCounter = DotsToClocks((_state.HvLatch.HorizSyncWidth + 1) * 8);
			_hSyncStartClock = _console->GetMasterClock();
			ProcessHorizontalSyncStart();
			//LogDebug("H: " + std::to_string(_state.HClock) + " - HSW");
			break;
	}
}

void PceVdc::ProcessVerticalSyncStart()
{
	//Latch VSW/VDS/VDW/VCR at the start of each vertical sync?
	_state.HvLatch.VertSyncWidth = _state.HvReg.VertSyncWidth;
	_state.HvLatch.VertDisplayStart = _state.HvReg.VertDisplayStart;
	_state.HvLatch.VertDisplayWidth = _state.HvReg.VertDisplayWidth;
	_state.HvLatch.VertEndPosVcr = _state.HvReg.VertEndPosVcr;

	_state.HvLatch.VramAccessMode = _state.HvReg.VramAccessMode;
	_state.HvLatch.SpriteAccessMode = _state.HvReg.SpriteAccessMode;
	_state.HvLatch.RowCount = _state.HvReg.RowCount;
	_state.HvLatch.ColumnCount = _state.HvReg.ColumnCount;
}

void PceVdc::ProcessHorizontalSyncStart()
{
	//Latch HSW/HDS/HDW/HDE at the start of each horizontal sync?
	_state.HvLatch.HorizSyncWidth = _state.HvReg.HorizSyncWidth;
	_state.HvLatch.HorizDisplayStart = _state.HvReg.HorizDisplayStart;
	_state.HvLatch.HorizDisplayWidth = _state.HvReg.HorizDisplayWidth;
	_state.HvLatch.HorizDisplayEnd = _state.HvReg.HorizDisplayEnd;
	_state.HvLatch.CgMode = _state.HvLatch.CgMode;

	_nextEvent = PceVdcEvent::None;
	_nextEventCounter = UINT16_MAX;
	_tileCount = 0;
	_screenOffsetX = 0;

	uint16_t displayStart = _state.HClock + _hModeCounter + DotsToClocks((_state.HvLatch.HorizDisplayStart + 1) * 8);

	if(displayStart - DotsToClocks(24) >= PceConstants::ClockPerScanline) {
		return;
	}

	//Calculate when sprite evaluation, sprite fetching and bg fetching will occur on the scanline
	if(_vMode == PceVdcModeV::Vdw || _state.RcrCounter == GetScanlineCount() - 1) {
		uint16_t displayWidth = DotsToClocks((_state.HvLatch.HorizDisplayWidth + 1) * 8);

		//Sprite evaluation runs on all visible scanlines + the scanline before the picture starts
		uint16_t spriteEvalStart = displayStart - DotsToClocks(16);
		_evalStartCycle = spriteEvalStart;
		_evalLastCycle = 0;
		_evalEndCycle = std::min<uint16_t>(PceConstants::ClockPerScanline, spriteEvalStart + displayWidth + DotsToClocks(8));

		if(_vMode == PceVdcModeV::Vdw) {
			//Turn on BG tile fetching
			uint16_t bgFetchStart = displayStart - DotsToClocks(16);
			_loadBgStart = bgFetchStart;
			_loadBgLastCycle = 0;
			_loadBgEnd = std::min<uint16_t>(PceConstants::ClockPerScanline, bgFetchStart + displayWidth + DotsToClocks(16));
		}
	}

	uint16_t eventClocks;
	if(_vMode == PceVdcModeV::Vdw) {
		_nextEvent = PceVdcEvent::LatchScrollY;

		//Less than 33 causes Asuka 120% to have a flickering line
		//Either the RCR interrupt is too early, or the latching was too late
		eventClocks = DotsToClocks(33);
	} else {
		_nextEvent = PceVdcEvent::HdsIrqTrigger;
		eventClocks = DotsToClocks(24);
	}

	if((int16_t)displayStart - (int16_t)eventClocks <= (int16_t)_state.HClock) {
		ProcessEvent();
	} else {
		_nextEventCounter = displayStart - eventClocks - _state.HClock;
	}
}

void PceVdc::ProcessSpriteEvaluation()
{
	if(_state.HClock < _evalStartCycle || _evalLastCycle >= 64 || _state.BurstModeEnabled) {
		return;
	}

	uint16_t end = (std::min(_evalEndCycle, _state.HClock) - _evalStartCycle) / GetClockDivider() / 4;
	if(_evalLastCycle >= end) {
		return;
	}

	//LogDebug("SPR EVAL: " + std::to_string(_evalLastCycle) + " -> " + std::to_string(end - 1));

	if(_evalLastCycle == 0) {
		LoadSpriteTiles();
		_spriteCount = 0;
		_spriteRow = (_state.RcrCounter + 1) % GetScanlineCount();
	}

	bool removeSpriteLimit = _emu->GetSettings()->GetPcEngineConfig().RemoveSpriteLimit;

	for(uint16_t i = _evalLastCycle; i < end; i++) {
		//4 VDC clocks is taken for each sprite
		if(i >= 64) {
			break;
		}

		int16_t y = (int16_t)(_spriteRam[i * 4] & 0x3FF) - 64;
		if(_spriteRow < y) {
			//Sprite not visible on this line
			continue;
		}

		uint8_t height;
		uint16_t flags = _spriteRam[i * 4 + 3];
		switch((flags >> 12) & 0x03) {
			default:
			case 0: height = 16; break;
			case 1: height = 32; break;
			case 2: case 3: height = 64; break;
		}

		if(_spriteRow >= y + height) {
			//Sprite not visible on this line
			continue;
		}

		uint16_t spriteRow = _spriteRow - y;

		bool verticalMirror = (flags & 0x8000) != 0;
		bool horizontalMirror = (flags & 0x800) != 0;

		int yOffset = 0;
		int rowOffset = 0;
		if(verticalMirror) {
			yOffset = (height - spriteRow - 1) & 0x0F;
			rowOffset = (height - spriteRow - 1) >> 4;
		} else {
			yOffset = spriteRow & 0x0F;
			rowOffset = spriteRow >> 4;
		}

		uint16_t tileIndex = (_spriteRam[i * 4 + 2] & 0x7FF) >> 1;
		bool loadSp23 = (_spriteRam[i * 4 + 2] & 0x01) != 0;
		uint8_t width = (flags & 0x100) ? 32 : 16;
		if(width == 32) {
			tileIndex &= ~0x01;
		}
		if(height == 32) {
			tileIndex &= ~0x02;
		} else if(height == 64) {
			tileIndex &= ~0x06;
		}
		uint16_t spriteTileY = tileIndex | (rowOffset << 1);

		for(int x = 0; x < width; x+=16) {
			if(_spriteCount >= 16) {
				_hasSpriteOverflow = true;
				if(!removeSpriteLimit) {
					break;
				}
			}

			int columnOffset;
			if(horizontalMirror) {
				columnOffset = (width - x - 1) >> 4;
			} else {
				columnOffset = x >> 4;
			}

			uint16_t spriteTile = spriteTileY | columnOffset;
			_sprites[_spriteCount].Index = i;
			_sprites[_spriteCount].X = (int16_t)(_spriteRam[i * 4 + 1] & 0x3FF) - 32 + x;
			_sprites[_spriteCount].TileAddress = spriteTile * 64 + yOffset;
			_sprites[_spriteCount].HorizontalMirroring = horizontalMirror;
			_sprites[_spriteCount].ForegroundPriority = (flags & 0x80) != 0;
			_sprites[_spriteCount].Palette = (flags & 0x0F);
			_sprites[_spriteCount].LoadSp23 = loadSp23;
			_spriteCount++;
		}
	}

	_evalLastCycle = end;
}

template<bool skipRender>
void PceVdc::LoadBackgroundTiles()
{
	if(_state.HClock < _loadBgStart || _state.BurstModeEnabled) {
		return;
	}

	uint16_t end = (std::min(_loadBgEnd, _state.HClock) - _loadBgStart) / GetClockDivider();

	if(_loadBgLastCycle >= end) {
		return;
	}

	//LogDebug("BG: " + std::to_string(_loadBgLastCycle) + " -> " + std::to_string(end - 1));

	uint16_t columnMask = _state.HvLatch.ColumnCount - 1;
	uint16_t scrollOffset = _state.HvLatch.BgScrollX >> 3;
	uint16_t row = (_state.HvLatch.BgScrollY) & ((_state.HvLatch.RowCount * 8) - 1);

	if(_state.HvLatch.VramAccessMode == 0) {
		for(uint16_t i = _loadBgLastCycle; i < end; i++) {
			if constexpr(skipRender) {
				_allowVramAccess = (i & 0x01) == 0;
			} else {
				switch(i & 0x07) {
					//CPU can access VRAM
					case 0: case 2: case 4: case 6: _allowVramAccess = true; break;

					case 1: LoadBatEntry(scrollOffset, columnMask, row); break;
					case 3: _allowVramAccess = false; break; //Unused BAT read?
					case 5: LoadTileDataCg0(row); break;
					case 7: LoadTileDataCg1(row); break;
				}
			}
		}
	} else if(_state.HvLatch.VramAccessMode == 3) {
		//Mode 3 is 4 cycles per read, CPU has no VRAM access, only 2BPP
		LoadBackgroundTilesWidth4(end, scrollOffset, columnMask, row);
	} else {
		//Mode 1/2 are 2 cycles per read
		LoadBackgroundTilesWidth2(end, scrollOffset, columnMask, row);
	}

	_loadBgLastCycle = end;
}

void PceVdc::LoadBackgroundTilesWidth2(uint16_t end, uint16_t scrollOffset, uint16_t columnMask, uint16_t row)
{
	for(uint16_t i = _loadBgLastCycle; i < end; i++) {
		_allowVramAccess = false;
		switch(i & 0x07) {
			case 1: LoadBatEntry(scrollOffset, columnMask, row); break;
			case 2: _allowVramAccess = true; break; //CPU
			case 3: _allowVramAccess = true; break; //CPU
			case 5: LoadTileDataCg0(row); break;
			case 7: LoadTileDataCg1(row); break;
		}
	}
}

void PceVdc::LoadBackgroundTilesWidth4(uint16_t end, uint16_t scrollOffset, uint16_t columnMask, uint16_t row)
{
	_allowVramAccess = false;
	for(uint16_t i = _loadBgLastCycle; i < end; i++) {
		switch(i & 0x07) {
			case 3: LoadBatEntry(scrollOffset, columnMask, row); break;

			case 7:
				//Load CG0 or CG1 based on CG mode flag
				_tiles[_tileCount].TileData[0] = ReadVram(_tiles[_tileCount].TileAddr + (row & 0x07) + (_state.HvLatch.CgMode ? 8 : 0));
				_tiles[_tileCount].TileData[1] = 0;
				_tileCount++;
				break;
		}
	}
}

void PceVdc::LoadBatEntry(uint16_t scrollOffset, uint16_t columnMask, uint16_t row)
{
	uint16_t tileColumn = (scrollOffset + _tileCount) & columnMask;
	uint16_t batEntry = ReadVram((row >> 3) * _state.HvLatch.ColumnCount + tileColumn);
	_tiles[_tileCount].Palette = batEntry >> 12;
	_tiles[_tileCount].TileAddr = ((batEntry & 0xFFF) * 16);
	_allowVramAccess = false;
}

void PceVdc::LoadTileDataCg0(uint16_t row)
{
	_tiles[_tileCount].TileData[0] = ReadVram(_tiles[_tileCount].TileAddr + (row & 0x07));
	_allowVramAccess = false;
}

void PceVdc::LoadTileDataCg1(uint16_t row)
{
	_tiles[_tileCount].TileData[1] = ReadVram(_tiles[_tileCount].TileAddr + (row & 0x07) + 8);
	_allowVramAccess = false;
	_tileCount++;
}

uint16_t PceVdc::ReadVram(uint16_t addr)
{
	if(addr < 0x8000) {
		return _vramOpenBus = _vram[addr];
	}

	//Camp California expects an empty sprite if tile index is out of bounds (>= $200) - this is probably caused by open bus behavior?
	//Return last word of the previously loaded VRAM data
	_emu->BreakIfDebugging(CpuType::Pce, BreakSource::PceBreakOnInvalidVramAddress);
	return _vramOpenBus;
}

void PceVdc::LoadSpriteTiles()
{
	_drawSpriteCount = 0;
	_rowHasSprite0 = false;

	if(_state.BurstModeEnabled || (_loadSpriteStart >= _loadBgStart && _loadSpriteStart < _loadBgEnd)) {
		return;
	}

	bool removeSpriteLimit = _emu->GetSettings()->GetPcEngineConfig().RemoveSpriteLimit;
	uint16_t clockCount = _loadSpriteStart > _loadBgStart ? (PceConstants::ClockPerScanline - _loadSpriteStart) + _loadBgStart : (_loadBgStart - _loadSpriteStart);
	bool hasSprite0 = false;
	memset(_xPosHasSprite, 0, sizeof(_xPosHasSprite));
	if(_state.HvLatch.SpriteAccessMode != 1) {
		//Modes 0/2/3 load 4 words over 4, 8 or 16 VDC clocks
		uint16_t clocksPerSprite;
		switch(_state.HvLatch.SpriteAccessMode) {
			default: case 0: clocksPerSprite = 4; break;
			case 2: clocksPerSprite = 8; break;
			case 3: clocksPerSprite = 16; break;
		}

		_drawSpriteCount = std::min<uint16_t>(_spriteCount, clockCount / GetClockDivider() / clocksPerSprite);
		_totalSpriteCount = removeSpriteLimit ? _spriteCount : _drawSpriteCount;
		for(int i = 0; i < _totalSpriteCount; i++) {
			PceSpriteInfo& spr = _drawSprites[i];
			spr = _sprites[i];
			memset(_xPosHasSprite + spr.X + 32, 1, 16);
			uint16_t addr = spr.TileAddress;
			spr.TileData[0] = ReadVram(addr);
			spr.TileData[1] = ReadVram(addr + 16);
			spr.TileData[2] = ReadVram(addr + 32);
			spr.TileData[3] = ReadVram(addr + 48);
			hasSprite0 |= spr.Index == 0;
		}
	} else {
		//Mode 1 uses 2BPP sprites, 4 clocks per sprite
		_drawSpriteCount = std::min<uint16_t>(_spriteCount, clockCount / GetClockDivider() / 4);
		_totalSpriteCount = removeSpriteLimit ? _spriteCount : _drawSpriteCount;
		for(int i = 0; i < _totalSpriteCount; i++) {
			PceSpriteInfo& spr = _drawSprites[i];
			spr = _sprites[i];
			memset(_xPosHasSprite + spr.X + 32, 1, 16);
			//Load SP0/SP1 or SP2/SP3 based on flag
			uint16_t addr = spr.TileAddress + (spr.LoadSp23 ? 32 : 0);
			spr.TileData[0] = ReadVram(addr);
			spr.TileData[1] = ReadVram(addr + 16);
			spr.TileData[2] = 0;
			spr.TileData[3] = 0;
			hasSprite0 |= spr.Index == 0;
		}
	}

	if(hasSprite0 && _drawSpriteCount > 1) {
		//Force VDC emulation to run on each CPU cycle, to ensure any sprite 0 hit IRQ is triggered at the correct time
		_rowHasSprite0 = true;
	}
}

bool PceVdc::IsDmaAllowed()
{
	if(!_allowDma && !_state.BurstModeEnabled) {
		//Can't DMA during rendering
		return false;
	}

	if(_hMode == PceVdcModeH::Hsw && _console->GetMasterClock() - _hSyncStartClock <= DotsToClocks(8)) {
		//VRAM accesses are blocked during the first 8 dots after horizontal sync,
		//which prevents SATB/VRAM DMA from running during that time (based on test rom result)
		return false;
	}

	return true;
}

void PceVdc::ProcessSatbTransfer()
{
	if(!IsDmaAllowed()) {
		return;
	}

	//This takes 1024 VDC cycles (so 2048/3072/4096 master clocks depending on VCE/VDC speed)
	//1 word transfered every 4 dots (8 to 16 master clocks, depending on VCE clock divider)
	_state.SatbTransferNextWordCounter += 3;
	if(_state.SatbTransferNextWordCounter / GetClockDivider() >= 4) {
		_state.SatbTransferNextWordCounter -= 4 * GetClockDivider();

		int i = _state.SatbTransferOffset;
		uint16_t value = ReadVram(_state.SatbBlockSrc + i);
		_emu->ProcessPpuWrite<CpuType::Pce>(i << 1, value, _spriteRamType);
		_emu->ProcessPpuWrite<CpuType::Pce>((i << 1) + 1, value, _spriteRamType);
		_spriteRam[i] = value;

		_state.SatbTransferOffset++;

		if(_state.SatbTransferOffset == 0) {
			_state.SatbTransferRunning = false;

			if(_state.VramSatbIrqEnabled) {
				_state.SatbTransferDone = true;
				_vpc->SetIrq(this);
			}
		}
	}
}

void PceVdc::ProcessVramDmaTransfer()
{
	if(!IsDmaAllowed()) {
		return;
	}

	_vramDmaPendingCycles += 3;

	uint8_t hClocksPerDmaCycle = GetClockDivider() * 2;

	while(_vramDmaPendingCycles >= hClocksPerDmaCycle) {
		if(_vramDmaReadCycle) {
			_vramDmaBuffer = ReadVram(_state.BlockSrc);
			_vramDmaReadCycle = false;
		} else {
			_state.BlockLen--;

			if(_state.BlockDst < 0x8000) {
				//Ignore writes over $8000
				_vram[_state.BlockDst] = _vramDmaBuffer;
			}

			_state.BlockSrc += (_state.DecrementSrc ? -1 : 1);
			_state.BlockDst += (_state.DecrementDst ? -1 : 1);
			_vramDmaReadCycle = true;

			if(_state.BlockLen == 0xFFFF) {
				_vramDmaRunning = false;
				_vramDmaPendingCycles = 0;
				if(_state.VramVramIrqEnabled) {
					_state.VramTransferDone = true;
					_vpc->SetIrq(this);
				}
				break;
			}
		}

		_vramDmaPendingCycles -= hClocksPerDmaCycle;
	}
}

void PceVdc::SetVertMode(PceVdcModeV vMode)
{
	_vMode = vMode;
	switch(_vMode) {
		default:
		case PceVdcModeV::Vds:
			_vModeCounter = _state.HvLatch.VertDisplayStart + 2;
			break;

		case PceVdcModeV::Vdw:
			_allowDma = false;
			_vModeCounter = _state.HvLatch.VertDisplayWidth + 1;
			_state.RcrCounter = 0;
			break;

		case PceVdcModeV::Vde:
			_vModeCounter = _state.HvLatch.VertEndPosVcr;
			break;

		case PceVdcModeV::Vsw:
			ProcessVerticalSyncStart();
			_vModeCounter = _state.HvLatch.VertSyncWidth + 1;
			break;
	}
}

void PceVdc::ClockVCounter()
{
	_vModeCounter--;
	if(_vModeCounter == 0) {
		SetVertMode((PceVdcModeV)(((int)_vMode + 1) % 4));
	}
	_needVCounterClock = false;
}

void PceVdc::IncrementRcrCounter()
{
	_state.RcrCounter++;

	_needRcrIncrement = false;
	ClockVCounter();

	if(_vMode == PceVdcModeV::Vde && _state.RcrCounter == _state.HvLatch.VertDisplayWidth + 1) {
		_needVertBlankIrq = true;
		_verticalBlankDone = true;
	}

	//This triggers ~12 VDC cycles before the end of the visible part of the scanline
	if(_state.EnableScanlineIrq && _state.RcrCounter == (int)_state.RasterCompareRegister - 0x40) {
		_state.ScanlineDetected = true;
		_vpc->SetIrq(this);
	}
}

void PceVdc::IncScrollY()
{
	if(_state.RcrCounter == 0) {
		_state.HvLatch.BgScrollY = _state.HvReg.BgScrollY;
	} else {
		if(_state.BgScrollYUpdatePending) {
			_state.HvLatch.BgScrollY = _state.HvReg.BgScrollY;
			_state.BgScrollYUpdatePending = false;
		}

		_state.HvLatch.BgScrollY++;
	}
}

void PceVdc::ProcessEndOfScanline()
{
	DrawScanline();

	_state.HClock = 0;
	_state.Scanline++;

	_latchClockX = UINT16_MAX;
	_latchClockY = UINT16_MAX;

	if(_state.Scanline == 256) {
		_state.FrameCount++;
		_vpc->SendFrame(this);
	} else if(_state.Scanline >= GetScanlineCount()) {
		//Update flags that were locked during burst mode
		_state.Scanline = 0;
		_verticalBlankDone = false;
		_state.BurstModeEnabled = !_state.NextBackgroundEnabled && !_state.NextSpritesEnabled;
		_state.BackgroundEnabled = _state.NextBackgroundEnabled;
		_state.SpritesEnabled = _state.NextSpritesEnabled;

		if(!_isVdc2) {
			_vpc->ProcessStartFrame();
			_emu->ProcessEvent(EventType::StartFrame);
		}
	}

	_vpc->ProcessScanlineStart(this, _state.Scanline);

	if(_needRcrIncrement) {
		IncrementRcrCounter();
	} else if(_needVCounterClock) {
		ClockVCounter();
	}

	if(_hMode == PceVdcModeH::Hdw) {
		//Display output was interrupted by hblank, start loading sprites in ~36 dots. (approximate, based on timing test)
		//Could be incorrect in some scenarios, needs more testing
		_loadSpriteStart = DotsToClocks(36);
	}
	
	//VCE sets HBLANK to low every 1365 clocks, interrupting what 
	//the VDC was doing and starting a HSW phase
	if(_hMode != PceVdcModeH::Hsw) {
		_hMode = PceVdcModeH::Hsw;
		_hSyncStartClock = _console->GetMasterClock();
	}

	_loadBgStart = UINT16_MAX;
	_evalStartCycle = UINT16_MAX;

	//The HSW phase appears to be longer in 7mhz mode compared to 5/10mhz modes
	//Less than 32 here breaks Camp California and Shapeshifter
	_hModeCounter = DotsToClocks(GetClockDivider() == 3 ? 32 : 24);
	_xStart = 0;
	_lastDrawHClock = 0;

	ProcessHorizontalSyncStart();

	if(_state.Scanline == GetScanlineCount() - 3) {
		//VCE sets VBLANK for 3 scanlines at the end of every frame
		SetVertMode(PceVdcModeV::Vsw);
	} else if(_state.Scanline == GetScanlineCount() - 2) {
		if(!_verticalBlankDone) {
			_needVertBlankIrq = true;
		}
	}
}

void PceVdc::TriggerDmaStart()
{
	_allowDma = true;

	if(_state.SatbTransferPending || _state.RepeatSatbTransfer) {
		_state.SatbTransferPending = false;
		_state.SatbTransferRunning = true;
		_state.SatbTransferNextWordCounter = 0;
		_state.SatbTransferOffset = 0;
	}
}

void PceVdc::TriggerVerticalBlank()
{
	//End of display, trigger irq
	if(_state.EnableVerticalBlankIrq) {
		_state.VerticalBlank = true;
		_vpc->SetIrq(this);
	}

	_needVertBlankIrq = false;

	//Any pending SATB/VRAM DMA starts at the same time as the vblank irq is triggered
	//This fixes the "new season" screen in "TV Sports Football"
	TriggerDmaStart();
}

uint8_t PceVdc::GetTilePixelColor(const uint16_t chr0, const uint16_t chr1, const uint8_t shift)
{
	uint16_t color = ((chr0 >> shift) & 0x101) | (((chr1 >> shift) & 0x101) << 2);
	return (uint8_t)(color | ((color & 0x500) >> 7));
}

uint8_t PceVdc::GetSpritePixelColor(const uint16_t chrData[4], const uint8_t shift)
{
	return (
		((chrData[0] >> shift) & 0x01) |
		(((chrData[1] >> shift) & 0x01) << 1) |
		(((chrData[2] >> shift) & 0x01) << 2) |
		(((chrData[3] >> shift) & 0x01) << 3)
	);
}

void PceVdc::DrawScanline()
{
	if(_state.Scanline < 14 || _state.Scanline >= 256) {
		//Only 242 rows can be shown
		return;
	}

	ProcessSpriteEvaluation();

	bool skipRender = _vpc->IsSkipRenderEnabled();
	if(skipRender) {
		LoadBackgroundTiles<true>();
	} else {
		LoadBackgroundTiles<false>();
	}

	if(_totalSpriteCount > 0) {
		if(_rowHasSprite0) {
			if(skipRender) {
				InternalDrawScanline<true, true, true>();
			} else {
				InternalDrawScanline<true, true, false>();
			}
		} else if(!skipRender) {
			InternalDrawScanline<true, false, false>();
		}
	} else if(!skipRender) {
		InternalDrawScanline<false, false, false>();
	}
}

template<bool hasSprites, bool hasSprite0, bool skipRender>
void PceVdc::InternalDrawScanline()
{
	uint16_t* out = _rowBuffer;

	uint16_t pixelsToDraw = (_state.HClock - _lastDrawHClock) / GetClockDivider();
	uint16_t xStart = _xStart;
	uint16_t xMax = _xStart + pixelsToDraw;

	bool inPicture = _hMode == PceVdcModeH::Hdw && _tileCount > 0;

	if(inPicture && (_state.BackgroundEnabled || _state.SpritesEnabled)) {
		PcEngineConfig& cfg = _emu->GetSettings()->GetPcEngineConfig();
		bool bgEnabled = _state.BackgroundEnabled && !(_isVdc2 ? cfg.DisableBackgroundVdc2 : cfg.DisableBackground);
		bool sprEnabled = _state.SpritesEnabled && !(_isVdc2 ? cfg.DisableSpritesVdc2 : cfg.DisableSprites);
		uint16_t grayscaleBit = _vce->IsGrayscale() ? 0x200 : 0;

		uint16_t outColor = 0;
		uint8_t bgColor = 0;
		for(; xStart < xMax; xStart++) {
			if constexpr(!skipRender) {
				outColor = PceVpc::TransparentPixelFlag | _vce->GetPalette(0);
				bgColor = 0;
				if(bgEnabled) {
					uint16_t screenX = (_state.HvLatch.BgScrollX & 0x07) + _screenOffsetX;
					uint16_t column = screenX >> 3;
					bgColor = GetTilePixelColor(_tiles[column].TileData[0], _tiles[column].TileData[1], 7 - (screenX & 0x07));
					if(bgColor != 0) {
						outColor = _vce->GetPalette(_tiles[column].Palette * 16 + bgColor);
					}
				}
			}

			if constexpr(hasSprites) {
				if(_state.SpritesEnabled && _xPosHasSprite[_screenOffsetX + 32]) {
					uint8_t sprColor;
					bool checkSprite0Hit = false;
					for(uint16_t i = 0; i < _totalSpriteCount; i++) {
						int16_t xOffset = _screenOffsetX - _drawSprites[i].X;
						if(xOffset >= 0 && xOffset < 16) {
							if(!_drawSprites[i].HorizontalMirroring) {
								xOffset = 15 - xOffset;
							}

							sprColor = GetSpritePixelColor(_drawSprites[i].TileData, xOffset);

							if(sprColor != 0) {
								if constexpr(hasSprite0) {
									if(checkSprite0Hit) {
										//Note: don't trigger sprite 0 hit for sprites that are drawn because of the "remove sprite limit" option
										if(_state.EnableCollisionIrq && i < _drawSpriteCount) {
											_state.Sprite0Hit = true;
											_vpc->SetIrq(this);
										}
									} else {
										if(sprEnabled && (bgColor == 0 || _drawSprites[i].ForegroundPriority)) {
											outColor = PceVpc::SpritePixelFlag | _vce->GetPalette(256 + _drawSprites[i].Palette * 16 + sprColor);
										}
									}
								
									if(_drawSprites[i].Index == 0) {
										checkSprite0Hit = true;
									} else {
										break;
									}
								} else {
									if(sprEnabled && (bgColor == 0 || _drawSprites[i].ForegroundPriority)) {
										outColor = PceVpc::SpritePixelFlag | _vce->GetPalette(256 + _drawSprites[i].Palette * 16 + sprColor);
									}
									break;
								}
							}
						}
					}
				}
			}

			if constexpr(!skipRender) {
				out[xStart] = outColor | grayscaleBit;
			}
			_screenOffsetX++;
		}
	} else if(inPicture) {
		if constexpr(!skipRender) {
			uint16_t color = _vce->GetPalette(0);
			for(; xStart < xMax; xStart++) {
				//In picture, but BG is not enabled, draw bg color
				out[xStart] = PceVpc::TransparentPixelFlag | color;
			}
		}
	} else {
		if constexpr(!skipRender) {
			uint16_t color = _vce->GetPalette(16 * 16);
			for(; xStart < xMax; xStart++) {
				//Output hasn't started yet, display overscan color
				out[xStart] = PceVpc::TransparentPixelFlag | color;
			}
		}
	}

	if(_state.HClock == 1365) {
		_vpc->ProcessScanlineEnd(this, _state.Scanline, _rowBuffer);
	}

	_xStart = xStart;
	_lastDrawHClock = _state.HClock / GetClockDivider() * GetClockDivider();
}

void PceVdc::ProcessVramRead()
{
	_state.ReadBuffer = ReadVram(_state.MemAddrRead);
	if(_state.MemAddrRead < 0x8000) {
		_emu->ProcessPpuRead<CpuType::Pce>((_state.MemAddrRead << 1), _state.ReadBuffer, _vramType);
		_emu->ProcessPpuRead<CpuType::Pce>((_state.MemAddrRead << 1) + 1, _state.ReadBuffer, _vramType);
	}
	_state.MemAddrRead += _state.VramAddrIncrement;
	_pendingMemoryRead = false;
}

void PceVdc::ProcessVramWrite()
{
	if(_state.MemAddrWrite < 0x8000) {
		//Ignore writes at $8000+
		_emu->ProcessPpuWrite<CpuType::Pce>(_state.MemAddrWrite << 1, _state.VramData, _vramType);
		_emu->ProcessPpuWrite<CpuType::Pce>((_state.MemAddrWrite << 1) + 1, _state.VramData, _vramType);
		_vram[_state.MemAddrWrite] = _state.VramData;
	} else {
		_emu->BreakIfDebugging(CpuType::Pce, BreakSource::PceBreakOnInvalidVramAddress);
	}
	_state.MemAddrWrite += _state.VramAddrIncrement;
	_pendingMemoryWrite = false;
}

void PceVdc::ProcessVramAccesses()
{
	if(_transferDelay) {
		_transferDelay -= 3;
		if(_transferDelay > 0) {
			return;
		}
	}

	_transferDelay = 0;

	bool inBgFetch = !_state.BurstModeEnabled && _state.HClock >= _loadBgStart && _state.HClock < _loadBgEnd && _state.Scanline >= 14 && _state.Scanline < 256;
	bool accessBlocked;

	if(_vMode != PceVdcModeV::Vdw || _state.BurstModeEnabled || (((!_state.SpritesEnabled || _spriteCount == 0) && !inBgFetch && _vMode == PceVdcModeV::Vdw))) {
		//-During SATB/VRAM DMA, prevent all transfers.
		//-Allow a VRAM read/write every other dot during:
		//  -vblank
		//  -forced blank (burst mode)
		//  -sprite fetching when no sprites need to be fetched (sprites disabled or no sprites were found during sprite evaluation)
		accessBlocked = (_state.SatbTransferRunning || _vramDmaRunning || ((_state.HClock / GetClockDivider()) & 0x01)) ? true : false;
	} else {
		//During tile/sprite fetching, only allow access on the CPU slots available during background tile fetches
		accessBlocked = inBgFetch && !_allowVramAccess;
		if(!accessBlocked && !inBgFetch && _state.SpritesEnabled) {
			//Find how many clocks have elapsed since sprite fetching started
			uint16_t clockCount = _state.HClock > _loadBgEnd ? (_state.HClock - _loadBgEnd) : (PceConstants::ClockPerScanline - _loadBgEnd + _state.HClock);
			uint16_t dotCount = clockCount / GetClockDivider();
			uint16_t clocksPerSprite;
			switch(_state.HvLatch.SpriteAccessMode) {
				default: case 0: case 1: clocksPerSprite = 4; break;
				case 2: clocksPerSprite = 8; break;
				case 3: clocksPerSprite = 16; break;
			}
			if(dotCount < _spriteCount * clocksPerSprite) {
				//VDC is still fetching sprites, block access
				accessBlocked = true;
			} else {
				//Sprite fetching is done, allow access every other dot
				accessBlocked = ((_state.HClock / GetClockDivider()) & 0x01) ? true : false;
			}
		}
	}

	if(!accessBlocked) {
		if(_hMode == PceVdcModeH::Hsw && _console->GetMasterClock() - _hSyncStartClock < 8 * GetClockDivider()) {
			//VRAM accesses appear to be blocked during the first 8 dots of horizontal sync
			return;
		}

		if(_pendingMemoryRead) {
			ProcessVramRead();
		} else if(_pendingMemoryWrite) {
			ProcessVramWrite();
		}
	}
}

void PceVdc::QueueMemoryRead()
{
	//All of this is guesswork based on results from a test rom
	//Read operations appear to be processed slightly slower than writes?
	_pendingMemoryRead = true;
	switch(GetClockDivider()) {
		case 2: _transferDelay = 15; break; //5 exec ticks
		case 3: _transferDelay = 24; break; //8 exec ticks
		case 4: _transferDelay = 24; break; //8 exec ticks
	}
}

void PceVdc::QueueMemoryWrite()
{
	//All of this is guesswork based on results from a test rom
	//Write operations appear to be processed slightly faster than reads?
	_pendingMemoryWrite = true;
	switch(GetClockDivider()) {
		case 2: _transferDelay = 12; break; //4 exec ticks
		case 3: _transferDelay = 18; break; //6 exec ticks
		case 4: _transferDelay = 21; break; //7 exec ticks
	}
}

void PceVdc::WaitForVramAccess()
{
	//Stall the CPU when a read/write operation is pending
	while(_pendingMemoryRead || _pendingMemoryWrite) {
		//TODO timing, this is probably not quite right. CPU will be stalled until
		//a VDC cycle that allows VRAM access is reached. This isn't always going to
		//be a multiple of 3 master clocks like this currently assumes
		_console->GetMemoryManager()->ExecFastCycle();
		DrawScanline();
	}
}

uint8_t PceVdc::ReadRegister(uint16_t addr)
{
	DrawScanline();

	switch(addr & 0x03) {
		default:
		case 0: {
			uint8_t result = 0;
			result |= (_pendingMemoryRead || _pendingMemoryWrite) ? 0x40 : 0x00;
			result |= _state.VerticalBlank ? 0x20 : 0x00;
			result |= _state.VramTransferDone ? 0x10 : 0x00;
			result |= _state.SatbTransferDone ? 0x08 : 0x00;
			result |= _state.ScanlineDetected ? 0x04 : 0x00;
			result |= _state.SpriteOverflow ? 0x02 : 0x00;
			result |= _state.Sprite0Hit ? 0x01 : 0x00;

			_state.VerticalBlank = false;
			_state.VramTransferDone = false;
			_state.SatbTransferDone = false;
			_state.ScanlineDetected = false;
			_state.SpriteOverflow = false;
			_state.Sprite0Hit = false;

			_vpc->ClearIrq(this);
			return result;
		}

		case 1: return 0; //Unused, reads return 0

		//Reads to 2/3 will always return the read buffer, but the
		//read address will only increment when register 2 is selected
		case 2:
			if(_pendingMemoryRead) {
				//D&D Order of the Griffon breaks without this
				WaitForVramAccess();
			}
			return (uint8_t)_state.ReadBuffer;

		case 3:
			if(_pendingMemoryRead) {
				WaitForVramAccess();
			}
			uint8_t value = _state.ReadBuffer >> 8;
			if(_state.CurrentReg == 0x02) {
				QueueMemoryRead();
			}
			return value;
	}
}

void PceVdc::WriteRegister(uint16_t addr, uint8_t value)
{
	DrawScanline();

	switch(addr & 0x03) {
		case 0: _state.CurrentReg = value & 0x1F; break;

		case 1: break; //Unused, writes do nothing

		case 2:
		case 3:
			bool msb = (addr & 0x03) == 0x03;

			switch(_state.CurrentReg) {
				case 0x00:
					WaitForVramAccess(); //Wonder Momo has graphical issues without this
					UpdateReg(_state.MemAddrWrite, value, msb);
					break;

				case 0x01:
					WaitForVramAccess();
					UpdateReg(_state.MemAddrRead, value, msb);
					if(msb) {
						QueueMemoryRead();
					}
					break;

				case 0x02:
					WaitForVramAccess();
					if(msb) {
						UpdateReg(_state.VramData, value, true);
						QueueMemoryWrite();
					} else {
						UpdateReg(_state.VramData, value, false);
					}
					break;

				case 0x05:
					if(msb) {
						//TODO output select
						//TODO dram refresh
						switch((value >> 3) & 0x03) {
							case 0: _state.VramAddrIncrement = 1; break;
							case 1: _state.VramAddrIncrement = 0x20; break;
							case 2: _state.VramAddrIncrement = 0x40; break;
							case 3: _state.VramAddrIncrement = 0x80; break;
						}
					} else {
						_state.EnableCollisionIrq = (value & 0x01) != 0;
						_state.EnableOverflowIrq = (value & 0x02) != 0;
						_state.EnableScanlineIrq = (value & 0x04) != 0;
						_state.EnableVerticalBlankIrq = (value & 0x08) != 0;
						
						_state.OutputVerticalSync = ((value & 0x30) >> 4) >= 2;
						_state.OutputHorizontalSync = ((value & 0x30) >> 4) >= 1;

						_state.NextSpritesEnabled = (value & 0x40) != 0;
						_state.NextBackgroundEnabled = (value & 0x80) != 0;
						if(_latchClockY == _state.HClock && !_state.BurstModeEnabled) {
							//Write occurred at the same time as the CR latch, update latch too
							_state.SpritesEnabled = _state.NextSpritesEnabled;
							_state.BackgroundEnabled = _state.NextBackgroundEnabled;
						}
					}
					break;

				case 0x06: UpdateReg<0x3FF>(_state.RasterCompareRegister, value, msb); break;
				case 0x07:
					UpdateReg<0x3FF>(_state.HvReg.BgScrollX, value, msb);
					if(_latchClockX == _state.HClock) {
						//Write occurred at the same time as the BXR latch, update latch too
						_state.HvLatch.BgScrollX = _state.HvReg.BgScrollX;
					}
					break;
				case 0x08:
					UpdateReg<0x1FF>(_state.HvReg.BgScrollY, value, msb);
					_state.BgScrollYUpdatePending = true;
					if(_latchClockY == _state.HClock) {
						//Write occurred at the same time as the BYR latch, update latch too
						IncScrollY();
					}
					break;

				case 0x09:
					if(!msb) {
						switch((value >> 4) & 0x03) {
							case 0: _state.HvReg.ColumnCount = 32; break;
							case 1: _state.HvReg.ColumnCount = 64; break;
							case 2: case 3: _state.HvReg.ColumnCount = 128; break;
						}

						_state.HvReg.RowCount = (value & 0x40) ? 64 : 32;

						_state.HvReg.VramAccessMode = value & 0x03;
						_state.HvReg.SpriteAccessMode = (value >> 2) & 0x03;
						_state.HvReg.CgMode = (value & 0x80) != 0;
					}
					break;

				case 0x0A:
					if(msb) {
						_state.HvReg.HorizDisplayStart = value & 0x7F;
					} else {
						_state.HvReg.HorizSyncWidth = value & 0x1F;
					}
					break;

				case 0x0B:
					if(msb) {
						_state.HvReg.HorizDisplayEnd = value & 0x7F;
					} else {
						_state.HvReg.HorizDisplayWidth = value & 0x7F;
					}
					break;

				case 0x0C: 
					if(msb) {
						_state.HvReg.VertDisplayStart = value;
					} else {
						_state.HvReg.VertSyncWidth = value & 0x1F;
					}
					break;

				case 0x0D:
					UpdateReg<0x1FF>(_state.HvReg.VertDisplayWidth, value, msb);
					break;

				case 0x0E: 
					if(!msb) {
						_state.HvReg.VertEndPosVcr = value;
					}
					break;

				case 0x0F:
					if(!msb) {
						LogDebugIf(_vramDmaRunning, "[VRAM DMA] Write to register while running");
						_state.VramSatbIrqEnabled = (value & 0x01) != 0;
						_state.VramVramIrqEnabled = (value & 0x02) != 0;
						_state.DecrementSrc = (value & 0x04) != 0;
						_state.DecrementDst = (value & 0x08) != 0;
						_state.RepeatSatbTransfer = (value & 0x10) != 0;
					}
					break;

				case 0x10:
					LogDebugIf(_vramDmaRunning, "[VRAM DMA] Write to register while running");
					UpdateReg(_state.BlockSrc, value, msb);
					break;

				case 0x11:
					LogDebugIf(_vramDmaRunning, "[VRAM DMA] Write to register while running");
					UpdateReg(_state.BlockDst, value, msb);
					break;

				case 0x12:
					LogDebugIf(_vramDmaRunning, "[VRAM DMA] Write to register while running");
					UpdateReg(_state.BlockLen, value, msb);
					if(msb) {
						_vramDmaRunning = true;
						_vramDmaReadCycle = true;
					}
					break;

				case 0x13:
					LogDebugIf(_state.SatbTransferRunning, "[Sprite DMA] Write to register while running");
					UpdateReg(_state.SatbBlockSrc, value, msb);
					if(msb) {
						_state.SatbTransferPending = true;
					}
					break;
			}
	}
}

void PceVdc::Serialize(Serializer& s)
{
	SVArray(_vram, 0x8000);
	SVArray(_spriteRam, 0x100);
	SVArray(_rowBuffer, PceConstants::MaxScreenWidth);

	SV(_state.FrameCount);
	SV(_state.HClock);
	SV(_state.Scanline);
	SV(_state.RcrCounter);
	SV(_state.CurrentReg);
	SV(_state.MemAddrWrite);
	SV(_state.MemAddrRead);
	SV(_state.ReadBuffer);
	SV(_state.VramData);
	SV(_state.EnableCollisionIrq);
	SV(_state.EnableOverflowIrq);
	SV(_state.EnableScanlineIrq);
	SV(_state.EnableVerticalBlankIrq);
	SV(_state.OutputVerticalSync);
	SV(_state.OutputHorizontalSync);
	SV(_state.SpritesEnabled);
	SV(_state.BackgroundEnabled);
	SV(_state.VramAddrIncrement);
	SV(_state.RasterCompareRegister);

	if(!s.IsSaving() && s.ContainsKey("ColumnCount")) {
		//Backward-compatibility for older save states
		s.Stream(_state.HvReg.ColumnCount, "ColumnCount");
		s.Stream(_state.HvReg.RowCount, "RowCount");
		s.Stream(_state.HvReg.SpriteAccessMode, "SpriteAccessMode");
		s.Stream(_state.HvReg.VramAccessMode, "VramAccessMode");
		s.Stream(_state.HvReg.CgMode, "CgMode");

		_state.HvLatch.ColumnCount = _state.HvReg.ColumnCount;
		_state.HvLatch.RowCount = _state.HvReg.RowCount;
		_state.HvLatch.SpriteAccessMode = _state.HvReg.SpriteAccessMode;
		_state.HvLatch.VramAccessMode = _state.HvReg.VramAccessMode;
		_state.HvLatch.CgMode = _state.HvReg.CgMode;
	} else {
		SV(_state.HvLatch.ColumnCount);
		SV(_state.HvLatch.RowCount);
		SV(_state.HvLatch.SpriteAccessMode);
		SV(_state.HvLatch.VramAccessMode);
		SV(_state.HvLatch.CgMode);
		SV(_state.HvReg.ColumnCount);
		SV(_state.HvReg.RowCount);
		SV(_state.HvReg.SpriteAccessMode);
		SV(_state.HvReg.VramAccessMode);
		SV(_state.HvReg.CgMode);
	}

	SV(_state.BgScrollYUpdatePending);
	SV(_state.HvLatch.BgScrollX);
	SV(_state.HvLatch.BgScrollY);
	SV(_state.HvLatch.HorizDisplayEnd);
	SV(_state.HvLatch.HorizDisplayStart);
	SV(_state.HvLatch.HorizDisplayWidth);
	SV(_state.HvLatch.HorizSyncWidth);
	SV(_state.HvLatch.VertDisplayStart);
	SV(_state.HvLatch.VertDisplayWidth);
	SV(_state.HvLatch.VertEndPosVcr);
	SV(_state.HvLatch.VertSyncWidth);
	SV(_state.HvReg.BgScrollX);
	SV(_state.HvReg.BgScrollY);
	SV(_state.HvReg.HorizDisplayEnd);
	SV(_state.HvReg.HorizDisplayStart);
	SV(_state.HvReg.HorizDisplayWidth);
	SV(_state.HvReg.HorizSyncWidth);
	SV(_state.HvReg.VertDisplayStart);
	SV(_state.HvReg.VertDisplayWidth);
	SV(_state.HvReg.VertEndPosVcr);
	SV(_state.HvReg.VertSyncWidth);
	SV(_state.VramSatbIrqEnabled);
	SV(_state.VramVramIrqEnabled);
	SV(_state.DecrementSrc);
	SV(_state.DecrementDst);
	SV(_state.RepeatSatbTransfer);
	SV(_state.BlockSrc);
	SV(_state.BlockDst);
	SV(_state.BlockLen);
	SV(_state.SatbBlockSrc);
	SV(_state.SatbTransferPending);
	SV(_state.SatbTransferRunning);
	SV(_state.SatbTransferNextWordCounter);
	SV(_state.SatbTransferOffset);
	SV(_state.VerticalBlank);
	SV(_state.VramTransferDone);
	SV(_state.SatbTransferDone);
	SV(_state.ScanlineDetected);
	SV(_state.SpriteOverflow);
	SV(_state.Sprite0Hit);
	SV(_state.BurstModeEnabled);
	SV(_state.NextSpritesEnabled);
	SV(_state.NextBackgroundEnabled);

	if(s.GetFormat() != SerializeFormat::Map) {
		//Hide these entries from the Lua API
		SV(_vramOpenBus);
		SV(_lastDrawHClock);
		SV(_xStart);
		SV(_hMode);
		SV(_hModeCounter);
		SV(_vMode);
		SV(_vModeCounter);

		SV(_screenOffsetX);
		SV(_needRcrIncrement);
		SV(_needVCounterClock);
		SV(_needVertBlankIrq);
		SV(_verticalBlankDone);

		SV(_spriteCount);
		SV(_spriteRow);
		SV(_evalStartCycle);
		SV(_evalEndCycle);
		SV(_evalLastCycle);
		SV(_hasSpriteOverflow);

		SV(_loadBgStart);
		SV(_loadBgEnd);
		SV(_loadBgLastCycle);
		SV(_tileCount);
		SV(_allowVramAccess);

		SV(_pendingMemoryRead);
		SV(_pendingMemoryWrite);
		SV(_transferDelay);

		SV(_vramDmaRunning);
		SV(_vramDmaReadCycle);
		SV(_vramDmaBuffer);
		SV(_vramDmaPendingCycles);

		SV(_nextEvent);
		SV(_nextEventCounter);
		SV(_hSyncStartClock);
		SV(_allowDma);

		SV(_drawSpriteCount);
		SV(_totalSpriteCount);
		SV(_rowHasSprite0);
		SV(_loadSpriteStart);

		SV(_latchClockX);
		SV(_latchClockY);

		SVArray(_xPosHasSprite, sizeof(_xPosHasSprite));

		for(int i = 0; i < _spriteCount; i++) {
			SVI(_sprites[i].X);
			SVI(_sprites[i].TileAddress);
			SVI(_sprites[i].Index);
			SVI(_sprites[i].Palette);
			SVI(_sprites[i].HorizontalMirroring);
			SVI(_sprites[i].ForegroundPriority);
			SVI(_sprites[i].LoadSp23);
		}

		for(int i = 0; i < _totalSpriteCount; i++) {
			SVI(_drawSprites[i].TileData[0]);
			SVI(_drawSprites[i].TileData[1]);
			SVI(_drawSprites[i].TileData[2]);
			SVI(_drawSprites[i].TileData[3]);
			SVI(_drawSprites[i].X);
			SVI(_drawSprites[i].Index);
			SVI(_drawSprites[i].Palette);
			SVI(_drawSprites[i].HorizontalMirroring);
			SVI(_drawSprites[i].ForegroundPriority);
		}

		for(int i = 0; i < _tileCount; i++) {
			SVI(_tiles[i].TileData[0]);
			SVI(_tiles[i].TileData[1]);
			SVI(_tiles[i].Palette);
			SVI(_tiles[i].TileAddr);
		}
	}
}

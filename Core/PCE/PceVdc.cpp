#include "stdafx.h"
#include "PCE/PceVdc.h"
#include "PCE/PceVce.h"
#include "PCE/PceVpc.h"
#include "PCE/PceMemoryManager.h"
#include "PCE/PceConstants.h"
#include "PCE/PceConsole.h"
#include "Shared/EmuSettings.h"
#include "EventType.h"

PceVdc::PceVdc(Emulator* emu, PceConsole* console, PceVpc* vpc, PceVce* vce, bool isVdc2)
{
	_emu = emu;
	_console = console;
	_vpc = vpc;
	_vce = vce;

	_vram = new uint16_t[0x8000];
	_spriteRam = new uint16_t[0x100];

	_emu->GetSettings()->InitializeRam(_vram, 0x10000);
	_emu->GetSettings()->InitializeRam(_spriteRam, 0x200);

	//These values can't ever be 0, init them to a possible value
	_state.ColumnCount = 32;
	_state.RowCount = 32;
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
		ProcessEndOfVisibleFrame();
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
			_needRcrIncrement = true;
			IncScrollY();
			_nextEvent = PceVdcEvent::LatchScrollX;
			_nextEventCounter = DotsToClocks(1);
			break;

		case PceVdcEvent::LatchScrollX:
			_state.HvLatch.BgScrollX = _state.HvReg.BgScrollX;
			_nextEvent = PceVdcEvent::HdsIrqTrigger;
			_nextEventCounter = DotsToClocks(7);
			if(!_state.BurstModeEnabled) {
				_state.BackgroundEnabled = _state.NextBackgroundEnabled;
				_state.SpritesEnabled = _state.NextSpritesEnabled;
			}
			break;

		case PceVdcEvent::HdsIrqTrigger:
			_needRcrIncrement = true;
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
}

void PceVdc::ProcessHorizontalSyncStart()
{
	//Latch HSW/HDS/HDW/HDE at the start of each horizontal sync?
	_state.HvLatch.HorizSyncWidth = _state.HvReg.HorizSyncWidth;
	_state.HvLatch.HorizDisplayStart = _state.HvReg.HorizDisplayStart;
	_state.HvLatch.HorizDisplayWidth = _state.HvReg.HorizDisplayWidth;
	_state.HvLatch.HorizDisplayEnd = _state.HvReg.HorizDisplayEnd;

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
	if(_state.HClock < _evalStartCycle || _hasSpriteOverflow || _evalLastCycle >= 64 || _state.BurstModeEnabled) {
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

	uint16_t columnMask = _state.ColumnCount - 1;
	uint16_t scrollOffset = _state.HvLatch.BgScrollX >> 3;
	uint16_t row = (_state.HvLatch.BgScrollY) & ((_state.RowCount * 8) - 1);

	if(_state.VramAccessMode == 0) {
		for(uint16_t i = _loadBgLastCycle; i < end; i++) {
			switch(i & 0x07) {
				//CPU can access VRAM
				case 0: case 2: case 4: case 6: _allowVramAccess = true; break;

				case 1: LoadBatEntry(scrollOffset, columnMask, row); break;
				case 3: _allowVramAccess = false; break; //Unused BAT read?
				case 5: LoadTileDataCg0(row); break;
				case 7: LoadTileDataCg1(row); break;
			}
		}
	} else if(_state.VramAccessMode == 3) {
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
		switch(i & 0x07) {
			case 1: LoadBatEntry(scrollOffset, columnMask, row); break;
			case 2: _allowVramAccess = true; break; //CPU
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
				_tiles[_tileCount].TileData[0] = ReadVram(_tiles[_tileCount].TileAddr + (row & 0x07) + (_state.CgMode ? 8 : 0));
				_tiles[_tileCount].TileData[1] = 0;
				_allowVramAccess = false;
				_tileCount++;
				break;
		}
	}
}

void PceVdc::LoadBatEntry(uint16_t scrollOffset, uint16_t columnMask, uint16_t row)
{
	uint16_t tileColumn = (scrollOffset + _tileCount) & columnMask;
	uint16_t batEntry = ReadVram((row >> 3) * _state.ColumnCount + tileColumn);
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
	//Camp California expects an empty sprite if tile index is out of bounds (>= $200) - this is probably caused by open bus behavior
	//Return last word of the previously loaded VRAM data
	return addr >= 0x8000 ? _vramOpenBus : _vramOpenBus = _vram[addr];
}

void PceVdc::LoadSpriteTiles()
{
	_drawSpriteCount = 0;
	_rowHasSprite0 = false;

	if(_state.BurstModeEnabled) {
		return;
	}

	bool removeSpriteLimit = _emu->GetSettings()->GetPcEngineConfig().RemoveSpriteLimit;
	uint16_t clockCount = _loadSpriteStart > _loadBgStart ? (PceConstants::ClockPerScanline - _loadSpriteStart) + _loadBgStart : (_loadBgStart - _loadSpriteStart);
	bool hasSprite0 = false;
	if(_state.SpriteAccessMode != 1) {
		//Modes 0/2/3 load 4 words over 4, 8 or 16 VDC clocks
		uint16_t clocksPerSprite;
		switch(_state.SpriteAccessMode) {
			default: case 0: clocksPerSprite = 4; break;
			case 2: clocksPerSprite = 8; break;
			case 3: clocksPerSprite = 16; break;
		}

		_drawSpriteCount = std::min<uint16_t>(_spriteCount, clockCount / GetClockDivider() / clocksPerSprite);
		_totalSpriteCount = removeSpriteLimit ? _spriteCount : _drawSpriteCount;
		for(int i = 0; i < _totalSpriteCount; i++) {
			PceSpriteInfo& spr = _drawSprites[i];
			spr = _sprites[i];
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
		//Force VDC emulation to run on each CPU cycle, to ensure any sprite 0 hit IRQ is triggerd at the correct time
		_rowHasSprite0 = true;
	}
}

void PceVdc::ProcessSatbTransfer()
{
	//This takes 1024 VDC cycles (so 2048/3072/4096 master clocks depending on VCE/VDC speed)
	//1 word transfered every 4 dots (8 to 16 master clocks, depending on VCE clock divider)
	_state.SatbTransferNextWordCounter += 3;
	if(_state.SatbTransferNextWordCounter / GetClockDivider() >= 4) {
		_state.SatbTransferNextWordCounter -= 4 * GetClockDivider();

		int i = _state.SatbTransferOffset;
		uint16_t value = ReadVram(_state.SatbBlockSrc + i);
		_emu->ProcessPpuWrite<CpuType::Pce>(i << 1, value & 0xFF, _spriteRamType);
		_emu->ProcessPpuWrite<CpuType::Pce>((i << 1) + 1, value >> 8, _spriteRamType);
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
	if(_vMode == PceVdcModeV::Vdw) {
		return;
	}

	_vramDmaPendingCycles += 3;

	uint8_t hClocksPerDmaCycle;
	switch(_state.VramAccessMode) {
		default:
		case 0: hClocksPerDmaCycle = GetClockDivider(); break;
		case 1: case 2: hClocksPerDmaCycle = GetClockDivider() * 2; break;
		case 3: hClocksPerDmaCycle = GetClockDivider() * 4; break;
	}

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
				if(_state.VramVramIrqEnabled) {
					_state.VramTransferDone = true;
					_vpc->SetIrq(this);
				}
			}
		}

		_vramDmaPendingCycles -= hClocksPerDmaCycle;
	}
}

void PceVdc::IncrementRcrCounter()
{
	_state.RcrCounter++;

	_needRcrIncrement = false;

	_vModeCounter--;
	if(_vModeCounter == 0) {
		_vMode = (PceVdcModeV)(((int)_vMode + 1) % 4);
		switch(_vMode) {
			default:
			case PceVdcModeV::Vds:
				_vModeCounter = _state.HvLatch.VertDisplayStart + 2;
				break;

			case PceVdcModeV::Vdw:
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
			_emu->ProcessEvent(EventType::StartFrame);
		}
	}

	_vpc->ProcessScanlineStart(this, _state.Scanline);

	if(_needRcrIncrement) {
		IncrementRcrCounter();
	}

	if(_hMode == PceVdcModeH::Hdw) {
		//Display output was interrupted by hblank, start loading sprites in ~36 dots. (approximate, based on timing test)
		//Could be incorrect in some scenarios, needs more testing
		_loadSpriteStart = DotsToClocks(36);
	}
	
	//VCE sets HBLANK to low every 1365 clocks, interrupting what 
	//the VDC was doing and starting a HSW phase
	_hMode = PceVdcModeH::Hsw;
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
		_vMode = PceVdcModeV::Vsw;
		ProcessVerticalSyncStart();
		_vModeCounter = _state.HvLatch.VertSyncWidth + 1;
	} else if(_state.Scanline == GetScanlineCount() - 2) {
		if(!_verticalBlankDone) {
			_needVertBlankIrq = true;
		}
	}
}

void PceVdc::ProcessEndOfVisibleFrame()
{
	//End of display, trigger irq?
	if(_state.SatbTransferPending || _state.RepeatSatbTransfer) {
		_state.SatbTransferPending = false;
		_state.SatbTransferRunning = true;
		_state.SatbTransferNextWordCounter = 0;
		_state.SatbTransferOffset = 0;
	}

	if(_state.EnableVerticalBlankIrq) {
		_state.VerticalBlank = true;
		_vpc->SetIrq(this);
	}

	_needVertBlankIrq = false;
}

uint8_t PceVdc::GetTilePixelColor(const uint16_t chrData[2], const uint8_t shift)
{
	return (
		((chrData[0] >> shift) & 0x01) |
		((chrData[0] >> (7 + shift)) & 0x02) |
		(((chrData[1] >> shift) & 0x01) << 2) |
		(((chrData[1] >> (7 + shift)) & 0x02) << 2)
	);
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
	LoadBackgroundTiles();

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

		for(; xStart < xMax; xStart++) {
			uint8_t bgColor = 0;
			uint16_t outColor = PceVpc::TransparentPixelFlag | _vce->GetPalette(0);
			if(bgEnabled) {
				uint16_t screenX = (_state.HvLatch.BgScrollX & 0x07) + _screenOffsetX;
				uint16_t column = screenX >> 3;
				bgColor = GetTilePixelColor(_tiles[column].TileData, 7 - (screenX & 0x07));
				if(bgColor != 0) {
					outColor = _vce->GetPalette(_tiles[column].Palette * 16 + bgColor);
				}
			}

			if(_state.SpritesEnabled) {
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
						}
					}
				}
			}

			out[xStart] = outColor | grayscaleBit;
			_screenOffsetX++;
		}
	} else if(inPicture) {
		uint16_t color = _vce->GetPalette(0);
		for(; xStart < xMax; xStart++) {
			//In picture, but BG is not enabled, draw bg color
			out[xStart] = color;
		}
	} else {
		uint16_t color = _vce->GetPalette(16 * 16);
		for(; xStart < xMax; xStart++) {
			//Output hasn't started yet, display overscan color
			out[xStart] = color;
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
	_emu->ProcessPpuRead<CpuType::Pce>((_state.MemAddrRead << 1), (uint8_t)_state.ReadBuffer, _vramType);
	_emu->ProcessPpuRead<CpuType::Pce>((_state.MemAddrRead << 1) + 1, (uint8_t)(_state.ReadBuffer >> 8), _vramType);
	_pendingMemoryRead = false;
}

void PceVdc::ProcessVramWrite()
{
	if(_state.MemAddrWrite < 0x8000) {
		//Ignore writes to mirror at $8000+
		_emu->ProcessPpuWrite<CpuType::Pce>(_state.MemAddrWrite << 1, _state.VramData & 0xFF, _vramType);
		_emu->ProcessPpuWrite<CpuType::Pce>((_state.MemAddrWrite << 1) + 1, _state.VramData, _vramType);
		_vram[_state.MemAddrWrite] = _state.VramData;
		_state.MemAddrWrite += _state.VramAddrIncrement;
	}
	_pendingMemoryWrite = false;
}

void PceVdc::ProcessVramAccesses()
{
	bool inBgFetch = !_state.BurstModeEnabled && _state.HClock >= _loadBgStart && _state.HClock < _loadBgEnd;
	bool accessBlocked = (
		_state.SatbTransferRunning ||
		(_vramDmaRunning && _vMode != PceVdcModeV::Vdw) ||
		(inBgFetch && !_allowVramAccess) ||
		(_state.SpritesEnabled && !inBgFetch && _vMode == PceVdcModeV::Vdw)
	);

	if(!accessBlocked) {
		if(_pendingMemoryRead) {
			ProcessVramRead();
		} else if(_pendingMemoryWrite) {
			ProcessVramWrite();
		}
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
			WaitForVramAccess();
			return (uint8_t)_state.ReadBuffer;

		case 3:
			WaitForVramAccess();

			uint8_t value = _state.ReadBuffer >> 8;
			if(_state.CurrentReg == 0x02) {
				_state.MemAddrRead += _state.VramAddrIncrement;
				_pendingMemoryRead = true;
			}
			return value;
	}
}

bool PceVdc::IsVramAccessBlocked()
{
	bool inBgFetch = !_state.BurstModeEnabled && _state.HClock >= _loadBgStart && _state.HClock < _loadBgEnd;
	//TODO timing:
	//does disabling sprites allow vram access during hblank?
	//can you access vram after the VDC is done loading sprites for that scanline?
	return (
		_pendingMemoryRead ||
		_pendingMemoryWrite ||
		_state.SatbTransferRunning ||
		(_vramDmaRunning && _vMode != PceVdcModeV::Vdw) ||
		(inBgFetch && !_allowVramAccess) || 
		(_state.SpritesEnabled && !inBgFetch && _vMode == PceVdcModeV::Vdw)
	);
}

void PceVdc::WaitForVramAccess()
{
	while(IsVramAccessBlocked()) {
		//TODO timing, this is probably not quite right. CPU will be stalled until
		//a VDC cycle that allows VRAM access is reached. This isn't always going to
		//be a multiple of 3 master clocks like this currently assumes
		_console->GetMemoryManager()->ExecFast();
		DrawScanline();
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
				case 0x00: UpdateReg(_state.MemAddrWrite, value, msb); break;

				case 0x01:
					UpdateReg(_state.MemAddrRead, value, msb);
					if(msb) {
						_pendingMemoryRead = true;
					}
					break;

				case 0x02:
					if(msb) {
						WaitForVramAccess();
						UpdateReg(_state.VramData, value, true);
						_pendingMemoryWrite = true;
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
					}
					break;

				case 0x06: UpdateReg<0x3FF>(_state.RasterCompareRegister, value, msb); break;
				case 0x07: UpdateReg<0x3FF>(_state.HvReg.BgScrollX, value, msb); break;
				case 0x08:
					UpdateReg<0x1FF>(_state.HvReg.BgScrollY, value, msb);
					_state.BgScrollYUpdatePending = true;
					break;

				case 0x09:
					if(!msb) {
						switch((value >> 4) & 0x03) {
							case 0: _state.ColumnCount = 32; break;
							case 1: _state.ColumnCount = 64; break;
							case 2: case 3: _state.ColumnCount = 128; break;
						}

						_state.RowCount = (value & 0x40) ? 64 : 32;

						_state.VramAccessMode = value & 0x03;
						_state.SpriteAccessMode = (value >> 2) & 0x03;
						_state.CgMode = (value & 0x80) != 0;
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
						_state.VramSatbIrqEnabled = (value & 0x01) != 0;
						_state.VramVramIrqEnabled = (value & 0x02) != 0;
						_state.DecrementSrc = (value & 0x04) != 0;
						_state.DecrementDst = (value & 0x08) != 0;
						_state.RepeatSatbTransfer = (value & 0x10) != 0;
					}
					break;

				case 0x10: UpdateReg(_state.BlockSrc, value, msb); break;
				case 0x11: UpdateReg(_state.BlockDst, value, msb); break;
				case 0x12:
					UpdateReg(_state.BlockLen, value, msb);
					if(msb) {
						_vramDmaRunning = true;
						_vramDmaReadCycle = true;
					}
					break;

				case 0x13:
					UpdateReg(_state.SatbBlockSrc, value, msb);
					if(msb) {
						_state.SatbTransferPending = true;
					}
					break;
			}
	}
}

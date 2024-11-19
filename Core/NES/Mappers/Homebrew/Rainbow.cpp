#include "pch.h"
#include "NES/Mappers/Homebrew/Rainbow.h"
#include "NES/Mappers/Homebrew/RainbowAudio.h"
#include "NES/Mappers/Homebrew/RainbowAudio.h"
#include "NES/Mappers/Homebrew/FlashS29.h"
#include "NES/NesConsole.h"
#include "NES/NesCpu.h"
#include "NES/NesMemoryManager.h"
#include "NES/BaseNesPpu.h"
#include "Shared/BatteryManager.h"
#include "Utilities/BitUtilities.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/Patches/IpsPatcher.h"

Rainbow::Rainbow()
{
}

void Rainbow::InitMapper()
{
	_audio.reset(new RainbowAudio(_console));
	_prgFlash.reset(new FlashS29(_prgRom, _prgSize));
	_chrFlash.reset(new FlashS29(_chrRom, _chrRomSize));
	UpdateState();

	AddRegisterRange(0x6000, 0xFFFF, MemoryOperation::Any);

	_orgPrgRom = vector<uint8_t>(_prgRom, _prgRom + _prgSize);
	_orgChrRom = vector<uint8_t>(_chrRom, _chrRom + _chrRomSize);
	ApplySaveData();
}

void Rainbow::ApplySaveData()
{
	if(_console->GetNesConfig().DisableFlashSaves) {
		return;
	}

	//Apply save data (saved as an IPS file), if found
	vector<uint8_t> ipsData = _emu->GetBatteryManager()->LoadBattery(".ips");
	if(!ipsData.empty()) {
		vector<uint8_t> patchedPrgRom;
		if(IpsPatcher::PatchBuffer(ipsData, _orgPrgRom, patchedPrgRom)) {
			memcpy(_prgRom, patchedPrgRom.data(), _prgSize);
		}
	}

	if(_chrRomSize > 0) {
		ipsData = _emu->GetBatteryManager()->LoadBattery(".chr.ips");
		if(!ipsData.empty()) {
			vector<uint8_t> patchedChrRom;
			if(IpsPatcher::PatchBuffer(ipsData, _orgChrRom, patchedChrRom)) {
				memcpy(_chrRom, patchedChrRom.data(), _chrRomSize);
			}
		}
	}
}

void Rainbow::SaveBattery()
{
	BaseMapper::SaveBattery();

	if(_console->GetNesConfig().DisableFlashSaves) {
		return;
	}

	vector<uint8_t> prgRom = vector<uint8_t>(_prgRom, _prgRom + _prgSize);
	vector<uint8_t> ipsData = IpsPatcher::CreatePatch(_orgPrgRom, prgRom);
	if(ipsData.size() > 8) {
		_emu->GetBatteryManager()->SaveBattery(".ips", ipsData.data(), (uint32_t)ipsData.size());
	}

	if(_chrRomSize > 0) {
		vector<uint8_t> chrRom = vector<uint8_t>(_chrRom, _chrRom + _chrRomSize);
		ipsData = IpsPatcher::CreatePatch(_orgChrRom, chrRom);
		if(ipsData.size() > 8) {
			_emu->GetBatteryManager()->SaveBattery(".chr.ips", ipsData.data(), (uint32_t)ipsData.size());
		}
	}
}

void Rainbow::Reset(bool softReset)
{
	WriteRegister(0x4100, 0x00);
	WriteRegister(0x4108, 0x00);
	WriteRegister(0x4118, 0x00);
	WriteRegister(0x4120, 0x00);
	WriteRegister(0x4130, 0x00);
	WriteRegister(0x4140, 0x00);
	WriteRegister(0x4126, 0x00);
	WriteRegister(0x4127, 0x00);
	WriteRegister(0x4128, 0x01);
	WriteRegister(0x4129, 0x01);
	WriteRegister(0x412E, 0x00);
	WriteRegister(0x412A, 0x00);
	WriteRegister(0x412B, 0x00);
	WriteRegister(0x412C, 0x00);
	WriteRegister(0x412D, 0x00);
	WriteRegister(0x412F, 0x80);
	WriteRegister(0x4241, 0x07);
	WriteRegister(0x4242, 0x1B);
	WriteRegister(0x4152, 0x00);
	WriteRegister(0x4153, 0x87);
	WriteRegister(0x415A, 0x00);
	WriteRegister(0x4190, 0x00);
	WriteRegister(0x41A9, 0x03);
	WriteRegister(0x41AA, 0x0F);
}

void Rainbow::OnAfterResetPowerOn()
{
	_console->GetMemoryManager()->RegisterReadHandler(this, 0x4011, 0x4011);
	_console->GetMemoryManager()->RegisterWriteHandler(this, 0x2000, 0x2000);
	_console->GetMemoryManager()->RegisterWriteHandler(this, 0x2003, 0x2004);
}

void Rainbow::WriteRam(uint16_t addr, uint8_t value)
{
	if(addr < 0x4000) {
		switch(addr) {
			case 0x2000: _largeSprites = value & 0x20; break;
			case 0x2003: _oamAddr = value; break;
			case 0x2004:
				if((_oamAddr & 0x03) == 0) {
					_oamPosY[_oamAddr >> 2] = value;
				}
				_oamAddr++;
				break;
		}

		_console->GetPpu()->WriteRam(addr, value);
	} else {
		BaseMapper::WriteRam(addr, value);
	}
}

uint8_t Rainbow::ReadRam(uint16_t addr)
{
	if(addr == 0x4011) {
		if(_cpuIrqAckOn4011) {
			AckCpuIrq();
		}
		return (_audio->GetLastOutput() << 1);
	}
	return BaseMapper::ReadRam(addr);
}

void Rainbow::GenerateOamClear()
{
	if(_oamCodeLocked) {
		return;
	}
	_oamCodeLocked = true;

	int i = 6;
	for(int spr = 0; spr < 64; spr++) {
		_oamCode[i++] = 0xA9; //LDA #spr
		_oamCode[i++] = spr * 4;

		if(spr == 0) {
			_oamCode[i++] = 0xAA; //TAX
			_oamCode[i++] = 0xCA; //DEX
		}

		_oamCode[i++] = 0x8D; //STA $2003
		_oamCode[i++] = 0x03;
		_oamCode[i++] = 0x20;

		_oamCode[i++] = 0x8E; //STX $2004
		_oamCode[i++] = 0x04;
		_oamCode[i++] = 0x20;
	}
	_oamCode[i++] = 0x60; //RTS
}

void Rainbow::GenerateExtUpdate()
{
	if(_oamCodeLocked) {
		return;
	}
	_oamCodeLocked = true;

	int i = 2;
	for(int spr = 0; spr < 64; spr++) {
		_oamCode[i++] = 0xA9; //LDA #[fpga ram value]
		_oamCode[i++] = _mapperRam[0x1800 + (_oamExtUpdatePage * 0x40) + spr];

		_oamCode[i++] = 0x8D; //STA $42xx
		_oamCode[i++] = spr;
		_oamCode[i++] = 0x42;
	}
	_oamCode[i++] = 0x60; //RTS
}

void Rainbow::GenerateOamSlowUpdate()
{
	if(_oamCodeLocked) {
		return;
	}
	_oamCodeLocked = true;

	int i = 0;
	_oamCode[i++] = 0xA9; //LDA #00
	_oamCode[i++] = 0x00;

	_oamCode[i++] = 0x8D; //STA $2003
	_oamCode[i++] = 0x03;
	_oamCode[i++] = 0x20;

	for(int j = 0; j < 256; j++) {
		_oamCode[i++] = 0xA9; //LDA #[fpga ram value]
		_oamCode[i++] = _mapperRam[0x1800 + (_oamSlowUpdatePage * 0x100) + j];

		_oamCode[i++] = 0x8D; //STA $2004
		_oamCode[i++] = 0x04;
		_oamCode[i++] = 0x20;
	}
	_oamCode[i++] = 0x60; //RTS
}

PrgMemoryType Rainbow::GetWorkRamType()
{
	return HasBattery() ? PrgMemoryType::SaveRam : PrgMemoryType::WorkRam;
}

void Rainbow::SelectHighBank(uint16_t start, uint16_t size, uint8_t reg)
{
	PrgMemoryType memType = (_highBanks[reg] & 0x8000) ? GetWorkRamType() : PrgMemoryType::PrgRom;
	MemoryAccessType accessType = (_highBanks[reg] & 0x8000) ? MemoryAccessType::ReadWrite : MemoryAccessType::Read;
	SetCpuMemoryMapping(start, start + size - 1, memType, (_highBanks[reg] & 0x7FFF) * size, accessType);
}

void Rainbow::SelectLowBank(uint16_t start, uint16_t size, uint8_t reg)
{
	switch((_lowBanks[reg] & 0xC000) >> 14) {
		case 0: case 1: SetCpuMemoryMapping(start, start + size - 1, PrgMemoryType::PrgRom, (_lowBanks[reg] & 0x7FFF) * size, MemoryAccessType::Read); break;
		case 2: SetCpuMemoryMapping(start, start + size - 1, GetWorkRamType(), (_lowBanks[reg] & 0x3FFF) * size, MemoryAccessType::ReadWrite); break;
		case 3: SetCpuMemoryMapping(start, start + size - 1, PrgMemoryType::MapperRam, (_lowBanks[reg] & 0x3FFF) * size, MemoryAccessType::ReadWrite); break;
	}
}

void Rainbow::SelectChrBank(uint16_t start, uint16_t size, uint8_t reg)
{
	if(_chrSource >= 2) {
		SetPpuMemoryMapping(0x0000, 0x0FFF, ChrMemoryType::MapperRam, 0, MemoryAccessType::ReadWrite);
		SetPpuMemoryMapping(0x1000, 0x1FFF, ChrMemoryType::MapperRam, 0, MemoryAccessType::ReadWrite);
	} else {
		ChrMemoryType type = _chrSource == 0 ? ChrMemoryType::ChrRom : ChrMemoryType::ChrRam;
		MemoryAccessType accessType = _chrSource == 0 ? MemoryAccessType::Read : MemoryAccessType::ReadWrite;
		SetPpuMemoryMapping(start, start + size - 1, type, _chrBanks[reg] * size, accessType);
	}
}

void Rainbow::UpdateState()
{
	switch(_highMode) {
		case 0:
			SelectHighBank(0x8000, 0x8000, 0);
			break;

		case 1:
			SelectHighBank(0x8000, 0x4000, 0);
			SelectHighBank(0xC000, 0x4000, 4);
			break;

		case 2:
			SelectHighBank(0x8000, 0x4000, 0);
			SelectHighBank(0xC000, 0x2000, 4);
			SelectHighBank(0xE000, 0x2000, 6);
			break;

		case 3:
			SelectHighBank(0x8000, 0x2000, 0);
			SelectHighBank(0xA000, 0x2000, 2);
			SelectHighBank(0xC000, 0x2000, 4);
			SelectHighBank(0xE000, 0x2000, 6);
			break;

		default:
			for(int i = 0; i < 8; i++) {
				SelectHighBank(0x8000 + i * 0x1000, 0x1000, i);
			}
			break;
	}

	if(_lowMode) {
		SelectLowBank(0x6000, 0x1000, 0);
		SelectLowBank(0x7000, 0x1000, 1);
	} else {
		SelectLowBank(0x6000, 0x2000, 0);
	}

	SetCpuMemoryMapping(0x5000, 0x5FFF, PrgMemoryType::MapperRam, _fpgaRamBank * 0x1000, MemoryAccessType::ReadWrite);
	SetCpuMemoryMapping(0x4800, 0x4FFF, PrgMemoryType::MapperRam, 0x1800, MemoryAccessType::ReadWrite);

	uint8_t chrBankCount = (1 << _chrMode);
	uint16_t chrBankSize = 0x2000 >> _chrMode;
	for(int i = 0; i < chrBankCount; i++) {
		SelectChrBank(0x0000 + i * chrBankSize, chrBankSize, i);
	}

	for(int i = 0; i < 4; i++) {
		NtControl& ctrl = _ntControl[i];
		uint16_t start = 0x2000 + i * 0x400;
		uint16_t end = 0x23FF + i * 0x400;
		switch(ctrl.Source) {
			case 0: SetPpuMemoryMapping(start, end, ChrMemoryType::NametableRam, _ntBanks[i] * 0x400, MemoryAccessType::ReadWrite); break;
			case 1: SetPpuMemoryMapping(start, end, ChrMemoryType::ChrRam, _ntBanks[i] * 0x400, MemoryAccessType::ReadWrite); break;
			case 2: SetPpuMemoryMapping(start, end, ChrMemoryType::MapperRam, (_ntBanks[i] & 0x03) * 0x400, MemoryAccessType::ReadWrite); break;
			case 3: SetPpuMemoryMapping(start, end, ChrMemoryType::ChrRom, _ntBanks[i] * 0x400, MemoryAccessType::Read); break;
		}
	}
}

void Rainbow::ProcessCpuClock()
{
	BaseProcessCpuClock();

	_jitterCounter++;

	_audio->Clock();

	if(_cpuIrqCounter && --_cpuIrqCounter == 0) {
		_cpuIrqCounter = _cpuIrqReloadValue;
		_cpuIrqPending = true;
		UpdateIrqStatus();
	}

	if(_ppuIdleCounter) {
		_ppuIdleCounter--;
		if(_ppuIdleCounter == 0) {
			//"The "in-frame" flag is cleared when the PPU is no longer rendering. This is detected when 3 CPU cycles pass without a PPU read having occurred (PPU /RD has not been low during the last 3 M2 rises)."
			_inFrame = false;
			_inHBlank = false;
			_scanlineCounter = -1;
			_ntFetchCounter = 0;
			_ntReadCounter = 0;
			_oamAddr = 0;
		}
	}
}

void Rainbow::UpdateInWindowFlag()
{
	bool yMatch;
	bool xMatch;

	uint8_t scanline = _ntFetchCounter >= 41 ? _scanlineCounter + 1 : _scanlineCounter;
	if(_windowY1 >= _windowY2) {
		yMatch = scanline <= _windowY2 || scanline > _windowY1;
	} else {
		yMatch = scanline >= _windowY1 && scanline <= _windowY2;
	}

	uint8_t column = (_ntFetchCounter + 1) % 42;
	if(_windowX1 >= _windowX2) {
		xMatch = column <= _windowX2 || column > _windowX1;
	} else {
		xMatch = column >= _windowX1 && column <= _windowX2;
	}

	_inWindow = xMatch && yMatch;
}

uint8_t Rainbow::MapperReadVram(uint16_t addr, MemoryOperationType memoryOperationType)
{
	if(_chrFlash->IsSoftwareIdMode()) {
		AddressInfo absAddr = GetPpuAbsoluteAddress(addr);
		if(absAddr.Address >= 0 && absAddr.Type == MemoryType::NesChrRom) {
			return _chrFlash->Read(absAddr.Address);
		}
	}

	_ppuReadCounter++;
	DetectScanlineStart(addr);

	if(_slIrqScanline == _scanlineCounter && _slIrqOffset == _ppuReadCounter) {
		_slIrqPending = true;
		UpdateIrqStatus();
	}

	_ppuIdleCounter = 3;
	_lastPpuReadAddr = addr;

	if(!_inFrame) {
		return InternalReadVram(addr);
	}

	bool isNtFetch = addr >= 0x2000 && addr <= 0x2FFF;
	if(isNtFetch) {
		//Nametable or attribute fetch
		bool isAttributeFetch = (addr & 0x3FF) >= 0x3C0;
		if(!isAttributeFetch) {
			//Nametable fetch, check if we're inside or outside the window
			_ntFetchCounter++;
			if(_ntFetchCounter == 33) {
				_inHBlank = true;
			}
			_inWindow = false;

			if(_windowEnabled) {
				UpdateInWindowFlag();
			}
		}

		NtControl& ctrl = _inWindow ? _windowControl : _ntControl[(addr >> 10) & 0x03];

		if(_inWindow) {
			uint8_t scanline = _ntFetchCounter >= 41 ? _scanlineCounter + 1 : _scanlineCounter;
			uint8_t windowScanline = (scanline + _windowScrollY) % 240;
			uint8_t column = (_ntFetchCounter + 1) % 42;
			if(!isAttributeFetch) {
				addr = ((windowScanline / 8 * 32) + ((column + _windowScrollX) & 0x1F));
			} else {
				addr = 0x3C0 + ((windowScanline >> 3) | (((column + _windowScrollX) & 0x1F) >> 2));
			}
		}

		//Process BG/Attribute ext modes and fill mode
		if(!isAttributeFetch) {
			_overrideTileFetch = ctrl.BgExtMode;
			bool hasExtMode = ctrl.AttrExtMode || ctrl.BgExtMode;
			if(hasExtMode) {
				_extData = _mapperRam[(ctrl.FpgaRamSrc * 0x400) + (addr & 0x3FF)];
			}
			if(ctrl.FillMode) {
				return _fillModeTileIndex;
			}
		} else {
			if(ctrl.AttrExtMode) {
				uint8_t attr = (_extData & 0xC0) >> 6;
				return attr | (attr << 2) | (attr << 4) | (attr << 6);
			} else if(ctrl.FillMode) {
				uint8_t attr = _fillModeAttrIndex;
				return attr | (attr << 2) | (attr << 4) | (attr << 6);
			}
		}

		if(_inWindow) {
			//If in window (and fill mode is not enabled), return the window nametable/attribute data
			return _mapperRam[_windowBank * 0x400 + addr];
		}
	} else {
		//Tile data fetches
		bool isBgFetch = _ntFetchCounter < 33 || _ntFetchCounter >= 41;
		if(_inWindow && isBgFetch) {
			uint8_t scanline = _ntFetchCounter >= 41 ? _scanlineCounter + 1 : _scanlineCounter;
			uint8_t windowScanline = (scanline + _windowScrollY) % 240;
			if(_overrideTileFetch) {
				uint32_t fetchAddr = (addr & 0xFF8) | (windowScanline & 0x07) | ((_extData & 0x3F) << 12) | (_bgExtModeOffset << 18);
				return ReadChr(fetchAddr);
			} else {
				uint32_t fetchAddr = (addr & 0x1FF8) | (windowScanline & 0x07);
				return ReadChr(fetchAddr);
			}
		} else if(_overrideTileFetch && isBgFetch) {
			uint32_t fetchAddr = (addr & 0xFFF) | ((_extData & 0x3F) << 12) | (_bgExtModeOffset << 18);
			return ReadChr(fetchAddr);
		} else if(_spriteExtMode && !isBgFetch) {
			uint8_t spriteIndex = _oamMappings[_ntFetchCounter - 33];
			uint32_t fetchAddr;
			if(_largeSprites) {
				fetchAddr = (_spriteExtBank << 21) | (_spriteExtData[spriteIndex] << 13) | (addr & 0x1FFF);
			} else {
				fetchAddr = (_spriteExtBank << 20) | (_spriteExtData[spriteIndex] << 12) | (addr & 0xFFF);
			}
			return ReadChr(fetchAddr);
		}
	}

	return InternalReadVram(addr);
}

void Rainbow::MapperWriteVram(uint16_t addr, uint8_t value)
{
	if(addr < 0x2000) {
		AddressInfo absAddr = GetPpuAbsoluteAddress(addr);
		if(absAddr.Address >= 0 && absAddr.Type == MemoryType::NesChrRom) {
			_chrFlash->Write(absAddr.Address, value);
			return;
		}
	}

	InternalWriteVram(addr, value);
}

uint8_t Rainbow::ReadChr(uint32_t addr)
{
	switch(_chrSource) {
		default:
		case 0: return _chrRomSize ? _chrRom[addr & (_chrRomSize - 1)] : 0;
		case 1: return _chrRamSize ? _chrRam[addr & (_chrRamSize - 1)] : 0;

		case 2:
		case 3:
			return _mapperRam[addr & 0x1FFF];
	}
}

void Rainbow::UpdateIrqStatus()
{
	bool active = (_cpuIrqEnabled && _cpuIrqPending) || (_slIrqEnabled && _slIrqPending);
	if(active) {
		if(!_console->GetCpu()->HasIrqSource(IRQSource::External)) {
			_jitterCounter = 0;
		}
		_console->GetCpu()->SetIrqSource(IRQSource::External);
	} else {
		_console->GetCpu()->ClearIrqSource(IRQSource::External);
	}
}

void Rainbow::AckCpuIrq()
{
	_cpuIrqEnabled = _cpuIrqEnableAfterAck;
	if(_cpuIrqEnabled) {
		_cpuIrqCounter = _cpuIrqReloadValue;
	}
	_cpuIrqPending = false;
	UpdateIrqStatus();
}

void Rainbow::ProcessSpriteEval()
{
	uint8_t spriteCount = 0;
	int scanline = _scanlineCounter;
	int height = (_largeSprites ? 16 : 8);

	memset(_oamMappings, 0, sizeof(_oamMappings));

	for(int i = 0; i < 64; i++) {
		uint8_t y = _oamPosY[i];
		if(scanline >= y && scanline < y + height) {
			_oamMappings[spriteCount] = i;
			spriteCount++;

			if(spriteCount >= 8) {
				break;
			}
		}
	}
}

void Rainbow::DetectScanlineStart(uint16_t addr)
{
	if(addr >= 0x2000 && addr <= 0x2FFF) {
		if(_lastPpuReadAddr == addr) {
			//Count consecutive identical reads
			_ntReadCounter++;

			if(_ntReadCounter >= 2) {
				//After 3 identical NT reads, trigger IRQ when the following attribute byte is read
				if(!_inFrame) {
					_inFrame = true;
					_scanlineCounter = 0;
				} else {
					_scanlineCounter++;
				}

				ProcessSpriteEval();

				_ntFetchCounter = 0;
				_ppuReadCounter = 0;
				_ntReadCounter = 0;
				_inHBlank = false;
			}
		} else {
			_ntReadCounter = 0;
		}
	} else {
		_ntReadCounter = 0;
	}
}

uint8_t Rainbow::ReadRegister(uint16_t addr)
{
	switch(addr) {
		case 0x4100: return _highMode | (_lowMode << 7);
		case 0x4120: return _chrMode | ((uint8_t)_windowEnabled << 4) | ((uint8_t)_spriteExtMode << 5) | (_chrSource << 6);

		case 0x412A: case 0x412B: case 0x412C: case 0x412D:
			return _ntControl[addr - 0x412A].ToByte();

		case 0x412F: return _windowControl.ToByte();

		case 0x4151:
			_slIrqPending = false;
			UpdateIrqStatus();
			return (
				((uint8_t)_inHBlank << 7) |
				((uint8_t)_inFrame << 6) |
				(uint8_t)_slIrqPending
				);

		case 0x4154: return _jitterCounter;

		case 0x415F:
		{
			uint8_t value = _mapperRam[_fpgaRamAddr];
			_fpgaRamAddr = (_fpgaRamAddr + _fpgaRamInc) & 0x1FFF;
			return value;
		}

		case 0x4160: return 0x20;
		case 0x4161:
			return (
				(uint8_t)_wifiIrqPending |
				((uint8_t)_cpuIrqPending << 6) |
				((uint8_t)_slIrqPending << 7)
				);

		case 0x4280: GenerateOamSlowUpdate(); break;
		case 0x4282: GenerateExtUpdate(); break;
		case 0x4286: GenerateOamClear(); break;

		case 0x4190: return (uint8_t)_espEnabled | ((uint8_t)_wifiIrqEnabled << 1);
		case 0x4191: return ((uint8_t)_dataReady << 6) | ((uint8_t)_dataReceived << 7);
		case 0x4192: return (uint8_t)_dataSent << 7;

		case 0xFFFA:
		case 0xFFFB:
			_inFrame = false;
			_lastPpuReadAddr = 0;
			_scanlineCounter = 0;
			_slIrqPending = false;
			UpdateIrqStatus();

			if(_nmiVectorEnabled) {
				return (addr & 0x01) ? _nmiVectorAddr : (_nmiVectorAddr >> 8);
			}
			return DebugReadRam(addr);

		case 0xFFFE:
		case 0xFFFF:
			if(_irqVectorEnabled) {
				return (addr & 0x01) ? _irqVectorAddr : (_irqVectorAddr >> 8);
			}
			return DebugReadRam(addr);
	}

	if(addr >= 0x4280 && addr < 0x4800) {
		//Built-in OAM functions
		if(addr >= 0x4286) {
			_oamCodeLocked = false;
		}
		return _oamCode[addr - 0x4280];
	}

	if(addr >= 0x6000) {
		if(_prgFlash->IsSoftwareIdMode()) {
			AddressInfo absAddr = GetAbsoluteAddress(addr);
			if(absAddr.Address >= 0 && absAddr.Type == MemoryType::NesPrgRom) {
				return _prgFlash->Read(absAddr.Address);
			}
		}
		return InternalReadRam(addr);
	} else {
		return _console->GetMemoryManager()->GetOpenBus();
	}
}

void Rainbow::WriteRegister(uint16_t addr, uint8_t value)
{
	switch(addr) {
		case 0x4100:
			_highMode = value & 0x07;
			_lowMode = (value & 0x80) >> 7;
			UpdateState();
			break;

		case 0x4115:
			_fpgaRamBank = value & 0x01;
			UpdateState();
			break;

		case 0x4120:
			_chrMode = value & 0x07;
			_windowEnabled = value & 0x10;
			_spriteExtMode = value & 0x20;
			_chrSource = (value & 0xC0) >> 6;
			UpdateState();
			break;

		case 0x4121: _bgExtModeOffset = value & 0x1F; break;

		case 0x4124: _fillModeTileIndex = value; break;
		case 0x4125: _fillModeAttrIndex = value; break;
		case 0x412E: _windowBank = value; break;

		case 0x412F:
			_windowControl.BgExtMode = value & 0x01;
			_windowControl.AttrExtMode = value & 0x02;
			_windowControl.FpgaRamSrc = (value & 0x0C) >> 2;
			_windowControl.FillMode = value & 0x20;
			_windowControl.Source = (value & 0xC0) >> 6; //todo this is writeable/readable but ignored?
			break;

		case 0x4150: _slIrqScanline = value; break;
		case 0x4151: _slIrqEnabled = true; break;

		case 0x4152:
			_slIrqEnabled = false;
			_slIrqPending = false;
			UpdateIrqStatus();
			break;

		case 0x4153: _slIrqOffset = std::clamp<uint8_t>(value, 1, 170); break;

		case 0x4158: BitUtilities::SetBits<8>(_cpuIrqReloadValue, value); break;
		case 0x4159: BitUtilities::SetBits<0>(_cpuIrqReloadValue, value); break;
		case 0x415A:
			_cpuIrqEnabled = value & 0x01;
			_cpuIrqEnableAfterAck = value & 0x02;
			_cpuIrqAckOn4011 = value & 0x04;
			if(_cpuIrqEnabled) {
				_cpuIrqCounter = _cpuIrqReloadValue;
			}
			UpdateIrqStatus();
			break;

		case 0x415B: AckCpuIrq(); break;

		case 0x415C: BitUtilities::SetBits<8>(_fpgaRamAddr, value & 0x1F); break;
		case 0x415D: BitUtilities::SetBits<0>(_fpgaRamAddr, value); break;
		case 0x415E: _fpgaRamInc = value; break;
		case 0x415F:
			_mapperRam[_fpgaRamAddr] = value;
			_fpgaRamAddr = (_fpgaRamAddr + _fpgaRamInc) & 0x1FFF;
			break;

		case 0x416B:
			_nmiVectorEnabled = value & 0x01;
			_irqVectorEnabled = value & 0x02;
			break;

		case 0x416C: BitUtilities::SetBits<8>(_nmiVectorAddr, value); break;
		case 0x416D: BitUtilities::SetBits<0>(_nmiVectorAddr, value); break;
		case 0x416E: BitUtilities::SetBits<8>(_irqVectorAddr, value); break;
		case 0x416F: BitUtilities::SetBits<0>(_irqVectorAddr, value); break;

		case 0x4170: _windowX1 = value & 0x1F; break;
		case 0x4171: _windowX2 = value & 0x1F; break;
		case 0x4172: _windowY1 = value; break;
		case 0x4173: _windowY2 = value; break;
		case 0x4174: _windowScrollX = value & 0x1F; break;
		case 0x4175: _windowScrollY = value; break;

		case 0x4190:
			_espEnabled = value & 0x01;
			_wifiIrqEnabled = value & 0x02;
			break;

		case 0x4191: _dataReceived = false; break;
		case 0x4192: _dataSent = false; break;
		case 0x4193: _recvDstAddr = value & 0x07; break;
		case 0x4194: _sendSrcAddr = value & 0x07; break;

		case 0x4240: _spriteExtBank = value & 0x07; break;
		case 0x4241: _oamSlowUpdatePage = value & 0x07; break;
		case 0x4242: _oamExtUpdatePage = value & 0x1F; break;
	}

	if(addr >= 0x4106 && addr <= 0x4107) {
		BitUtilities::SetBits<8>(_lowBanks[addr - 0x4106], value);
		UpdateState();
	} else if(addr >= 0x4116 && addr <= 0x4117) {
		BitUtilities::SetBits<0>(_lowBanks[addr - 0x4116], value);
		UpdateState();
	} if(addr >= 0x4108 && addr <= 0x410F) {
		BitUtilities::SetBits<8>(_highBanks[addr - 0x4108], value);
		UpdateState();
	} else if(addr >= 0x4118 && addr <= 0x411F) {
		BitUtilities::SetBits<0>(_highBanks[addr - 0x4118], value);
		UpdateState();
	} else if(addr >= 0x4130 && addr <= 0x413F) {
		BitUtilities::SetBits<8>(_chrBanks[addr - 0x4130], value);
		UpdateState();
	} else if(addr >= 0x4140 && addr <= 0x414F) {
		BitUtilities::SetBits<0>(_chrBanks[addr - 0x4140], value);
		UpdateState();
	} else if(addr >= 0x4126 && addr <= 0x4129) {
		_ntBanks[addr - 0x4126] = value;
		UpdateState();
	} else if(addr >= 0x412A && addr <= 0x412D) {
		NtControl& ctrl = _ntControl[addr - 0x412A];
		ctrl.AttrExtMode = value & 0x01;
		ctrl.BgExtMode = value & 0x02;
		ctrl.FpgaRamSrc = (value & 0x0C) >> 2;
		ctrl.FillMode = value & 0x20;
		ctrl.Source = (value & 0xC0) >> 6;
		UpdateState();
	} else if(addr >= 0x41A0 && addr <= 0x41AA) {
		_audio->WriteRegister(addr, value);
	} else if(addr >= 0x4200 && addr <= 0x423F) {
		_spriteExtData[addr - 0x4200] = value;
	}

	if(addr >= 0x6000) {
		AddressInfo absAddr = GetAbsoluteAddress(addr);
		if(absAddr.Address >= 0 && absAddr.Type == MemoryType::NesPrgRom) {
			_prgFlash->Write(absAddr.Address, value);
		} else {
			WritePrgRam(addr, value);
		}
	}
}

vector<MapperStateEntry> Rainbow::GetMapperStateEntries()
{
	vector<MapperStateEntry> entries;
	entries.push_back(MapperStateEntry("", "CPU Mappings"));
	entries.push_back(MapperStateEntry("$4100.0-2", "ROM Mode (8000-FFFF)", _highMode, MapperStateValueType::Number8));
	entries.push_back(MapperStateEntry("$4100.7", "RAM Mode (6000-7FFF)", _lowMode, MapperStateValueType::Number8));

	entries.push_back(MapperStateEntry("$4106/16", "RAM Bank 0", _lowBanks[0] & 0x7FFF, MapperStateValueType::Number16));
	string source;
	switch((_lowBanks[0] & 0xC000) >> 14) {
		case 0: case 1: source = "PRG-ROM"; break;
		case 2: source = "PRG-RAM"; break;
		case 3: source = "FPGA RAM"; break;
	}
	entries.push_back(MapperStateEntry("$4116.6-7", "PRG-RAM Bank 0 Source", source));

	entries.push_back(MapperStateEntry("$4107/17", "PRG-RAM Bank 1", _lowBanks[1], MapperStateValueType::Number16));
	switch((_lowBanks[0] & 0xC000) >> 14) {
		case 0: case 1: source = "PRG-ROM"; break;
		case 2: source = "PRG-RAM"; break;
		case 3: source = "FPGA RAM"; break;
	}
	entries.push_back(MapperStateEntry("$4117.6-7", "PRG-RAM Bank 1 Source", source));

	for(int i = 0; i < 8; i++) {
		entries.push_back(MapperStateEntry(
			"$" + HexUtilities::ToHex(0x4108 + i) + "/" + HexUtilities::ToHex(0x4118 + i) + ".0-14",
			"ROM Bank " + std::to_string(i),
			_highBanks[i] & 0x7FFF,
			MapperStateValueType::Number16
		));

		entries.push_back(MapperStateEntry(
			HexUtilities::ToHex(0x4118 + i) + ".7",
			"ROM Bank " + std::to_string(i) + " Source",
			(string)((_highBanks[i] & 0x8000) ? "RAM" : "ROM")
		));
	}

	entries.push_back(MapperStateEntry("$4115.0", "FPGA RAM Bank", _fpgaRamBank, MapperStateValueType::Number8));

	entries.push_back(MapperStateEntry("", "PPU Config"));
	entries.push_back(MapperStateEntry("$4120.0-2", "CHR Banking Mode", _chrMode, MapperStateValueType::Number16));
	entries.push_back(MapperStateEntry("$4120.4", "Window Split Mode", _windowEnabled));
	entries.push_back(MapperStateEntry("$4120.5", "Sprite Extended Mode", _spriteExtMode));
	entries.push_back(MapperStateEntry("$4120.6-7", "CHR Source", string(_chrSource ? "RAM" : "ROM"), _chrSource));
	entries.push_back(MapperStateEntry("$4121.0-5", "Extended BG CHR Bank", _bgExtModeOffset, MapperStateValueType::Number8));

	entries.push_back(MapperStateEntry("", "Fill Mode"));
	entries.push_back(MapperStateEntry("$4124", "Tile Index", _fillModeTileIndex, MapperStateValueType::Number8));
	entries.push_back(MapperStateEntry("$4125", "Attribute Index", _fillModeAttrIndex, MapperStateValueType::Number8));

	auto processNtControl = [&](uint16_t addr, NtControl& ctrl) {
		entries.push_back(MapperStateEntry("$" + HexUtilities::ToHex(addr) + ".0", "Attribute Ext. Mode", ctrl.AttrExtMode));
		entries.push_back(MapperStateEntry("$" + HexUtilities::ToHex(addr) + ".1", "BG Ext. Mode", ctrl.BgExtMode));
		entries.push_back(MapperStateEntry("$" + HexUtilities::ToHex(addr) + ".2-3", "Ext. Mode FPGA RAM Source", "$" + HexUtilities::ToHex(ctrl.FpgaRamSrc * 0x400), ctrl.FpgaRamSrc));
		entries.push_back(MapperStateEntry("$" + HexUtilities::ToHex(addr) + ".5", "Fill Mode", ctrl.FillMode));

		string src;
		switch(ctrl.Source) {
			case 0: src = "Nametable RAM"; break;
			case 1: src = "CHR RAM"; break;
			case 2: src = "FPGA RAM"; break;
			case 3: src = "CHR ROM"; break;
		}
		entries.push_back(MapperStateEntry("$412A.6-7", "Source", src, ctrl.Source));
	};

	for(int i = 0; i < 4; i++) {
		entries.push_back(MapperStateEntry("$" + HexUtilities::ToHex(0x4126 + i) + "/" + HexUtilities::ToHex(0x412A + i).substr(3, 1), "Nametable " + std::to_string(i) + " ($" + HexUtilities::ToHex(0x2000 + i * 0x400) + ")"));
		entries.push_back(MapperStateEntry("$" + HexUtilities::ToHex(0x4126 + i), "Selected Bank", _ntBanks[i], MapperStateValueType::Number8));
		processNtControl(0x412A + i, _ntControl[i]);
	}

	entries.push_back(MapperStateEntry("", "Window"));
	entries.push_back(MapperStateEntry("$412E", "Window Bank", _windowBank, MapperStateValueType::Number8));
	processNtControl(0x412F, _windowControl);

	entries.push_back(MapperStateEntry("", "PPU Mappings"));
	for(int i = 0; i < 16; i++) {
		entries.push_back(MapperStateEntry(
			"$" + HexUtilities::ToHex(0x4130 + i) + "/" + HexUtilities::ToHex(0x4140 + i),
			"CHR Bank " + std::to_string(i),
			_chrBanks[i],
			MapperStateValueType::Number16
		));
	}

	entries.push_back(MapperStateEntry("", "Scanline IRQ"));
	entries.push_back(MapperStateEntry("$4150", "Target Scanline", _slIrqScanline, MapperStateValueType::Number8));
	entries.push_back(MapperStateEntry("$4151/2", "Enabled", _slIrqEnabled));
	entries.push_back(MapperStateEntry("$4153", "Cycle Offset", _slIrqOffset, MapperStateValueType::Number8));
	entries.push_back(MapperStateEntry("$4154", "Jitter Counter", _jitterCounter, MapperStateValueType::Number8));

	entries.push_back(MapperStateEntry("", "CPU IRQ"));
	entries.push_back(MapperStateEntry("$4158/9", "Reload Value", _cpuIrqReloadValue, MapperStateValueType::Number16));
	entries.push_back(MapperStateEntry("$415A.0", "Enabled", _cpuIrqEnabled));
	entries.push_back(MapperStateEntry("$415A.1", "Enable after Ack", _cpuIrqEnableAfterAck));
	entries.push_back(MapperStateEntry("$415A.2", "Ack after $4011 read", _cpuIrqAckOn4011));
	entries.push_back(MapperStateEntry("", "Counter", _cpuIrqCounter, MapperStateValueType::Number16));

	entries.push_back(MapperStateEntry("", "FPGA RAM"));
	entries.push_back(MapperStateEntry("$415C/D", "Address", _fpgaRamAddr, MapperStateValueType::Number16));
	entries.push_back(MapperStateEntry("$415E", "Increment", _fpgaRamInc, MapperStateValueType::Number8));

	entries.push_back(MapperStateEntry("$4160", "Mapper Version", 0x20, MapperStateValueType::Number8));
	entries.push_back(MapperStateEntry("$4161.0", "Wi-Fi IRQ Pending", _wifiIrqPending));
	entries.push_back(MapperStateEntry("$4161.6", "CPU IRQ Pending", _cpuIrqPending));
	entries.push_back(MapperStateEntry("$4161.7", "Scanline IRQ Pending", _slIrqPending));

	entries.push_back(MapperStateEntry("", "IRQ/NMI Vectors"));
	entries.push_back(MapperStateEntry("$416B.0", "NMI Redirection Enabled", _nmiVectorEnabled));
	entries.push_back(MapperStateEntry("$416B.1", "IRQ Redirection Enabled", _irqVectorEnabled));
	entries.push_back(MapperStateEntry("$416C/D", "NMI Vector", _nmiVectorAddr, MapperStateValueType::Number16));
	entries.push_back(MapperStateEntry("$416E/F", "IRQ Vector", _irqVectorAddr, MapperStateValueType::Number16));

	entries.push_back(MapperStateEntry("", "Window Split Config"));
	entries.push_back(MapperStateEntry("$4170.0-4", "Start Column", _windowX1, MapperStateValueType::Number8));
	entries.push_back(MapperStateEntry("$4171.0-4", "End Column", _windowX2, MapperStateValueType::Number8));
	entries.push_back(MapperStateEntry("$4172", "Start Scanline", _windowY1, MapperStateValueType::Number8));
	entries.push_back(MapperStateEntry("$4173", "End Scanline", _windowY2, MapperStateValueType::Number8));
	entries.push_back(MapperStateEntry("$4174.0-4", "Scroll X", _windowScrollX, MapperStateValueType::Number8));
	entries.push_back(MapperStateEntry("$4175", "Scroll Y", _windowScrollX, MapperStateValueType::Number8));

	entries.push_back(MapperStateEntry("", "Wi-Fi"));
	entries.push_back(MapperStateEntry("$4190.0", "ESP Enabled", _espEnabled));
	entries.push_back(MapperStateEntry("$4190.1", "Wi-Fi IRQ Enabled", _wifiIrqEnabled));

	entries.push_back(MapperStateEntry("$4191.6", "Data Ready", _dataReady));
	entries.push_back(MapperStateEntry("$4191.7", "Data Received", _dataReceived));
	entries.push_back(MapperStateEntry("$4192.7", "Data Sent", _dataSent));
	entries.push_back(MapperStateEntry("$4193", "Receive Destination Page", _recvDstAddr, MapperStateValueType::Number8));
	entries.push_back(MapperStateEntry("$4194", "Send Source Page", _sendSrcAddr, MapperStateValueType::Number8));

	entries.push_back(MapperStateEntry("", "Sprite Extended Data"));
	for(int i = 0; i < 64; i++) {
		entries.push_back(MapperStateEntry("$" + HexUtilities::ToHex(0x4200 + i), "Sprite #" + std::to_string(i), _spriteExtData[i], MapperStateValueType::Number8));
	}
	entries.push_back(MapperStateEntry("$4240", "Sprite Extended Bank", _spriteExtBank, MapperStateValueType::Number8));

	entries.push_back(MapperStateEntry("", "OAM Functions"));
	entries.push_back(MapperStateEntry("$4241", "OAM Update Page", _oamSlowUpdatePage, MapperStateValueType::Number8));
	entries.push_back(MapperStateEntry("$4242", "OAM Ext. Update Page", _oamExtUpdatePage, MapperStateValueType::Number8));

	//todo audio?

	return entries;
}

bool Rainbow::HasExtendedAttributes(ExtModeConfig& cfg, uint8_t ntIndex)
{
	return cfg.Nametables[ntIndex].AttrExtMode;
}

bool Rainbow::HasExtendedBackground(ExtModeConfig& cfg, uint8_t ntIndex)
{
	return cfg.Nametables[ntIndex].BgExtMode;
}

bool Rainbow::HasExtendedSprites(ExtModeConfig& cfg)
{
	return cfg.SpriteExtMode;
}

uint8_t Rainbow::DebugReadChr(ExtModeConfig& cfg, uint32_t addr)
{
	switch(cfg.ChrSource) {
		default:
		case 0: return _chrRomSize ? _chrRom[addr & (_chrRomSize - 1)] : 0;
		case 1: return _chrRamSize ? _chrRam[addr & (_chrRamSize - 1)] : 0;

		case 2:
		case 3:
			return cfg.ExtRam[addr & 0x1FFF];
	}
}

uint8_t Rainbow::GetExAttributePalette(ExtModeConfig& cfg, uint8_t ntIndex, uint16_t ntOffset)
{
	uint8_t extData = cfg.ExtRam[cfg.Nametables[ntIndex].SourceOffset + ntOffset];
	return (extData & 0xC0) >> 6;
}

uint8_t Rainbow::GetExBackgroundChrData(ExtModeConfig& cfg, uint8_t ntIndex, uint16_t ntOffset, uint16_t chrAddr)
{
	uint8_t extData = cfg.ExtRam[cfg.Nametables[ntIndex].SourceOffset + ntOffset];
	uint32_t fetchAddr = (chrAddr & 0xFFF) | ((extData & 0x3F) << 12) | (cfg.BgExtBank << 18);
	return DebugReadChr(cfg, fetchAddr);
}

uint8_t Rainbow::GetExSpriteChrData(ExtModeConfig& cfg, uint8_t spriteIndex, uint16_t chrAddr)
{
	uint32_t addr;
	if(_largeSprites) {
		addr = (cfg.SpriteExtBank << 21) | (cfg.SpriteExtData[spriteIndex] << 13) | (chrAddr & 0x1FFF);
	} else {
		addr = (cfg.SpriteExtBank << 20) | (cfg.SpriteExtData[spriteIndex] << 12) | (chrAddr & 0xFFF);
	}
	return DebugReadChr(cfg, addr);
}

ExtModeConfig Rainbow::GetExModeConfig()
{
	ExtModeConfig cfg = {};

	for(int i = 0; i < 4; i++) {
		cfg.Nametables[i].SourceOffset = _ntControl[i].FpgaRamSrc * 0x400;
		cfg.Nametables[i].AttrExtMode = _ntControl[i].AttrExtMode;
		cfg.Nametables[i].BgExtMode = _ntControl[i].BgExtMode;
		cfg.Nametables[i].FillMode = _ntControl[i].FillMode;
	}

	cfg.Nametables[4].SourceOffset = _windowControl.FpgaRamSrc * 0x400;
	cfg.Nametables[4].AttrExtMode = _windowControl.AttrExtMode;
	cfg.Nametables[4].BgExtMode = _windowControl.BgExtMode;
	cfg.Nametables[4].FillMode = _windowControl.FillMode;

	cfg.WindowScrollX = _windowScrollX * 8;
	cfg.WindowScrollY = _windowScrollY;

	memcpy(cfg.ExtRam, _mapperRam, 0x1000);
	cfg.SpriteExtMode = _spriteExtMode;
	cfg.BgExtBank = _bgExtModeOffset;
	cfg.ChrSource = _chrSource;

	memcpy(cfg.SpriteExtData, _spriteExtData, sizeof(_spriteExtData));
	cfg.SpriteExtBank = _spriteExtBank;

	return cfg;
}

void Rainbow::Serialize(Serializer& s)
{
	BaseMapper::Serialize(s);

	SVArray(_highBanks, 8);
	SVArray(_lowBanks, 2);
	SVArray(_chrBanks, 16);
	SV(_fpgaRamBank);
	SV(_highMode);
	SV(_lowMode);
	SV(_chrMode);
	SV(_chrSource);
	SV(_windowEnabled);
	SV(_spriteExtMode);
	SV(_bgExtModeOffset);
	SVArray(_ntBanks, 4);

	for(int i = 0; i < 4; i++) {
		SVI(_ntControl[i].AttrExtMode);
		SVI(_ntControl[i].BgExtMode);
		SVI(_ntControl[i].FillMode);
		SVI(_ntControl[i].FpgaRamSrc);
		SVI(_ntControl[i].Source);
	}

	SV(_fillModeTileIndex);
	SV(_fillModeAttrIndex);

	SV(_windowControl.AttrExtMode);
	SV(_windowControl.BgExtMode);
	SV(_windowControl.FillMode);
	SV(_windowControl.FpgaRamSrc);
	SV(_windowControl.Source);

	SV(_windowBank);
	SV(_windowX1);
	SV(_windowX2);
	SV(_windowY1);
	SV(_windowY2);
	SV(_windowScrollX);
	SV(_windowScrollY);
	SV(_inWindow);
	SV(_slIrqEnabled);
	SV(_slIrqPending);
	SV(_slIrqScanline);
	SV(_slIrqOffset);
	SV(_lastPpuReadAddr);
	SV(_scanlineCounter);
	SV(_ppuIdleCounter);
	SV(_ntReadCounter);
	SV(_ppuReadCounter);
	SV(_inFrame);
	SV(_inHBlank);
	SV(_jitterCounter);
	SV(_cpuIrqCounter);
	SV(_cpuIrqReloadValue);
	SV(_cpuIrqEnabled);
	SV(_cpuIrqPending);
	SV(_cpuIrqEnableAfterAck);
	SV(_cpuIrqAckOn4011);
	SV(_fpgaRamAddr);
	SV(_fpgaRamInc);
	SV(_nmiVectorEnabled);
	SV(_irqVectorEnabled);
	SV(_nmiVectorAddr);
	SV(_irqVectorAddr);
	SV(_overrideTileFetch);
	SV(_extData);
	SV(_ntFetchCounter);
	SVArray(_spriteExtData, 64);
	SVArray(_oamPosY, 64);
	SVArray(_oamMappings, 8);
	SV(_spriteExtBank);
	SV(_largeSprites);
	SV(_oamAddr);
	SV(_oamExtUpdatePage);
	SV(_oamSlowUpdatePage);
	SVArray(_oamCode, 0x506);
	SV(_oamCodeLocked);

	SV(_espEnabled);
	SV(_wifiIrqEnabled);
	SV(_wifiIrqPending);
	SV(_dataSent);
	SV(_dataReceived);
	SV(_dataReady);
	SV(_sendSrcAddr);
	SV(_recvDstAddr);

	SV(_prgFlash);
	SV(_chrFlash);

	SV(_audio);
}

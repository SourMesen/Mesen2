#pragma once
#include "stdafx.h"
#include "MemoryOperationType.h"
#include "PCE/PceConsole.h"
#include "PCE/PcePpu.h"
#include "PCE/PceTimer.h"
#include "PCE/PcePsg.h"
#include "PCE/PceControlManager.h"
#include "PCE/PceSf2RomMapper.h"
#include "PCE/PceCdRom.h"
#include "Debugger/DebugTypes.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/MessageManager.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/RandomHelper.h"

class Emulator;

class PceMemoryManager
{
private:
	Emulator* _emu = nullptr;
	PceConsole* _console = nullptr;
	PcePpu* _ppu = nullptr;
	PcePsg* _psg = nullptr;
	PceControlManager* _controlManager = nullptr;
	PceCdRom* _cdrom = nullptr;
	unique_ptr<PceSf2RomMapper> _mapper;
	unique_ptr<PceTimer> _timer;

	PceMemoryManagerState _state = {};
	uint8_t* _prgRom = nullptr;
	uint32_t _prgRomSize = 0;
	
	uint8_t* _readBanks[0x100] = {};
	uint8_t* _writeBanks[0x100] = {};
	MemoryType _bankMemType[0x100] = {};

	uint8_t* _workRam = nullptr;
	uint32_t _workRamSize = 0x2000;

	uint8_t* _cdromRam = nullptr;
	uint32_t _cdromRamSize = 0x10000;

	uint8_t* _cardRam = nullptr;
	uint32_t _cardRamSize = 0x30000;
	
	uint8_t* _saveRam = nullptr;
	uint32_t _saveRamSize = 0x2000;

	uint8_t* _unmappedBank = nullptr;

public:
	PceMemoryManager(Emulator* emu, PceConsole* console, PcePpu* ppu, PceControlManager* controlManager, PcePsg* psg, PceCdRom* cdrom, vector<uint8_t> romData)
	{
		_emu = emu;
		_console = console;
		_ppu = ppu;
		_psg = psg;
		_cdrom = cdrom;
		_controlManager = controlManager;
		_prgRomSize = (uint32_t)romData.size();
		_prgRom = new uint8_t[_prgRomSize];
		_workRam = new uint8_t[_workRamSize];
		
		_unmappedBank = new uint8_t[0x2000];
		memset(_unmappedBank, 0xFF, 0x2000);

		memcpy(_prgRom, romData.data(), _prgRomSize);
		
		//TODO random
		memset(_workRam, 0, _workRamSize);
		_emu->GetSettings()->InitializeRam(_workRam, _workRamSize);

		if(_cdrom) {
			_saveRam = new uint8_t[_saveRamSize];
			_cdromRam = new uint8_t[_cdromRamSize];
			_cardRam = new uint8_t[_cardRamSize];
			//TODO random
			memset(_saveRam, 0, _saveRamSize);
			memset(_cdromRam, 0, _cdromRamSize);
			memset(_cardRam, 0, _cardRamSize);
			_emu->RegisterMemory(MemoryType::PceSaveRam, _saveRam, _saveRamSize);
			_emu->RegisterMemory(MemoryType::PceCdromRam, _cdromRam, _cdromRamSize);
			_emu->RegisterMemory(MemoryType::PceCardRam, _cardRam, _cardRamSize);

			_saveRam[0] = 0x48;
			_saveRam[1] = 0x55;
			_saveRam[2] = 0x42;
			_saveRam[3] = 0x4D;
			_saveRam[4] = 0x00;
			_saveRam[5] = 0xA0;
			_saveRam[6] = 0x10;
			_saveRam[7] = 0x80;
		}

		_emu->RegisterMemory(MemoryType::PcePrgRom, _prgRom, _prgRomSize);
		_emu->RegisterMemory(MemoryType::PceWorkRam, _workRam, _workRamSize);

		_timer.reset(new PceTimer(this));

		//CPU boots up in slow speed
		_state.FastCpuSpeed = false;

		//Set to 0 on power on
		_state.DisabledIrqs = 0;

		//MPR 7 is set to 0 on power on
		_state.Mpr[7] = 0;

		//Other MPR registers are random
		for(int i = 0; i < 7; i++) {
			_state.Mpr[i] = RandomHelper::GetValue(0, 255);
		}

		//Map ROM to all 128 banks
		uint32_t bankCount = _prgRomSize / 0x2000;

		if(bankCount == 0x30) {
			//384KB games - Mirrored as 256x2, 128x4
			uint32_t bankOffsets[8] = { 0, 0x10, 0, 0x10, 0x20, 0x20, 0x20, 0x20 };
			UpdateMappings(bankOffsets);
		} else if(bankCount == 0x40) {
			//512KB games - Mirrored as either 256x2 + 256x2, or 512x2
			//Mirror them as 256 + 256x3 for compatibility
			uint32_t bankOffsets[8] = { 0, 0x10, 0x20, 0x30, 0x20, 0x30, 0x20, 0x30 };
			UpdateMappings(bankOffsets);
		} else if(bankCount == 0x60) {
			//768KB games - Mirrored as 512x1 + 256x2
			uint32_t bankOffsets[8] = { 0, 0x10, 0x20, 0x30, 0x40, 0x50, 0x40, 0x50 };
			UpdateMappings(bankOffsets);
		} else {
			//Use default mirroring for anything else
			uint32_t bankOffsets[8] = { 0, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70 };
			UpdateMappings(bankOffsets);
		}

		if(_prgRomSize > 0x80 * 0x2000) {
			//For ROMs over 1MB, assume this is the SF2 board
			_mapper.reset(new PceSf2RomMapper(this));
		}
	}

	~PceMemoryManager()
	{
		delete[] _prgRom;
		delete[] _workRam;
		delete[] _saveRam;
		delete[] _cdromRam;
		delete[] _cardRam;
		delete[] _unmappedBank;
	}

	PceMemoryManagerState& GetState()
	{
		return _state;
	}

	void SetSpeed(bool slow)
	{
		_state.FastCpuSpeed = !slow;
	}

	void UpdateMappings(uint32_t bankOffsets[8])
	{
		int bankCount = _prgRomSize / 0x2000;
		for(int i = 0; i <= 0x7F; i++) {
			//00 - 7F
			int bank = (bankOffsets[i >> 4] + (i & 0x0F)) % bankCount;
			_readBanks[i] = _prgRom + (bank * 0x2000);
			_bankMemType[i] = MemoryType::PcePrgRom;
		}

		for(int i = 0x80; i <= 0xFF; i++) {
			//80 - FF
			_readBanks[i] = _unmappedBank;
			_bankMemType[i] = MemoryType::Register;
		}

		for(int i = 0; i < 4; i++) {
			//F8 - FB
			_readBanks[0xF8 + i] = _workRam;
			_writeBanks[0xF8 + i] = _workRam;
			_bankMemType[0xF8 + i] = MemoryType::PceWorkRam;
		}

		if(_cdrom) {
			for(int i = 0; i < 8; i++) {
				//80 - 87
				_readBanks[0x80 + i] = _cdromRam + (i * 0x2000);
				_writeBanks[0x80 + i] = _cdromRam + (i * 0x2000);
				_bankMemType[0x80 + i] = MemoryType::PceCdromRam;
			}

			for(int i = 0; i < 0x18; i++) {
				//68 - 7F
				_readBanks[0x68 + i] = _cardRam + (i * 0x2000);
				_writeBanks[0x68 + i] = _cardRam + (i * 0x2000);
				_bankMemType[0x68 + i] = MemoryType::PceCardRam;
			}

			//F7 - BRAM
			_readBanks[0xF7] = _saveRam;
			_writeBanks[0xF7] = _saveRam;
			_bankMemType[0xF7] = MemoryType::PceSaveRam;
		}
	}

	void Exec()
	{
		if(_state.FastCpuSpeed) {
			_state.CycleCount += 3;
			_timer->Exec();
			_ppu->Exec();
			if(_cdrom) {
				_cdrom->Exec();
			}
		} else {
			ExecSlow();
		}
	}

	void ExecSlow()
	{
		for(int i = 0; i < 4; i++) {
			_state.CycleCount += 3;
			_timer->Exec();
			_ppu->Exec();
			if(_cdrom) {
				_cdrom->Exec();
			}
		}
	}

	uint8_t ReadRegister(uint16_t addr)
	{
		if(addr <= 0x3FF) {
			//VDC
			Exec(); //CPU is delayed by 1 CPU cycle when reading/writing to VDC/VCE
			return _ppu->ReadVdc(addr);
		} else if(addr <= 0x7FF) {
			//VCE
			Exec(); //CPU is delayed by 1 CPU cycle when reading/writing to VDC/VCE
			return _ppu->ReadVce(addr);
		} else if(addr <= 0xBFF) {
			//PSG
			return _state.IoBuffer;
		} else if(addr <= 0xFFF) {
			//Timer
			_state.IoBuffer = (_state.IoBuffer & 0x80) | (_timer->Read(addr) & 0x7F);
			return _state.IoBuffer;
		} else if(addr <= 0x13FF) {
			//IO
			_state.IoBuffer = 0xB0 | _controlManager->ReadInputPort();
			return _state.IoBuffer;
		} else if(addr <= 0x17FF) {
			//IRQ
			switch(addr & 0x03) {
				case 0:
				case 1:
					break;

				case 2: _state.IoBuffer = (_state.IoBuffer & 0xF8) | (_state.DisabledIrqs & 0x07); break;
				case 3: _state.IoBuffer = (_state.IoBuffer & 0xF8) | (_state.ActiveIrqs & 0x07); break;
			}
			return _state.IoBuffer;
		} else if(addr <= 0x1BFF) {
			return _cdrom ? _cdrom->Read(addr) : 0xFF;
		}

		LogDebug("[Debug] Read - missing handler: $" + HexUtilities::ToHex(addr));
		return 0xFF;
	}

	__forceinline uint8_t Read(uint16_t addr, MemoryOperationType type = MemoryOperationType::Read)
	{
		uint8_t bank = _state.Mpr[(addr & 0xE000) >> 13];
		uint8_t value;
		if(bank != 0xFF) {
			value = _readBanks[bank][addr & 0x1FFF];
		} else {
			value = ReadRegister(addr & 0x1FFF);
		}
		_emu->ProcessMemoryRead<CpuType::Pce>(addr, value, type);
		return value;
	}

	uint8_t DebugRead(uint16_t addr)
	{
		uint8_t bank = _state.Mpr[(addr & 0xE000) >> 13];
		addr &= 0x1FFF;
		if(bank != 0xFF) {
			return _readBanks[bank][addr];
		} else {
			if(addr <= 0x3FF) {
				//VDC
			} else if(addr <= 0x7FF) {
				//VCE
			} else if(addr <= 0xBFF) {
				//PSG
			} else if(addr <= 0xFFF) {
				//Timer
			} else if(addr <= 0x13FF) {
				//IO
			} else if(addr <= 0x17FF) {
				//IRQ
				switch(addr & 0x03) {
					case 2: return _state.DisabledIrqs;
					case 3: return _state.ActiveIrqs;
				}
			}
		}
		return 0xFF;
	}

	__forceinline void Write(uint16_t addr, uint8_t value, MemoryOperationType type)
	{
		_emu->ProcessMemoryWrite<CpuType::Pce>(addr, value, type);
		
		if(_mapper) {
			_mapper->Write(addr, value);
		}

		uint8_t bank = _state.Mpr[(addr & 0xE000) >> 13];
		addr &= 0x1FFF;
		if(bank != 0xFF) {
			if(_writeBanks[bank]) {
				_writeBanks[bank][addr] = value;
			}
		} else {
			if(addr <= 0x3FF) {
				_ppu->WriteVdc(addr, value);
				Exec(); //CPU is delayed by 1 CPU cycle when reading/writing to VDC/VCE
			} else if(addr <= 0x7FF) {
				_ppu->WriteVce(addr, value);
				Exec(); //CPU is delayed by 1 CPU cycle when reading/writing to VDC/VCE
			} else if(addr <= 0xBFF) {
				//PSG
				_psg->Write(addr, value);
				_state.IoBuffer = value;
			} else if(addr <= 0xFFF) {
				//Timer
				_timer->Write(addr, value);
				_state.IoBuffer = value;
			} else if(addr <= 0x13FF) {
				//IO
				_controlManager->WriteInputPort(value);
				_state.IoBuffer = value;
			} else if(addr <= 0x17FF) {
				//IRQ
				switch(addr & 0x03) {
					case 0:
					case 1:
						LogDebug("[Debug] Write - missing handler: $" + HexUtilities::ToHex(addr) + " = " + HexUtilities::ToHex(value));
						break;

					case 2: _state.DisabledIrqs = value & 0x07; break;
					case 3: ClearIrqSource(PceIrqSource::TimerIrq); break;
				}
				_state.IoBuffer = value;
			} else if(addr <= 0x1BFF) {
				if(_cdrom) {
					_cdrom->Write(addr, value);
				}
			} else {
				LogDebug("[Debug] Write - missing handler: $" + HexUtilities::ToHex(addr) + " = " + HexUtilities::ToHex(value));
			}
		}
	}

	void WriteVdc(uint16_t addr, uint8_t value)
	{
		_ppu->WriteVdc(addr, value);
		Exec(); //CPU is delayed by 1 CPU cycle when reading/writing to VDC/VCE
	}

	void SetMprValue(uint8_t regSelect, uint8_t value)
	{
		if(regSelect == 0) {
			return;
		}

		_state.MprReadBuffer = value;

		for(int i = 0; i < 8; i++) {
			if(regSelect & (1 << i)) {
				_state.Mpr[i] = value;
			}
		}
	}

	uint8_t GetMprValue(uint8_t regSelect)
	{
		if(regSelect == 0) {
			//"If an operand of $00 is used, the accumulator is loaded with the last value that was written with TAM"
			return _state.MprReadBuffer;
		}

		uint8_t value = 0;
		for(int i = 0; i < 8; i++) {
			//"If multiple bits are set in the operand to TMA, the values from several MPRs are combined togetherand returned."
			if(regSelect & (1 << i)) {
				value |= _state.Mpr[i];
			}
		}

		_state.MprReadBuffer = value;
		return value;
	}

	AddressInfo GetAbsoluteAddress(uint32_t relAddr)
	{
		uint8_t bank = _state.Mpr[(relAddr & 0xE000) >> 13];
		if(bank != 0xFF) {
			uint32_t absAddr;
			switch(_bankMemType[bank]) {
				case MemoryType::PcePrgRom: absAddr = (uint32_t)(_readBanks[bank] - _prgRom) + (relAddr & 0x1FFF); break;
				case MemoryType::PceWorkRam: absAddr = (uint32_t)(_readBanks[bank] - _workRam) + (relAddr & 0x1FFF); break;
				case MemoryType::PceSaveRam: absAddr = (uint32_t)(_readBanks[bank] - _saveRam) + (relAddr & 0x1FFF); break;
				case MemoryType::PceCdromRam: absAddr = (uint32_t)(_readBanks[bank] - _cdromRam) + (relAddr & 0x1FFF); break;
				case MemoryType::PceCardRam: absAddr = (uint32_t)(_readBanks[bank] - _cardRam) + (relAddr & 0x1FFF); break;
				default: return { -1, MemoryType::Register };
			}
			return { (int32_t)absAddr, _bankMemType[bank] };
		}/* else if(bank == 0xFF) { //TODO
			return { (int32_t)relAddr & 0x1FFF, MemoryType::Register };
		}*/ else {
			return { -1, MemoryType::Register };
		}
	}

	AddressInfo GetRelativeAddress(AddressInfo absAddr)
	{
		for(int i = 0; i < 8; i++) {
			AddressInfo bankStart = GetAbsoluteAddress(i * 0x2000);
			if(bankStart.Type == absAddr.Type && bankStart.Address == (absAddr.Address & ~0x1FFF)) {
				return { (i << 13) | (absAddr.Address & 0x1FFF), MemoryType::PceMemory };
			}/* else if(absAddr.Type == MemoryType::Register) { //TODO
				if(_state.Mpr[i] == 0xFF) {
					return { (i << 13) | (absAddr.Address & 0x1FFF), MemoryType::PceMemory };
				}
			}*/
		}

		return { -1, MemoryType::Register };
	}

	void SetIrqSource(PceIrqSource source)
	{
		_state.ActiveIrqs |= (int)source;
	}

	__forceinline bool HasPendingIrq()
	{
		return (_state.ActiveIrqs & ~_state.DisabledIrqs) != 0;
	}

	__forceinline bool HasIrqSource(PceIrqSource source)
	{
		return (_state.ActiveIrqs & ~_state.DisabledIrqs & (int)source) != 0;
	}

	void ClearIrqSource(PceIrqSource source)
	{
		_state.ActiveIrqs &= ~(int)source;
	}
};
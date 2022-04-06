#pragma once
#include "stdafx.h"
#include "MemoryOperationType.h"
#include "PCE/PcePpu.h"
#include "PCE/PceTimer.h"
#include "PCE/PceControlManager.h"
#include "Debugger/DebugTypes.h"
#include "Shared/Emulator.h"
#include "Shared/MessageManager.h"
#include "Utilities/HexUtilities.h"

class Emulator;

class PceMemoryManager
{
private:
	Emulator* _emu;
	PcePpu* _ppu;
	PceControlManager* _controlManager;
	unique_ptr<PceTimer> _timer;

	PceMemoryManagerState _state = {};
	uint8_t* _prgRom;
	uint32_t _prgRomSize;
	
	uint8_t _maxBank;
	uint8_t _mpr[8] = {};
	uint8_t _mappedBank[8] = {};

	uint8_t* _workRam;
	uint32_t _workRamSize = 0x2000;

public:
	PceMemoryManager(Emulator* emu, PcePpu* ppu, PceControlManager* controlManager, vector<uint8_t> romData)
	{
		_emu = emu;
		_ppu = ppu;
		_controlManager = controlManager;
		_prgRomSize = (uint32_t)romData.size();
		_prgRom = new uint8_t[_prgRomSize];
		_maxBank = _prgRomSize / 0x2000;

		_workRam = new uint8_t[_workRamSize];

		memcpy(_prgRom, romData.data(), _prgRomSize);

		memset(_workRam, 0, _workRamSize);

		_emu->RegisterMemory(MemoryType::PcePrgRom, _prgRom, _prgRomSize);
		_emu->RegisterMemory(MemoryType::PceWorkRam, _workRam, _workRamSize);

		_timer.reset(new PceTimer(this));
	}

	~PceMemoryManager()
	{
		delete[] _prgRom;
		delete[] _workRam;
	}

	PceMemoryManagerState& GetState()
	{
		return _state;
	}

	void SetSpeed(bool slow)
	{
		_state.CpuClockSpeed = slow ? 12 : 3;
	}

	void Exec()
	{
		_state.CycleCount += _state.CpuClockSpeed;
		_timer->Exec(_state.CpuClockSpeed);
		_ppu->Exec();
	}

	uint8_t Read(uint16_t addr, MemoryOperationType type = MemoryOperationType::Read)
	{
		uint8_t bank = _mappedBank[(addr & 0xE000) >> 13];
		uint8_t value;
		if(bank <= 0x7F) {
			uint32_t absAddr = (((bank % _maxBank) << 13) | (addr & 0x1FFF));
			value = _prgRom[absAddr];
		} else if(bank >= 0xF8 && bank <= 0xFB) {
			value = _workRam[addr & 0x1FFF];
		} else if(bank == 0xFF) {
			if(addr <= 0x3FF) {
				//VDC
				value = _ppu->ReadVdc(addr);
			} else if(addr <= 0x7FF) {
				//VCE
				value = _ppu->ReadVce(addr);
			} else if(addr <= 0xBFF) {
				//PSG
				value = _state.IoBuffer;
			} else if(addr <= 0xFFF) {
				//Timer
				_state.IoBuffer = (_state.IoBuffer & 0x80) | (_timer->Read(addr) & 0x7F);
				value = _state.IoBuffer;
			} else if(addr <= 0x13FF) {
				//IO
				_state.IoBuffer = value = 0xB0 | _controlManager->ReadInputPort();
				value = _state.IoBuffer;
			} else if(addr <= 0x17FF) {
				//IRQ
				switch(addr & 0x03) {
					case 0:
					case 1:
						break;

					case 2: _state.IoBuffer = (_state.IoBuffer & 0xF8) | (_state.DisabledIrqs & 0x07); break;
					case 3: _state.IoBuffer = (_state.IoBuffer & 0xF8) | (_state.ActiveIrqs & 0x07); break;
				}
				value = _state.IoBuffer;
			} else {
				value = 0xFF;
				LogDebug("[Debug] Read - missing handler: $" + HexUtilities::ToHex(addr));
			}
		} else {
			value = 0xFF;
			LogDebug("[Debug] Read unmapped memory: $" + HexUtilities::ToHex(addr));
		}
		_emu->ProcessMemoryRead<CpuType::Pce>(addr, value, type);
		return value;
	}

	uint8_t DebugRead(uint16_t addr)
	{
		uint8_t bank = _mappedBank[(addr & 0xE000) >> 13];
		uint8_t value = 0;
		if(bank <= 0x7F) {
			uint32_t absAddr = (((bank % _maxBank) << 13) | (addr & 0x1FFF));
			value = _prgRom[absAddr];
		} else if(bank >= 0xF8 && bank <= 0xFB) {
			value = _workRam[addr & 0x1FFF];
		} else if(bank == 0xFF) {
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
		} else {
			value = 0xFF;
		}
		return value;
	}

	void Write(uint16_t addr, uint8_t value, MemoryOperationType type)
	{
		_emu->ProcessMemoryWrite<CpuType::Pce>(addr, value, type);
		
		uint8_t bank = _mpr[(addr & 0xE000) >> 13];
		addr &= 0x1FFF;
		if(bank <= 0x7F) {
			//ROM
		} else if(bank >= 0xF8 && bank <= 0xFB) {
			_workRam[addr] = value;
		} else if(bank == 0xFF) {
			if(addr <= 0x3FF) {
				_ppu->WriteVdc(addr, value);
			} else if(addr <= 0x7FF) {
				_ppu->WriteVce(addr, value);
			} else if(addr <= 0xBFF) {
				//PSG
				//LogDebug("[Debug] Write PSG - missing handler: $" + HexUtilities::ToHex(addr) + " = " + HexUtilities::ToHex(value));
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
			} else {
				LogDebug("[Debug] Write - missing handler: $" + HexUtilities::ToHex(addr) + " = " + HexUtilities::ToHex(value));
			}
		} else {
			//Open bus?
		}
	}

	void WriteVdc(uint16_t addr, uint8_t value)
	{
		_ppu->WriteVdc(addr, value);
	}

	void SetMprValue(uint8_t regSelect, uint8_t value)
	{
		for(int i = 0; i < 8; i++) {
			if(regSelect & (1 << i)) {
				_mpr[i] = value;

				if(_mpr[i] <= 0x7F) {
					_mappedBank[i] = value % _maxBank;
					if(value >= _maxBank) {
						uint32_t power = (uint32_t)std::log2(_maxBank);
						if(_maxBank > (1u << power)) {
							uint32_t halfSize = 1 << power;
							uint32_t fullSize = 1 << (power + 1);
							value -= halfSize;
							value %= (_maxBank - halfSize);
							value += halfSize;
							_mappedBank[i] = value;
						}
					}
				} else {
					_mappedBank[i] = value;
				}
			}
		}
	}

	uint8_t GetMprValue(uint8_t regSelect)
	{
		uint8_t value = 0;
		for(int i = 0; i < 8; i++) {
			if(regSelect & (1 << i)) {
				value |= _mpr[i];
			}
		}
		return value;
	}

	AddressInfo GetAbsoluteAddress(uint32_t relAddr)
	{
		uint8_t bank = _mpr[(relAddr & 0xE000) >> 13];
		if(bank >= 0xF8 && bank <= 0xFB) {
			return { (int32_t)(relAddr & 0x1FFF), MemoryType::PceWorkRam };
		} else {
			uint32_t absAddr = (((bank % _maxBank) << 13) | (relAddr & 0x1FFF));
			return { (int32_t)absAddr, MemoryType::PcePrgRom };
		}
	}

	AddressInfo GetRelativeAddress(AddressInfo absAddr)
	{
		for(int i = 0; i < 8; i++) {
			if(absAddr.Type == MemoryType::PcePrgRom) {
				if((_mpr[i] << 13) == (absAddr.Address & ~0x1FFF)) {
					return { (i << 13) | (absAddr.Address & 0x1FFF), MemoryType::PceMemory };
				}
			} else if(absAddr.Type == MemoryType::PceWorkRam) {
				if(_mpr[i] >= 0xF8 && _mpr[i] <= 0xFB) {
					return { (i << 13) | (absAddr.Address & 0x1FFF), MemoryType::PceMemory };
				}
			}
		}

		return { -1, MemoryType::Register };
	}

	void SetIrqSource(PceIrqSource source)
	{
		_state.ActiveIrqs |= (int)source;
	}

	bool HasIrqSource(PceIrqSource source)
	{
		return (_state.ActiveIrqs & ~_state.DisabledIrqs & (int)source) != 0;
	}

	void ClearIrqSource(PceIrqSource source)
	{
		_state.ActiveIrqs &= ~(int)source;
	}
};
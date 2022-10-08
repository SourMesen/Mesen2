#include "pch.h"
#include "SNES/InternalRegisters.h"
#include "SNES/SnesConsole.h"
#include "SNES/SnesCpu.h"
#include "SNES/SnesPpu.h"
#include "SNES/SnesControlManager.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/InternalRegisterTypes.h"
#include "Shared/MessageManager.h"
#include "Utilities/Serializer.h"
#include "Utilities/HexUtilities.h"

InternalRegisters::InternalRegisters()
{
}

void InternalRegisters::Initialize(SnesConsole* console)
{
	_cpu = console->GetCpu();
	_aluMulDiv.Initialize(_cpu);
	_console = console;
	_memoryManager = console->GetMemoryManager();
	_ppu = _console->GetPpu();
	_controlManager = (SnesControlManager*)_console->GetControlManager();
	Reset();

	//Power on values
	_state = {};
	_state.HorizontalTimer = 0x1FF;
	_state.VerticalTimer = 0x1FF;
	_state.IoPortOutput = 0xFF;
}

void InternalRegisters::Reset()
{
	_state.EnableAutoJoypadRead = false;
	_state.EnableNmi = false;
	_state.EnableHorizontalIrq = false;
	_state.EnableVerticalIrq = false;
	_nmiFlag = false;
	_irqLevel = false;
	_needIrq = false;
	_irqFlag = false;
}

void InternalRegisters::ProcessAutoJoypadRead()
{
	if(_autoReadClockStart == 0) {
		//"[..] Specifically, it begins at dot 74.5 on the first frame" (74.5*4 = 298)
		_autoReadClockStart = _console->GetMasterClock() + 298;
	} else {
		//"This begins between dots 32.5 and 95.5 of the first V-Blank scanline" (32.5*4 = 130)
		//"and thereafter some multiple of 256 cycles after the start of the previous read that falls within the observed range."
		uint64_t rangeStart = (_console->GetMasterClock() + 130);
		uint64_t elapsed = rangeStart - _autoReadClockStart;
		uint64_t gap = elapsed % 256;
		_autoReadClockStart = rangeStart + gap;
	}

	if(!_state.EnableAutoJoypadRead) {
		return;
	}

	//TODO what happens if CPU reads or writes to 4016/4017 while this is running?

	//If bit 0 was set to 1 by a CPU write, auto-read can't set the value back to 0
	//causing the controllers to continuously report the value of the B button
	if((_controlManager->GetLastWriteValue() & 0x01) == 0) {
		_controlManager->Write(0x4016, 1);
		_controlManager->Write(0x4016, 0);
	}

	for(int i = 0; i < 4; i++) {
		_state.ControllerData[i] = _newControllerData[i];
		_newControllerData[i] = 0;
	}

	for(int i = 0; i < 16; i++) {
		uint8_t port1 = _controlManager->Read(0x4016, true);
		uint8_t port2 = _controlManager->Read(0x4017, true);

		_newControllerData[0] <<= 1;
		_newControllerData[1] <<= 1;
		_newControllerData[2] <<= 1;
		_newControllerData[3] <<= 1;

		_newControllerData[0] |= (port1 & 0x01);
		_newControllerData[1] |= (port2 & 0x01);
		_newControllerData[2] |= (port1 & 0x02) >> 1;
		_newControllerData[3] |= (port2 & 0x02) >> 1;
	}

	_newControllerDataPending = true;
}

bool InternalRegisters::IsAutoReadActive()
{
	//Auto-read is active for 4224 master cycles
	return _state.EnableAutoJoypadRead && _console->GetMasterClock() >= _autoReadClockStart && _console->GetMasterClock() < _autoReadClockStart + 4224;
}

uint8_t InternalRegisters::ReadControllerData(uint8_t port, bool getMsb)
{
	if(_newControllerDataPending && _console->GetMasterClock() > _autoReadClockStart + 4224) {
		for(int i = 0; i < 4; i++) {
			_state.ControllerData[i] = _newControllerData[i];
		}
		_newControllerDataPending = false;
	}

	//When auto-poll registers are read, don't count frame as a lag frame
	_controlManager->SetInputReadFlag();

	uint8_t value;
	if(getMsb) {
		value = (uint8_t)(_state.ControllerData[port] >> 8);
	} else {
		value = (uint8_t)_state.ControllerData[port];
	}

	if(IsAutoReadActive()) {
		if(value == 0) {
			//"The only reliable value is that no buttons pressed will return 0"
			return 0;
		} else {
			//TODO not accurate
			//"Reading $4218-f during this time will read back incorrect values."
			//Return bad data when reading during auto-read while buttons are pressed
			return (value ^ 0xFF) >> 2;
		}
	} else {
		return value;
	}
}

uint8_t InternalRegisters::GetIoPortOutput()
{
	return _state.IoPortOutput;
}

void InternalRegisters::SetNmiFlag(bool nmiFlag)
{
	_nmiFlag = nmiFlag;
}

void InternalRegisters::SetIrqFlag(bool irqFlag)
{
	_irqFlag = irqFlag && (_state.EnableHorizontalIrq || _state.EnableVerticalIrq);
	if(_irqFlag) {
		_cpu->SetIrqSource(SnesIrqSource::Ppu);
	} else {
		_cpu->ClearIrqSource(SnesIrqSource::Ppu);
	}
}

uint8_t InternalRegisters::Peek(uint16_t addr)
{
	switch(addr) {
		case 0x4210: return (_nmiFlag ? 0x80 : 0) | 0x02;
		case 0x4211: return (_irqFlag ? 0x80 : 0);

		case 0x4214:
		case 0x4215:
		case 0x4216:
		case 0x4217:
			//Not completely accurate because the ALU results are only 
			//updated when the CPU actually reads the registers
			return _aluMulDiv.Peek(addr);

		case 0x4218: return (uint8_t)_state.ControllerData[0];
		case 0x4219: return (uint8_t)(_state.ControllerData[0] >> 8);

		default: return Read(addr);
	}
}

uint8_t InternalRegisters::Read(uint16_t addr)
{
	switch(addr) {
		case 0x4210: {
			constexpr uint8_t cpuRevision = 0x02;
			
			uint8_t value = (_nmiFlag ? 0x80 : 0) | cpuRevision;

			//Reading $4210 on any cycle turns the NMI signal off (except presumably on the first PPU cycle (first 4 master clocks) of the NMI scanline.)
			//i.e: reading $4210 at the same it gets set will return it as set, and will keep it set.
			//Without this, Terranigma has corrupted sprites on some frames.
			if(_memoryManager->GetHClock() >= 4 || _ppu->GetScanline() != _ppu->GetNmiScanline()) {
				SetNmiFlag(false);
			}

			return value | (_memoryManager->GetOpenBus() & 0x70);
		}

		case 0x4211: {
			uint8_t value = (_irqFlag ? 0x80 : 0);
			SetIrqFlag(false);
			return value | (_memoryManager->GetOpenBus() & 0x7F);
		}

		case 0x4212: {
			uint16_t hClock = _memoryManager->GetHClock();
			uint16_t scanline = _ppu->GetScanline();
			uint16_t nmiScanline = _ppu->GetNmiScanline();
			//TODO TIMING (set/clear timing)
			return (
				(scanline >= nmiScanline ? 0x80 : 0) |
				((hClock >= 1*4 && hClock <= 274*4) ? 0 : 0x40) |
				(IsAutoReadActive() ? 0x01 : 0) | //Auto joypad read in progress
				(_memoryManager->GetOpenBus() & 0x3E)
			);
		}

		case 0x4213:
			//TODO  RDIO - Programmable I/O port (in-port)
			return 0;
						 
		case 0x4214:
		case 0x4215:
		case 0x4216:
		case 0x4217: 
			return _aluMulDiv.Read(addr);

		case 0x4218: return ReadControllerData(0, false);
		case 0x4219: return ReadControllerData(0, true);
		case 0x421A: return ReadControllerData(1, false);
		case 0x421B: return ReadControllerData(1, true);
		case 0x421C: return ReadControllerData(2, false);
		case 0x421D: return ReadControllerData(2, true);
		case 0x421E: return ReadControllerData(3, false);
		case 0x421F: return ReadControllerData(3, true);
		
		default:
			LogDebug("[Debug] Unimplemented register read: " + HexUtilities::ToHex(addr));
			return _memoryManager->GetOpenBus();
	}
}

void InternalRegisters::Write(uint16_t addr, uint8_t value)
{
	switch(addr) {
		case 0x4200:
			_state.EnableNmi = (value & 0x80) != 0;
			_state.EnableVerticalIrq = (value & 0x20) != 0;
			_state.EnableHorizontalIrq = (value & 0x10) != 0;
			_state.EnableAutoJoypadRead = (value & 0x01) != 0;
			
			SetNmiFlag(_nmiFlag);
			SetIrqFlag(_irqFlag);
			break;

		case 0x4201:
			//TODO WRIO - Programmable I/O port (out-port)
			if((_state.IoPortOutput & 0x80) && !(value & 0x80)) {
				_ppu->LatchLocationValues();
			}
			_state.IoPortOutput = value;
			break;

		case 0x4202:
		case 0x4203:
		case 0x4204:
		case 0x4205:
		case 0x4206:
			_aluMulDiv.Write(addr, value);
			break;

		case 0x4207: 
			_state.HorizontalTimer = (_state.HorizontalTimer & 0x100) | value; 
			ProcessIrqCounters();
			break;

		case 0x4208: 
			_state.HorizontalTimer = (_state.HorizontalTimer & 0xFF) | ((value & 0x01) << 8); 
			ProcessIrqCounters();
			break;

		case 0x4209: 
			_state.VerticalTimer = (_state.VerticalTimer & 0x100) | value; 

			//Calling this here fixes flashing issue in "Shin Nihon Pro Wrestling Kounin - '95 Tokyo Dome Battle 7"
			//The write to change from scanline 16 to 17 occurs between both ProcessIrqCounter calls, which causes the IRQ
			//line to always be high (since the previous check is on scanline 16, and the next check on scanline 17)
			ProcessIrqCounters();
			break;

		case 0x420A: 
			_state.VerticalTimer = (_state.VerticalTimer & 0xFF) | ((value & 0x01) << 8);
			ProcessIrqCounters();
			break;

		case 0x420D: _state.EnableFastRom = (value & 0x01) != 0; break;

		default:
			LogDebug("[Debug] Unimplemented register write: " + HexUtilities::ToHex(addr) + " = " + HexUtilities::ToHex(value));
			break;
	}
}

InternalRegisterState InternalRegisters::GetState()
{
	return _state;
}

AluState InternalRegisters::GetAluState()
{
	return _aluMulDiv.GetState();
}

void InternalRegisters::Serialize(Serializer &s)
{
	SV(_state.EnableFastRom); SV(_nmiFlag); SV(_state.EnableNmi); SV(_state.EnableHorizontalIrq); SV(_state.EnableVerticalIrq); SV(_state.HorizontalTimer);
	SV(_state.VerticalTimer); SV(_state.IoPortOutput); SV(_state.ControllerData[0]); SV(_state.ControllerData[1]); SV(_state.ControllerData[2]); SV(_state.ControllerData[3]);
	SV(_irqLevel); SV(_needIrq); SV(_state.EnableAutoJoypadRead); SV(_irqFlag);

	SV(_aluMulDiv);

	SV(_autoReadClockStart);
	SV(_newControllerData[0]); SV(_newControllerData[1]); SV(_newControllerData[2]); SV(_newControllerData[3]);
	SV(_newControllerDataPending);
}

#include "pch.h"
#include "SNES/RegisterHandlerB.h"
#include "SNES/SnesConsole.h"
#include "SNES/SnesPpu.h"
#include "SNES/Spc.h"
#include "SNES/BaseCartridge.h"
#include "SNES/Coprocessors/SA1/Sa1.h"
#include "SNES/Coprocessors/MSU1/Msu1.h"
#include "Shared/Emulator.h"
#include "Shared/CheatManager.h"
#include "Utilities/Serializer.h"

RegisterHandlerB::RegisterHandlerB(SnesConsole *console, SnesPpu * ppu, Spc * spc, uint8_t * workRam) : IMemoryHandler(MemoryType::SnesRegister)
{
	_console = console;
	_emu = console->GetEmulator();
	_cheatManager = _emu->GetCheatManager();
	_sa1 = console->GetCartridge()->GetSa1();
	_ppu = ppu;
	_spc = spc;
	_msu1 = console->GetMsu1();
	_workRam = workRam;
	_wramPosition = 0;
}

uint8_t RegisterHandlerB::Read(uint32_t addr)
{
	addr &= 0xFFFF;
	if(addr >= 0x2140 && addr <= 0x217F) {
		return _spc->CpuReadRegister(addr & 0x03);
	} else if(addr == 0x2180) {
		uint8_t value = _workRam[_wramPosition];
		if(_cheatManager->HasCheats<CpuType::Snes>()) {
			_cheatManager->ApplyCheat<CpuType::Snes>(0x7E0000 | _wramPosition, value);
		}
		_emu->ProcessMemoryRead<CpuType::Snes>(0x7E0000 | _wramPosition, value, MemoryOperationType::Read);
		_wramPosition = (_wramPosition + 1) & 0x1FFFF;
		return value;
	} else if(addr >= 0x2300 && addr <= 0x23FF && _console->GetCartridge()->GetSa1()) {
		return _console->GetCartridge()->GetSa1()->CpuRegisterRead(addr);
	} else if(_msu1 && addr <= 0x2007) {
		return _msu1->Read(addr);
	} else {
		return _ppu->Read(addr);
	}
}

uint8_t RegisterHandlerB::Peek(uint32_t addr)
{
	//Avoid side effects for now
	return 0;
}

void RegisterHandlerB::PeekBlock(uint32_t addr, uint8_t *output)
{
	//Avoid side effects for now
	memset(output, 0, 0x1000);
}

void RegisterHandlerB::Write(uint32_t addr, uint8_t value)
{
	addr &= 0xFFFF;
	if(addr >= 0x2140 && addr <= 0x217F) {
		_spc->CpuWriteRegister(addr & 0x03, value);
	} else if(addr >= 0x2180 && addr <= 0x2183) {
		switch(addr & 0xFFFF) {
			case 0x2180:
				if(_emu->ProcessMemoryWrite<CpuType::Snes>(0x7E0000 | _wramPosition, value, MemoryOperationType::Write)) {
					_workRam[_wramPosition] = value;
					_wramPosition = (_wramPosition + 1) & 0x1FFFF;
				}
				break;

			case 0x2181: _wramPosition = (_wramPosition & 0x1FF00) | value; break;
			case 0x2182: _wramPosition = (_wramPosition & 0x100FF) | (value << 8); break;
			case 0x2183: _wramPosition = (_wramPosition & 0xFFFF) | ((value & 0x01) << 16); break;
		}
	} else if(addr >= 0x2200 && addr <= 0x22FF && _console->GetCartridge()->GetSa1()) {
		_console->GetCartridge()->GetSa1()->CpuRegisterWrite(addr, value);
	} else if(_msu1 && addr <= 0x2007) {
		_msu1->Write(addr, value);
	} else {
		_ppu->Write(addr, value);
	}
}

AddressInfo RegisterHandlerB::GetAbsoluteAddress(uint32_t address)
{
	return { -1, MemoryType::SnesMemory };
}

uint32_t RegisterHandlerB::GetWramPosition()
{
	return _wramPosition;
}

void RegisterHandlerB::Serialize(Serializer &s)
{
	SV(_wramPosition);
}

#include "stdafx.h"
#include "RegisterHandlerB.h"
#include "Console.h"
#include "Ppu.h"
#include "Spc.h"
#include "BaseCartridge.h"
#include "Sa1.h"
#include "Msu1.h"
#include "CheatManager.h"
#include "../Utilities/Serializer.h"

RegisterHandlerB::RegisterHandlerB(Console *console, Ppu * ppu, Spc * spc, uint8_t * workRam)
{
	_console = console;
	_cheatManager = console->GetCheatManager().get();
	_sa1 = console->GetCartridge()->GetSa1();
	_ppu = ppu;
	_spc = spc;
	_msu1 = console->GetMsu1().get();
	_workRam = workRam;
	_wramPosition = 0;
	_memoryType = SnesMemoryType::Register;
}

uint8_t RegisterHandlerB::Read(uint32_t addr)
{
	addr &= 0xFFFF;
	if(addr >= 0x2140 && addr <= 0x217F) {
		return _spc->CpuReadRegister(addr & 0x03);
	} else if(addr == 0x2180) {
		uint8_t value = _workRam[_wramPosition];
		_console->ProcessWorkRamRead(_wramPosition, value);
		_console->GetCheatManager()->ApplyCheat(0x7E0000 | _wramPosition, value);
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
		return _spc->CpuWriteRegister(addr & 0x03, value);
	} if(addr >= 0x2180 && addr <= 0x2183) {
		switch(addr & 0xFFFF) {
			case 0x2180:
				_console->ProcessWorkRamWrite(_wramPosition, value);
				_workRam[_wramPosition] = value;
				_wramPosition = (_wramPosition + 1) & 0x1FFFF;
				break;

			case 0x2181: _wramPosition = (_wramPosition & 0x1FF00) | value; break;
			case 0x2182: _wramPosition = (_wramPosition & 0x100FF) | (value << 8); break;
			case 0x2183: _wramPosition = (_wramPosition & 0xFFFF) | ((value & 0x01) << 16); break;
		}
	} else if(addr >= 0x2200 && addr <= 0x22FF && _console->GetCartridge()->GetSa1()) {
		_console->GetCartridge()->GetSa1()->CpuRegisterWrite(addr, value);
	} else if(_msu1 && addr <= 0x2007) {
		return _msu1->Write(addr, value);
	} else {
		_ppu->Write(addr, value);
	}
}

AddressInfo RegisterHandlerB::GetAbsoluteAddress(uint32_t address)
{
	return { -1, SnesMemoryType::CpuMemory };
}

void RegisterHandlerB::Serialize(Serializer &s)
{
	s.Stream(_wramPosition);
}

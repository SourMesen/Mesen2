#pragma once
#include "stdafx.h"
#include "IMemoryHandler.h"
#include "Ppu.h"
#include "Spc.h"

class RegisterHandlerB : public IMemoryHandler
{
private:
	Ppu * _ppu;
	Spc *_spc;
	uint8_t *_workRam;
	uint32_t _wramPosition;

public:
	RegisterHandlerB(Ppu *ppu, Spc *spc, uint8_t *workRam)
	{
		_ppu = ppu;
		_spc = spc;
		_workRam = workRam;
		_wramPosition = 0;
	}

	uint8_t Read(uint32_t addr) override
	{
		addr &= 0xFFFF;
		if(addr >= 0x2140 && addr <= 0x217F) {
			return _spc->Read(addr & 0x03);
		} else if(addr == 0x2180) {
			return _workRam[_wramPosition++];
		} else {
			return _ppu->Read(addr);
		}
	}

	uint8_t Peek(uint32_t addr) override
	{
		//Avoid side effects for now
		return 0;
	}

	void Write(uint32_t addr, uint8_t value) override
	{
		addr &= 0xFFFF;
		if(addr >= 0x2140 && addr <= 0x217F) {
			return _spc->Write(addr & 0x03, value);
		} if(addr >= 0x2180 && addr <= 0x2183) {
			switch(addr & 0xFFFF) {
				case 0x2180:
					_workRam[_wramPosition] = value;
					_wramPosition = (_wramPosition + 1) & (0x1FFFF);
					break;

				case 0x2181: _wramPosition = (_wramPosition & 0x1FF00) | value; break;
				case 0x2182: _wramPosition = (_wramPosition & 0x100FF) | (value << 8); break;
				case 0x2183: _wramPosition = (_wramPosition & 0xFFFF) | ((value & 0x01) << 16); break;
			}
		} else {
			_ppu->Write(addr, value);
		}
	}

	AddressInfo GetAbsoluteAddress(uint32_t address) override
	{
		return { -1, SnesMemoryType::CpuMemory };
	}
};
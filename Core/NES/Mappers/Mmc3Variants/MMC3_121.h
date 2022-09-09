#pragma once

#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_121 : public MMC3
{
private:
	uint8_t _exRegs[8] = {};

protected:
	bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		MMC3::InitMapper();

		AddRegisterRange(0x5000, 0x5FFF, MemoryOperation::Any);
		RemoveRegisterRange(0x8000, 0xFFFF, MemoryOperation::Read);
	}

	void Reset(bool softReset) override
	{
		memset(_exRegs, 0, sizeof(_exRegs));
		_exRegs[3] = 0x80;
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		return _exRegs[4];
	}

	void SelectPrgPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		uint8_t orValue = (_exRegs[3] & 0x80) >> 2;
		if(_exRegs[5] & 0x3F) {
			BaseMapper::SelectPrgPage(slot, (page & 0x1F) | orValue, memoryType);
			BaseMapper::SelectPrgPage(1, _exRegs[2] | orValue, memoryType);
			BaseMapper::SelectPrgPage(2, _exRegs[1] | orValue, memoryType);
			BaseMapper::SelectPrgPage(3, _exRegs[0] | orValue, memoryType);
		} else {
			BaseMapper::SelectPrgPage(slot, (page & 0x1F) | orValue, memoryType);
		}
	}

	void SelectChrPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType) override
	{
		if(_prgSize == _chrRomSize) {
			//Hack for Super 3-in-1
			BaseMapper::SelectChrPage(slot, page | ((_exRegs[3] & 0x80) << 1), memoryType);
		} else {			
			if((slot < 4 && _chrMode == 0) || (slot >= 4 && _chrMode == 1)) {
				page |= 0x100;
			}
			BaseMapper::SelectChrPage(slot, page, memoryType);
		}
	}

	void UpdateExRegs()
	{
		switch(_exRegs[5] & 0x3F) {
			case 0x20: _exRegs[7] = 1; _exRegs[0] = _exRegs[6]; break;
			case 0x29: _exRegs[7] = 1; _exRegs[0] = _exRegs[6]; break;
			case 0x26: _exRegs[7] = 0; _exRegs[0] = _exRegs[6]; break;
			case 0x2B: _exRegs[7] = 1; _exRegs[0] = _exRegs[6]; break;
			case 0x2C:
				_exRegs[7] = 1;
				if(_exRegs[6]) {
					_exRegs[0] = _exRegs[6];
				}
				break;
			case 0x3C:
			case 0x3F: _exRegs[7] = 1; _exRegs[0] = _exRegs[6]; break;
			case 0x28: _exRegs[7] = 0; _exRegs[1] = _exRegs[6]; break;
			case 0x2A: _exRegs[7] = 0; _exRegs[2] = _exRegs[6]; break;
			case 0x2F: break;
			default: _exRegs[5] = 0; break;
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			//$5000-$5FFF
			const uint8_t lookup[] = { 0x83, 0x83, 0x42, 0x00 };
			_exRegs[4] = lookup[value & 0x03];
			if((addr & 0x5180) == 0x5180) {	
				//Hack for Super 3-in-1
				_exRegs[3] = value;
				UpdateState();
			}
		} else if(addr < 0xA000) {
			//$8000-$9FFF
			if((addr & 0x03) == 0x03) {
				_exRegs[5] = value;
				UpdateExRegs();
				MMC3::WriteRegister(0x8000, value);
			} else if(addr & 0x01) {
				_exRegs[6] = ((value & 0x01) << 5) | ((value & 0x02) << 3) | ((value & 0x04) << 1) | ((value & 0x08) >> 1) | ((value & 0x10) >> 3) | ((value & 0x20) >> 5);
				if(!_exRegs[7]) {
					UpdateExRegs();
				}
				MMC3::WriteRegister(0x8001, value);
			} else {
				MMC3::WriteRegister(0x8000, value);
			}
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SVArray(_exRegs, 8);
	}
};
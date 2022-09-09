#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Mapper234 : public BaseMapper
{
private:
	uint8_t _regs[2] = {};

protected:
	uint16_t RegisterStartAddress() override { return 0xFF80; }
	uint16_t RegisterEndAddress() override { return 0xFF9F; }
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	bool AllowRegisterRead() override { return true; }
	bool HasBusConflicts() override { return true; }

	void InitMapper() override
	{
		AddRegisterRange(0xFFE8, 0xFFF8, MemoryOperation::Any);
		memset(_regs, 0, sizeof(_regs));
		UpdateState();
	}

	void UpdateState()
	{
		if(_regs[0] & 0x40) {
			//NINA-03 mode
			SelectPrgPage(0, (_regs[0] & 0x0E) | (_regs[1] & 0x01));
			SelectChrPage(0, ((_regs[0] << 2) & 0x38) | ((_regs[1] >> 4) & 0x07));
		} else {
			//CNROM mode
			SelectPrgPage(0, _regs[0] & 0x0F);
			SelectChrPage(0, ((_regs[0] << 2) & 0x3C) | ((_regs[1] >> 4) & 0x03));
		}

		SetMirroringType(_regs[0] & 0x80 ? MirroringType::Horizontal : MirroringType::Vertical);
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		uint8_t value = InternalReadRam(addr);
		if(addr <= 0xFF9F) {
			if(!(_regs[0] & 0x3F)) {
				_regs[0] = value;
				UpdateState();
			}
		} else {
			_regs[1] = value & 0x71;
			UpdateState();
		}

		return value;
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr <= 0xFF9F) {
			if(!(_regs[0] & 0x3F)) {
				_regs[0] = value;
				UpdateState();
			}
		} else {
			_regs[1] = value & 0x71;
			UpdateState();
		}		
	}
};
#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class Bmc80013B : public BaseMapper
{
private:
	uint8_t _regs[2] = {};
	uint8_t _mode = 0;

protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectCHRPage(0, 0);
	}

	void Reset(bool softReset) override
	{
		_regs[0] = _regs[1] = _mode = 0;
		UpdateState();
	}

	void Serialize(Serializer& s) override
	{
		SV(_regs[0]);
		SV(_regs[1]);
		SV(_mode);
	}

	void UpdateState()
	{
		if(_mode & 0x02) {
			SelectPRGPage(0, (_regs[0] & 0x0F) | (_regs[1] & 0x70));
		} else {
			SelectPRGPage(0, _regs[0] & 0x03);
		}
		
		SelectPRGPage(1, _regs[1] & 0x7F);
		SetMirroringType(_regs[0] & 0x10 ? MirroringType::Vertical : MirroringType::Horizontal);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		uint8_t reg = (addr >> 13) & 0x03;
		if(reg == 0) {
			_regs[0] = value;
		} else {
			_regs[1] = value;
			_mode = reg;
		}
		UpdateState();		
	}
};
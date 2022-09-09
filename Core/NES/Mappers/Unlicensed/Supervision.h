#pragma once
#include "pch.h"
#include "Utilities/CRC32.h"
#include "NES/BaseMapper.h"

class Supervision : public BaseMapper
{
private:
	static constexpr uint32_t EPROM_CRC = 0x63794E25;
	uint8_t _regs[2] = {};
	bool _epromFirst = false;

protected:
	virtual uint16_t RegisterStartAddress() override { return 0x6000; }
	virtual uint16_t RegisterEndAddress() override { return 0xFFFF; }

	virtual uint16_t GetPrgPageSize() override { return 0x2000; }
	virtual uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		_epromFirst = _prgSize >= 0x8000 && CRC32::GetCRC(_prgRom, 0x8000) == EPROM_CRC;
		_regs[0] = _regs[1] = 0;

		UpdateState();
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_regs[0]);
		SV(_regs[1]);
	}

	void UpdateState()
	{
		uint16_t r = _regs[0] << 3 & 0x78;
		
		SetCpuMemoryMapping(0x6000, 0x7FFF, (r << 1 | 0x0F) + (_epromFirst ? 0x04 : 0x00), PrgMemoryType::PrgRom);

		SelectPrgPage2x(0, ((_regs[0] & 0x10) ? (r | (_regs[1] & 0x07)) + (_epromFirst ? 0x02 : 0x00) : _epromFirst ? 0x00 : 0x80) << 1);
		SelectPrgPage2x(1, ((_regs[0] & 0x10) ? (r | (0xFF & 0x07)) + (_epromFirst ? 0x02 : 0x00) : _epromFirst ? 0x01 : 0x81) << 1);

		SetMirroringType(_regs[0] & 0x20 ? MirroringType::Horizontal : MirroringType::Vertical);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			_regs[0] = value;
		} else {
			_regs[1] = value;
		}

		UpdateState();
	}
};

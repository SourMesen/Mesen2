#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Mapper230 : public BaseMapper
{
private:
	bool _contraMode = false;

protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectChrPage(0, 0);
		Reset(true);
	}

	virtual void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_contraMode);
	}

	virtual void Reset(bool softReset) override
	{
		if(softReset) {
			_contraMode = !_contraMode;
			if(_contraMode) {
				SelectPrgPage(0, 0);
				SelectPrgPage(1, 7);
				SetMirroringType(MirroringType::Vertical);
			} else {
				SelectPrgPage(0, 8);
				SelectPrgPage(1, 9);
				SetMirroringType(MirroringType::Horizontal);
			}
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(_contraMode) {
			SelectPrgPage(0, value & 0x07);
		} else {
			if(value & 0x20) {
				SelectPrgPage(0, (value & 0x1F) + 8);
				SelectPrgPage(1, (value & 0x1F) + 8);
			} else {
				SelectPrgPage(0, (value & 0x1E) + 8);
				SelectPrgPage(1, (value & 0x1E) + 9);
			}
			SetMirroringType(value & 0x40 ? MirroringType::Vertical : MirroringType::Horizontal);
		}
	}
};

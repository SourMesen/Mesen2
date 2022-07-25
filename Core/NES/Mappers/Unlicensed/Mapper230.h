#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class Mapper230 : public BaseMapper
{
private:
	bool _contraMode = false;

protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectCHRPage(0, 0);
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
				SelectPRGPage(0, 0);
				SelectPRGPage(1, 7);
				SetMirroringType(MirroringType::Vertical);
			} else {
				SelectPRGPage(0, 8);
				SelectPRGPage(1, 9);
				SetMirroringType(MirroringType::Horizontal);
			}
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(_contraMode) {
			SelectPRGPage(0, value & 0x07);
		} else {
			if(value & 0x20) {
				SelectPRGPage(0, (value & 0x1F) + 8);
				SelectPRGPage(1, (value & 0x1F) + 8);
			} else {
				SelectPRGPage(0, (value & 0x1E) + 8);
				SelectPRGPage(1, (value & 0x1E) + 9);
			}
			SetMirroringType(value & 0x40 ? MirroringType::Vertical : MirroringType::Horizontal);
		}
	}
};

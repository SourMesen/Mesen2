#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Mapper60 : public BaseMapper
{
private:
	uint8_t _resetCounter = 0;

protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		_resetCounter = 0;
		SelectPrgPage(0, 0);
		SelectPrgPage(1, 0);
		SelectChrPage(0, 0);
	}

	virtual void Reset(bool softReset) override
	{
		if(softReset) {
			_resetCounter = (_resetCounter + 1) % 4;

			SelectPrgPage(0, _resetCounter);
			SelectPrgPage(1, _resetCounter);
			SelectChrPage(0, _resetCounter);
		}
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_resetCounter);
	}
};

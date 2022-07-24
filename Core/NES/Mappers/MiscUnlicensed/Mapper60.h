#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class Mapper60 : public BaseMapper
{
private:
	uint8_t _resetCounter = 0;

protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		_resetCounter = 0;
		SelectPRGPage(0, 0);
		SelectPRGPage(1, 0);
		SelectCHRPage(0, 0);
	}

	virtual void Reset(bool softReset) override
	{
		if(softReset) {
			_resetCounter = (_resetCounter + 1) % 4;

			SelectPRGPage(0, _resetCounter);
			SelectPRGPage(1, _resetCounter);
			SelectCHRPage(0, _resetCounter);
		}
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_resetCounter);
	}
};

#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class NROM : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPRGPage(0, 0);
		SelectPRGPage(1, 1);

		SelectCHRPage(0, 0);
	}
};
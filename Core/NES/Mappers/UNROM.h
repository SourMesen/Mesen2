#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class UNROM : public BaseMapper
{
	protected:
		uint16_t GetPRGPageSize() override { return 0x4000; }
		uint16_t GetCHRPageSize() override { return 0x2000; }

		void InitMapper() override 
		{
			//First and last PRG page
			SelectPRGPage(0, 0);
			SelectPRGPage(1, -1);

			SelectCHRPage(0, 0);
		}

		bool HasBusConflicts() override { return _romInfo.SubMapperID == 2; }

		void WriteRegister(uint16_t addr, uint8_t value) override
		{
			SelectPRGPage(0, value);
		}
};
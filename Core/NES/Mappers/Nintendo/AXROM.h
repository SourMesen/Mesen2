#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "Utilities/Serializer.h"

class AXROM : public BaseMapper
{
	protected:
		virtual uint16_t GetPrgPageSize() override { return 0x8000; }
		virtual uint16_t GetChrPageSize() override {	return 0x2000; }

		void InitMapper() override 
		{
			SelectChrPage(0, 0);
			WriteRegister(0, GetPowerOnByte());
		}

		bool HasBusConflicts() override { return _romInfo.SubMapperID == 2; }

		void WriteRegister(uint16_t addr, uint8_t value) override
		{
			SelectPrgPage(0, value & 0x0F);

			SetMirroringType(((value & 0x10) == 0x10) ? MirroringType::ScreenBOnly : MirroringType::ScreenAOnly);
		}
};
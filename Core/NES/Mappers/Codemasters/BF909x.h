#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "Utilities/Serializer.h"

class BF909x : public BaseMapper
{
private:
	bool _bf9097Mode = false;  //Auto-detect for firehawk

protected:
	virtual uint16_t GetPrgPageSize() override { return 0x4000; }
	virtual uint16_t GetChrPageSize() override {	return 0x2000; }

	void InitMapper() override 
	{
		if(_romInfo.SubMapperID == 1) {
			_bf9097Mode = true;
		}

		//First and last PRG page
		SelectPrgPage(0, 0);
		SelectPrgPage(1, -1);

		SelectChrPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr == 0x9000) {
			//Firehawk uses $9000 to change mirroring
			_bf9097Mode = true;
		}

		if(addr >= 0xC000 || !_bf9097Mode) {
			SelectPrgPage(0, value);
		} else if(addr < 0xC000) {
			SetMirroringType((value & 0x10) ? MirroringType::ScreenAOnly : MirroringType::ScreenBOnly);
		}
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_bf9097Mode);
	}
};
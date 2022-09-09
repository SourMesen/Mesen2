#pragma once
#include "pch.h"
#include "NES/Mappers/Unlicensed/Mapper226.h"

class Mapper233 : public Mapper226
{
private:
	uint8_t _reset = false;

protected:
	void Reset(bool softReset) override
	{
		Mapper226::Reset(softReset);

		if(softReset) {
			_reset = _reset ^ 0x01;
			UpdatePrg();
		} else {
			_reset = 0;
		}
	}

	void Serialize(Serializer& s) override
	{
		Mapper226::Serialize(s);
		SV(_reset);
	}

	uint8_t GetPrgPage() override
	{
		return (_registers[0] & 0x1F) | (_reset << 5) | ((_registers[1] & 0x01) << 6);
	}
};

#pragma once
#include "NES/BaseMapper.h"

class Mapper227 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		WriteRegister(0x8000, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		uint16_t prgBank = ((addr >> 2) & 0x1F) | ((addr & 0x100) >> 3);
		bool sFlag = (addr & 0x01) == 0x01;
		bool lFlag = ((addr >> 9) & 0x01) == 0x01;
		bool prgMode = ((addr >> 7) & 0x01) == 0x01;

		if(prgMode) {
			if(sFlag) {
				SelectPrgPage2x(0, prgBank & 0xFE);
			} else {
				SelectPRGPage(0, prgBank);
				SelectPRGPage(1, prgBank);
			}
		} else {
			if(sFlag){
				if(lFlag) {
					SelectPRGPage(0, prgBank & 0x3E);
					SelectPRGPage(1, prgBank | 0x07);
				} else {
					SelectPRGPage(0, prgBank & 0x3E);
					SelectPRGPage(1, prgBank & 0x38);
				}
			} else {
				if(lFlag) {
					SelectPRGPage(0, prgBank);
					SelectPRGPage(1, prgBank | 0x07);
				} else {
					SelectPRGPage(0, prgBank);
					SelectPRGPage(1, prgBank & 0x38);
				}
			}
		}

		SetMirroringType((addr & 0x02) == 0x02 ? MirroringType::Horizontal : MirroringType::Vertical);
	}
};
#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_114 : public MMC3
{
private:
	static constexpr uint8_t _security[8] = { 0,3,1,5,6,7,2,4 };
	uint8_t _exRegs[2] = {};

protected:
	uint16_t RegisterStartAddress() override { return 0x5000; }
	bool ForceMmc3RevAIrqs() override { return true; }

	void InitMapper() override
	{
		MMC3::InitMapper();
		_exRegs[0] = _exRegs[1] = 0;
	}

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SV(_exRegs[0]);
		SV(_exRegs[1]);
	}

	void UpdatePrgMapping() override
	{
		if(_exRegs[0] & 0x80) {
			SelectPrgPage2x(0, (_exRegs[0] & 0x0F) << 1);
			SelectPrgPage2x(1, (_exRegs[0] & 0x0F) << 1);
		} else {
			MMC3::UpdatePrgMapping();
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			_exRegs[0] = value;
			UpdatePrgMapping();
		} else {
			switch(addr & 0xE001) {
				case 0x8001: MMC3::WriteRegister(0xA000, value); break;

				case 0xA000:
					MMC3::WriteRegister(0x8000, (value & 0xC0) | _security[value & 0x07]);
					_exRegs[1] = 1;
					break;

				case 0xA001:
					_irqReloadValue = value;
					break;

				case 0xC000:
					if(_exRegs[1]) {
						_exRegs[1] = 0;
						MMC3::WriteRegister(0x8001, value);
					}
					break;

				case 0xC001:
					_irqReload = true;
					break;

				case 0xE000:
					_console->GetCpu()->ClearIrqSource(IRQSource::External);
					_irqEnabled = false;
					break;

				case 0xE001:
					_irqEnabled = true;
					break;
			}
		}
	}
};
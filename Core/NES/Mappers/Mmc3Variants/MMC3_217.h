#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_217 : public MMC3
{
private:
	uint8_t _exRegs[4] = {};
	static constexpr uint8_t _lut[8] = { 0,6,3,7,5,2,4,1 };

protected:
	void InitMapper() override
	{
		AddRegisterRange(0x5000, 0x5001, MemoryOperation::Write);
		AddRegisterRange(0x5007, 0x5007, MemoryOperation::Write);

		MMC3::InitMapper();
	}

	void Reset(bool softReset) override
	{
		_exRegs[0] = 0;
		_exRegs[1] = 0xFF;
		_exRegs[2] = 0x03;
		_exRegs[3] = 0;

		BaseMapper::Reset(softReset);

		UpdateState();
	}

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SVArray(_exRegs, 4);
	}

	void SelectChrPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		if(!(_exRegs[1] & 0x08)) {
			page = (_exRegs[1] << 3 & 0x80) | (page & 0x7F);
		}

		MMC3::SelectChrPage(slot, (_exRegs[1] << 8 & 0x0300) | page);
	}

	void SelectPrgPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		if(_exRegs[1] & 0x08) {
			page = (page & 0x1F);
		} else {
			page = (page & 0x0F) | (_exRegs[1] & 0x10);
		}

		MMC3::SelectPrgPage(slot, (_exRegs[1] << 5 & 0x60) | page);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			switch(addr) {
				case 0x5000:
					_exRegs[0] = value;

					if(value & 0x80) {
						value = (value & 0x0F) | (_exRegs[1] << 4 & 0x30);
						value <<= 1;
						SelectPrgPage(0, value);
						SelectPrgPage(1, value + 1);
						SelectPrgPage(2, value);
						SelectPrgPage(3, value + 1);
					} else {
						UpdatePrgMapping();
					}
					break;

				case 0x5001:
					if(_exRegs[1] != value) {
						_exRegs[1] = value;
						UpdatePrgMapping();
					}
					break;

				case 0x5007:
					_exRegs[2] = value;
					break;
			}
		} else {
			switch(addr & 0xE001) {
				case 0x8000: 
					MMC3::WriteRegister(_exRegs[2] ? 0xC000 : 0x8000, value);
					break;

				case 0x8001:
					if(_exRegs[2]) {
						value = (value & 0xC0) | _lut[value & 0x07];
						_exRegs[3] = 1;

						MMC3::WriteRegister(0x8000, value);
					} else {
						MMC3::WriteRegister(0x8001, value);
					}
					break;

				case 0xA000:
					if(_exRegs[2]) {
						if(_exRegs[3] && ((_exRegs[0] & 0x80) == 0 || GetCurrentRegister() < 6)) {
							_exRegs[3] = 0;
							MMC3::WriteRegister(0x8001, value);
						}
					} else {
						SetMirroringType(value & 0x01 ? MirroringType::Horizontal : MirroringType::Vertical);
					}
					break;

				case 0xA001:
					if(_exRegs[2]) {
						SetMirroringType(value & 0x01 ? MirroringType::Horizontal : MirroringType::Vertical);
					} else {
						MMC3::WriteRegister(0xA001, value);
					}
					break;

				default:
					MMC3::WriteRegister(addr, value);
					break;
			}
		}
	}
};
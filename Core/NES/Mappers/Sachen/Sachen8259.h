#pragma once
#include "NES/BaseMapper.h"

enum class Sachen8259Variant
{
	Sachen8259A,
	Sachen8259B,
	Sachen8259C,
	Sachen8259D,
};

class Sachen8259 : public BaseMapper
{
private:
	Sachen8259Variant _variant = Sachen8259Variant::Sachen8259A;
	uint8_t _currentReg = 0;
	uint8_t _regs[8] = {};
	uint8_t _shift = 0;
	uint8_t _chrOr[3] = {};

protected:
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return _variant == Sachen8259Variant::Sachen8259D ? 0x400 : 0x800; }
	uint16_t RegisterStartAddress() override { return 0x4100; }
	uint16_t RegisterEndAddress() override { return 0x7FFF; }
	
	void InitMapper() override
	{
		_currentReg = 0;
		memset(_regs, 0, sizeof(_regs));

		SelectPrgPage(0, 0);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);

		SVArray(_regs, 8);
		SV(_currentReg);
	}

	void UpdateState()
	{
		bool simpleMode = (_regs[7] & 0x01) == 0x01;
		switch((_regs[7] >> 1) & 0x03) {
			case 0: SetMirroringType(_variant == Sachen8259Variant::Sachen8259D ? MirroringType::Horizontal : MirroringType::Vertical); break;
			case 1: SetMirroringType(_variant == Sachen8259Variant::Sachen8259D ? MirroringType::Vertical : MirroringType::Horizontal); break;
			case 2: SetNametables(0, 1, 1, 1); break;
			case 3: SetMirroringType(MirroringType::ScreenAOnly); break;
		}

		if(_variant == Sachen8259Variant::Sachen8259D && simpleMode) {
			//"Enable "simple" mode. (mirroring is fixed to H, and banks become weird)"
			SetMirroringType(MirroringType::Horizontal);
		}

		SelectPrgPage(0, _regs[5]);

		if(_variant == Sachen8259Variant::Sachen8259D) {
			SelectChrPage(0, _regs[0]);
			SelectChrPage(1, ((_regs[4] & 0x01) << 4) | _regs[simpleMode ? 0 : 1]);
			SelectChrPage(2, ((_regs[4] & 0x02) << 3) | _regs[simpleMode ? 0 : 2]);
			SelectChrPage(3, ((_regs[4] & 0x04) << 2) | ((_regs[6] & 0x01) << 3) | _regs[simpleMode ? 0 : 3]);
			SelectChrPage4x(1, -4);
		} else {
			if(!HasChrRam()) {
				uint8_t chrHigh = _regs[4] << 3;
				SelectChrPage(0, ((chrHigh | _regs[0]) << _shift));
				SelectChrPage(1, ((chrHigh | (_regs[simpleMode ? 0 : 1])) << _shift) | _chrOr[0]);
				SelectChrPage(2, ((chrHigh | (_regs[simpleMode ? 0 : 2])) << _shift) | _chrOr[1]);
				SelectChrPage(3, ((chrHigh | (_regs[simpleMode ? 0 : 3])) << _shift) | _chrOr[2]);
			}
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xC101) {
			case 0x4100: _currentReg = value & 0x07; break;
			case 0x4101: _regs[_currentReg] = value & 0x07; UpdateState(); break;
		}
	}

public:
	Sachen8259(Sachen8259Variant variant)
	{
		_variant = variant;
		switch(variant) {
			case Sachen8259Variant::Sachen8259A:
				_shift = 1;
				_chrOr[0] = 1;
				_chrOr[1] = 0;
				_chrOr[2] = 1;
				break;
			case Sachen8259Variant::Sachen8259B:
				_shift = 0;
				_chrOr[0] = 0;
				_chrOr[1] = 0;
				_chrOr[2] = 0;
				break;
			case Sachen8259Variant::Sachen8259C:
				_shift = 2;
				_chrOr[0] = 1;
				_chrOr[1] = 2;
				_chrOr[2] = 3;
				break;

			case Sachen8259Variant::Sachen8259D:
				//Variant D does not use the _shift / _chrOr variables at all
				_shift = 0;
				_chrOr[0] = _chrOr[1] = _chrOr[2] = 0;
				break;
		}
	}
};

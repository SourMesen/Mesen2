#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class MMC2 : public BaseMapper
{
private:
	enum class MMC2Registers
	{
		RegA000 = 0xA,
		RegB000 = 0xB,
		RegC000 = 0xC,
		RegD000 = 0xD,
		RegE000 = 0xE,
		RegF000 = 0xF
	};

protected:
	uint8_t _leftLatch = 0;
	uint8_t _rightLatch = 0;
	uint8_t _prgPage = 0;
	uint8_t _leftChrPage[2] = {};
	uint8_t _rightChrPage[2] = {};
	bool _needChrUpdate = 0;

	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x1000; }
	bool EnableVramAddressHook() override { return true; }

	void InitMapper() override
	{
		_leftLatch = 1;
		_rightLatch = 1;
		_leftChrPage[0] = GetPowerOnByte() & 0x1F;
		_leftChrPage[1] = GetPowerOnByte() & 0x1F;
		_rightChrPage[0] = GetPowerOnByte() & 0x1F;
		_rightChrPage[1] = GetPowerOnByte() & 0x1F;
		_needChrUpdate = false;

		SelectPrgPage(1, -3);
		SelectPrgPage(2, -2);
		SelectPrgPage(3, -1);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_prgPage);
		SV(_leftLatch);
		SV(_rightLatch);
		SV(_needChrUpdate);
		SV(_leftChrPage[0]);
		SV(_leftChrPage[1]);
		SV(_rightChrPage[0]);
		SV(_rightChrPage[1]);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch((MMC2Registers)(addr >> 12)) {
			case MMC2Registers::RegA000:
				_prgPage = value & 0x0F;
				SelectPrgPage(0, _prgPage);
				break;

			case MMC2Registers::RegB000:
				_leftChrPage[0] = value & 0x1F;
				SelectChrPage(0, _leftChrPage[_leftLatch]);
				break;

			case MMC2Registers::RegC000:
				_leftChrPage[1] = value & 0x1F;
				SelectChrPage(0, _leftChrPage[_leftLatch]);
				break;

			case MMC2Registers::RegD000:
				_rightChrPage[0] = value & 0x1F;
				SelectChrPage(1, _rightChrPage[_rightLatch]);
				break;

			case MMC2Registers::RegE000:
				_rightChrPage[1] = value & 0x1F;
				SelectChrPage(1, _rightChrPage[_rightLatch]);
				break;

			case MMC2Registers::RegF000:
				SetMirroringType(((value & 0x01) == 0x01) ? MirroringType::Horizontal : MirroringType::Vertical);
				break;
		}
	}

	vector<MapperStateEntry> GetMapperStateEntries() override
	{
		vector<MapperStateEntry> entries;
		string mirroringType;
		int64_t mirValue = 0;
		switch(GetMirroringType()) {
			case MirroringType::Vertical: mirroringType = "Vertical"; mirValue = 0; break;
			case MirroringType::Horizontal: mirroringType = "Horizontal"; mirValue = 1; break;
		}
		entries.push_back(MapperStateEntry("$A000.0-3", "PRG Bank", _prgPage, MapperStateValueType::Number8));
		entries.push_back(MapperStateEntry("$B000.0-4", "CHR Bank ($0000) ($FD)", _leftChrPage[0], MapperStateValueType::Number8));
		entries.push_back(MapperStateEntry("$C000.0-4", "CHR Bank ($0000) ($FE)", _leftChrPage[1], MapperStateValueType::Number8));
		entries.push_back(MapperStateEntry("$D000.0-4", "CHR Bank ($1000) ($FD)", _rightChrPage[0], MapperStateValueType::Number8));
		entries.push_back(MapperStateEntry("$E000.0-4", "CHR Bank ($1000) ($FE)", _rightChrPage[1], MapperStateValueType::Number8));
		entries.push_back(MapperStateEntry("$F000.0", "Mirroring", mirroringType, mirValue));
		return entries;
	}

public:
	void NotifyVramAddressChange(uint16_t addr) override
	{
		if(_needChrUpdate) {
			SelectChrPage(0, _leftChrPage[_leftLatch]);
			SelectChrPage(1, _rightChrPage[_rightLatch]);
			_needChrUpdate = false;
		}

		if(addr == 0x0FD8) {
			_leftLatch = 0;
			_needChrUpdate = true;
		} else if(addr == 0x0FE8) {
			_leftLatch = 1;
			_needChrUpdate = true;
		} else if(addr >= 0x1FD8 && addr <= 0x1FDF) {
			_rightLatch = 0;
			_needChrUpdate = true;
		} else if(addr >= 0x1FE8 && addr <= 0x1FEF) {
			_rightLatch = 1;
			_needChrUpdate = true;
		}
	}
};
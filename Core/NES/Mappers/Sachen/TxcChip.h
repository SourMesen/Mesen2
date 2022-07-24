#pragma once
#include "stdafx.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"

class TxcChip : public ISerializable
{
private:
	uint8_t _accumulator;
	uint8_t _inverter;
	uint8_t _staging;
	uint8_t _output;
	bool _increase;
	bool _yFlag;
	bool _invert;

	uint8_t _mask;
	bool _isJv001;

public:
	TxcChip(bool isJv001)
	{
		_accumulator = 0;
		_inverter = 0;
		_staging = 0;
		_output = 0;

		_increase = false;
		_yFlag = false;

		_isJv001 = isJv001;
		_mask = isJv001 ? 0x0F : 0x07;
		_invert = isJv001;
	}

	void Serialize(Serializer& s) override
	{
		SV(_accumulator);
		SV(_invert);
		SV(_inverter);
		SV(_staging);
		SV(_output);
		SV(_increase);
		SV(_yFlag);
	}
	
	bool GetInvertFlag()
	{
		return _invert;
	}

	bool GetY()
	{
		return _yFlag;
	}

	uint8_t GetOutput()
	{
		return _output;
	}

	uint8_t Read()
	{
		uint8_t value = (_accumulator & _mask) | ((_inverter ^ (_invert ? 0xFF : 0)) & ~_mask);
		_yFlag = !_invert || ((value & 0x10) != 0);
		return value;
	}

	void Write(uint16_t addr, uint8_t value)
	{
		if(addr < 0x8000) {
			switch(addr & 0xE103) {
				case 0x4100:
					if(_increase) {
						_accumulator++;
					} else {
						_accumulator = ((_accumulator & ~_mask) | (_staging & _mask)) ^ (_invert ? 0xFF : 0);
					}
					break;

				case 0x4101:
					_invert = (value & 0x01) != 0;
					break;

				case 0x4102:
					_staging = value & _mask;
					_inverter = value & ~_mask;
					break;

				case 0x4103: 
					_increase = (value & 0x01) != 0;
					break;
			}
		} else {
			if(_isJv001) {
				_output = (_accumulator & 0x0F) | (_inverter & 0xF0);
			} else {
				_output = (_accumulator & 0x0F) | ((_inverter & 0x08) << 1);
			}
		}

		_yFlag = !_invert || ((value & 0x10) != 0);
	}
};
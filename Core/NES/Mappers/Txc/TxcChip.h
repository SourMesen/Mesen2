#pragma once
#include "pch.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"

class TxcChip : public ISerializable
{
private:
	uint8_t _accumulator = 0;
	uint8_t _inverter = 0;
	uint8_t _staging = 0;
	uint8_t _output = 0;
	bool _increase = false;
	bool _yFlag = false;
	bool _invert = false;

	uint8_t _mask = 0;
	bool _isJv001 = false;

public:
	TxcChip(bool isJv001)
	{
		_isJv001 = isJv001;
		_mask = isJv001 ? 0x0F : 0x07;
		_invert = isJv001;
	}

	void Serialize(Serializer& s)
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
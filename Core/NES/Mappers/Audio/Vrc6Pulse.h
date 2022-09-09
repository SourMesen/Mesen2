#pragma once
#include "pch.h"
#include "Utilities/Serializer.h"

class Vrc6Pulse: public ISerializable
{
private:
	uint8_t _volume = 0;
	uint8_t _dutyCycle = 0;
	bool _ignoreDuty = false;
	uint16_t _frequency = 1;
	bool _enabled = false;

	int32_t _timer = 1;
	uint8_t _step = 0;
	uint8_t _frequencyShift = 0;

	void Serialize(Serializer& s) override
	{
		SV(_volume); SV(_dutyCycle); SV(_ignoreDuty); SV(_frequency); SV(_enabled); SV(_timer); SV(_step); SV(_frequencyShift);
	}

public:
	void WriteReg(uint16_t addr, uint8_t value)
	{
		switch(addr & 0x03) {
			case 0:
				_volume = value & 0x0F;
				_dutyCycle = (value & 0x70) >> 4;
				_ignoreDuty = (value & 0x80) == 0x80;
				break;

			case 1:
				_frequency = (_frequency & 0x0F00) | value;
				break;

			case 2:
				_frequency = (_frequency & 0xFF) | ((value & 0x0F) << 8);
				_enabled = (value & 0x80) == 0x80;
				if(!_enabled) {
					_step = 0;
				}
				break;
		}
	}

	void SetFrequencyShift(uint8_t shift)
	{
		_frequencyShift = shift;
	}

	void Clock()
	{
		if(_enabled) {
			_timer--;
			if(_timer == 0) {
				_step = (_step + 1) & 0x0F;
				_timer = (_frequency >> _frequencyShift) + 1;
			}
		}
	}

	uint8_t GetVolume()
	{
		if(!_enabled) {
			return 0;
		} else if(_ignoreDuty) {
			return _volume;
		} else {
			return _step <= _dutyCycle ? _volume : 0;
		}
	}
};
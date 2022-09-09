#pragma once
#include "pch.h"
#include "Utilities/Serializer.h"

class Vrc6Saw : public ISerializable
{
private:
	uint8_t _accumulatorRate = 0;
	uint8_t _accumulator = 0;
	uint16_t _frequency = 1;
	bool _enabled = false;

	int32_t _timer = 1;
	uint8_t _step = 0;
	uint8_t _frequencyShift = 0;

	void Serialize(Serializer& s) override
	{
		SV(_accumulatorRate); SV(_accumulator); SV(_frequency); SV(_enabled); SV(_timer); SV(_step); SV(_frequencyShift);
	}

public:
	void WriteReg(uint16_t addr, uint8_t value)
	{
		switch(addr & 0x03) {
			case 0:
				_accumulatorRate = value & 0x3F;
				break;

			case 1:
				_frequency = (_frequency & 0x0F00) | value;
				break;

			case 2:
				_frequency = (_frequency & 0xFF) | ((value & 0x0F) << 8);
				_enabled = (value & 0x80) == 0x80;
				if(!_enabled) {
					//If E is clear, the accumulator is forced to zero until E is again set.
					_accumulator = 0;
					
					//"The phase of the saw generator can be mostly reset by clearing and immediately setting E. Clearing E does not reset the frequency divider, however."
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
				_step = (_step + 1) % 14;
				_timer = (_frequency >> _frequencyShift) + 1;

				if(_step == 0) {
					_accumulator = 0;
				} else if((_step & 0x01) == 0x00) {
					_accumulator += _accumulatorRate;
				}
			}
		}
	}

	uint8_t GetVolume()
	{
		if(!_enabled) {
			return 0;
		} else {
			//"The high 5 bits of the accumulator are then output (provided the channel is enabled by having the E bit set)."
			return _accumulator >> 3;
		}
	}
};
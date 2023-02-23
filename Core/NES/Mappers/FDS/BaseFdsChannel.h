#pragma once
#include "pch.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"

class BaseFdsChannel : public ISerializable
{
protected:
	uint8_t _speed = 0;
	uint8_t _gain = 0;
	bool _envelopeOff = false;
	bool _volumeIncrease = false;
	uint16_t _frequency = 0;

	uint32_t _timer = 0;

	//"Few FDS NSFs write to this register. The BIOS initializes this to $E8."
	uint8_t _masterSpeed = 0xE8;

	void Serialize(Serializer& s) override
	{
		SV(_speed); SV(_gain); SV(_envelopeOff); SV(_volumeIncrease); SV(_frequency); SV(_timer); SV(_masterSpeed);
	}

public:
	void SetMasterEnvelopeSpeed(uint8_t masterSpeed)
	{
		_masterSpeed = masterSpeed;
	}

	virtual void WriteReg(uint16_t addr, uint8_t value)
	{
		switch(addr & 0x03) {
			case 0:
				_speed = value & 0x3F;
				_volumeIncrease = (value & 0x40) == 0x40;
				_envelopeOff = (value & 0x80) == 0x80;

				//"Writing to this register immediately resets the clock timer that ticks the volume envelope (delaying the next tick slightly)."
				ResetTimer();

				if(_envelopeOff) {
					//Envelope is off, gain = speed
					_gain = _speed;
				}
				break;

			case 2:
				_frequency = (_frequency & 0x0F00) | value;
				break;

			case 3:
				_frequency = (_frequency & 0xFF) | ((value & 0x0F) << 8);
				break;
		}
	}

	bool TickEnvelope()
	{
		if(!_envelopeOff && _masterSpeed > 0) {
			_timer--;
			if(_timer == 0) {
				ResetTimer();

				if(_volumeIncrease && _gain < 32) {
					_gain++;
				} else if(!_volumeIncrease && _gain > 0) {
					_gain--;
				}
				return true;
			}
		}
		return false;
	}

	uint8_t GetSpeed()
	{
		return _speed;
	}

	uint8_t GetGain()
	{
		return _gain;
	}
	
	uint8_t GetMasterSpeed()
	{
		return _masterSpeed;
	}

	uint16_t GetFrequency()
	{
		return _frequency;
	}

	bool GetVolumeIncreaseFlag()
	{
		return _volumeIncrease;
	}

	bool IsEnvelopeDisabled()
	{
		return _envelopeOff;
	}

	void ResetTimer()
	{
		_timer = 8 * (_speed + 1) * _masterSpeed;
	}
};
#pragma once
#include "pch.h"
#include "Utilities/Serializer.h"

class GbChannelDac : ISerializable
{
private:
	int16_t _counter = 0;
	uint16_t _volume = 0;

public:
	uint16_t GetDacVolume()
	{
		return _volume;
	}

	void Exec(uint32_t clocksToRun, bool enabled)
	{
		_counter -= clocksToRun;

		if(_counter <= 0) {
			//When the DAC is enabled or disabled, the channel's output
			//progressively fades in/out. This is used to slowly
			//increase/decrease the volume between 0% and 100%
			//This fixes sound issues in:
			// -3D Pocket Pool
			// -Ready 2 Rumble Boxing
			// -Cannon Fodder
			if(enabled) {
				_volume = std::min(100, _volume + 1);
			} else {
				_volume = std::max(0, _volume - 1);
			}
			_counter += 250;
		}
	}

	void Serialize(Serializer& s) override
	{
		SV(_counter);
		SV(_volume);
	}
};
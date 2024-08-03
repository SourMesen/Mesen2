#pragma once
#include "pch.h"

class GbaEnvelope
{
public:
	template<typename T, typename U>
	static void ClockEnvelope(T& state, U& channel)
	{
		uint8_t timer = state.EnvTimer;
		if(state.EnvTimer == 0 || --state.EnvTimer == 0) {
			if(state.EnvPeriod > 0 && !state.EnvStopped) {
				if(state.EnvRaiseVolume && state.Volume < 0x0F) {
					state.Volume++;
				} else if(!state.EnvRaiseVolume && state.Volume > 0) {
					state.Volume--;
				} else {
					state.EnvStopped = true;
				}

				//Clocking envelope should update output immediately (based on div_trigger_volume/channel_4_volume_div tests)
				channel.UpdateOutput();

				state.EnvTimer = state.EnvPeriod;
				if(timer == 0) {
					//When the timer was already 0 (because period was 0), it looks like the next
					//clock occurs earlier than expected.
					//This fixes the last test result in channel_1_nrx2_glitch (but may be incorrect)
					state.EnvTimer--;
				}
			}
		}
	}

	template<typename T, typename U>
	static void WriteRegister(T& state, uint8_t value, U& channel)
	{
		bool raiseVolume = (value & 0x08) != 0;
		uint8_t period = value & 0x07;

		if((value & 0xF8) == 0) {
			state.Enabled = false;
		} else {
			//No zombie mode for GBA? (or maybe it behaves differently.)
			//Using the GB implementation of zombie mode causes sound issues in some games
			//e.g popping can be heard in M&L Superstar Saga's menu at the start of the game
		}

		state.EnvPeriod = period;
		state.EnvRaiseVolume = raiseVolume;
		state.EnvVolume = (value & 0xF0) >> 4;

		channel.UpdateOutput();
	}
};
#pragma once
#include "pch.h"

class GbEnvelope
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
			//This implementation of the Zombie mode behavior differs from the description
			//found here: https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware
			//Instead, it's based on the behavior of the channel_1_nrx2_glitch test and
			//and SameBoy's implementation of the glitch
			bool preventIncrement = false;
			if(raiseVolume != state.EnvRaiseVolume) {
				if(raiseVolume) {
					if(!state.EnvStopped && state.EnvPeriod == 0) {
						state.Volume ^= 0x0F;
					} else {
						state.Volume = 14 - state.Volume;
					}
					preventIncrement = true;
				} else {
					//"If the mode was changed (add to subtract or subtract to add), volume is set to 16 - volume."
					state.Volume = 16 - state.Volume;
				}

				//"Only the low 4 bits of volume are kept"
				state.Volume &= 0xF;
			}

			if(!state.EnvStopped && !preventIncrement) {
				if(state.EnvPeriod == 0 && (period || raiseVolume)) {
					if(raiseVolume) {
						//"If the old envelope period was zero and the envelope is still doing automatic updates, volume is incremented by 1"
						state.Volume++;
					} else {
						state.Volume--;
					}

					//"Only the low 4 bits of volume are kept"
					state.Volume &= 0xF;
				}
			}
		}

		state.EnvPeriod = period;
		state.EnvRaiseVolume = raiseVolume;
		state.EnvVolume = (value & 0xF0) >> 4;

		channel.UpdateOutput();
	}
};
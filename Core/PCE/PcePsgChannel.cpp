#include "pch.h"
#include "PCE/PcePsgChannel.h"
#include "PCE/PcePsg.h"
#include "Utilities/Serializer.h"

PcePsgChannel::PcePsgChannel()
{
	_state.NoiseLfsr = 1;
}

void PcePsgChannel::Init(uint8_t index, PcePsg* psg)
{
	_chIndex = index;
	_psg = psg;
}

void PcePsgChannel::SetOutputOffset(uint8_t offset)
{
	_outputOffset = offset;
}

uint32_t PcePsgChannel::GetNoisePeriod()
{
	if(_state.NoiseFrequency == 0x1F) {
		return 32;
	} else {
		return ((~_state.NoiseFrequency) & 0x1F) * 64;
	}
}

uint32_t PcePsgChannel::GetPeriod()
{
	if(_state.DdaEnabled) {
		return 0;
	} else {
		uint32_t period = _state.Frequency;

		if(_chIndex == 0 && _psg->IsLfoEnabled()) {
			//When enabled, LFO alters channel 1's frequency/period
			period = (period + _psg->GetLfoCh1PeriodOffset()) & 0xFFF;
		}

		period = period ? period : 0x1000;

		if(_chIndex == 1 && _psg->IsLfoEnabled()) {
			//When enabled, LFO acts as a clock divider for channel 2
			//which reduces its frequency (increases its period)
			period *= _psg->GetLfoFrequency();
		}

		return period;
	}
}

void PcePsgChannel::Run(uint32_t clocks)
{
	if(_chIndex >= 4) {
		//Source: https://web.archive.org/web/20080311065543/http://cgfm2.emuviews.com:80/blog/index.php
		//"I wanted to see which registers affected the LFSR, but as it turns out none of them do except for the LFSR frequency control.
		//If the channel is turned on or off, noise is turned on or off, or if any other setting is adjusted, the LFSR always runs at 
		//the specified noise frequency. Apart from a reset, the LFSR state can't be changed."
		if(_state.NoiseTimer <= clocks) {
			_state.NoiseTimer = GetNoisePeriod();

			//Clock noise LSFR
			//"The LFSR was 18 bits, with bits (LSB) 0, 1, 11, 12, and 17 (MSB) tapped. Assuming a right-shift on each clock pulse,
			//the bit shifted out is the noise sample. The tapped bits are XOR'd together and inserted into bit 17. After /RESET is
			//asserted the LFSR is initialized with bit 0 set and all other bits reset. It has a maximum sequence of 131071 bits before repeating."
			uint32_t v = _state.NoiseLfsr;
			uint32_t bit = ((v >> 0) ^ (v >> 1) ^ (v >> 11) ^ (v >> 12) ^ (v >> 17)) & 0x01;

			_state.NoiseOutput = (int8_t)((_state.NoiseLfsr & 0x01) ? 0x1F : 0);
			_state.NoiseLfsr >>= 1;
			_state.NoiseLfsr |= (bit << 17);
		} else {
			_state.NoiseTimer -= clocks;
		}
	}

	if(_state.Enabled) {
		if(_state.DdaEnabled) {
			_state.CurrentOutput = (int8_t)_state.DdaOutputValue - _outputOffset;
		} else if(!_state.NoiseEnabled) {
			_state.Timer -= clocks;

			if(_state.Timer == 0) {
				_state.Timer = GetPeriod();
				_state.ReadAddr = (_state.ReadAddr + 1) & 0x1F;
			}

			_state.CurrentOutput = (int8_t)_state.WaveData[_state.ReadAddr] - _outputOffset;
		} else {
			_state.CurrentOutput = _state.NoiseOutput - _outputOffset;
		}
	} else {
		_state.CurrentOutput = 0;
	}
}

int16_t PcePsgChannel::GetOutput(bool forLeftChannel, uint8_t masterVolume)
{
	//Sound reduction constants (in -1.5dB steps)
	constexpr uint8_t volumeReduction[30] = { 255,214,180,151,127,107,90,76,64,53,45,38,32,27,22,19,16,13,11,9,8,6,5,4,4,3,2,2,2,1 };

	uint8_t reductionFactor = (0xF - masterVolume) * 2 + (0x1F - _state.Amplitude) + (0xF - (forLeftChannel ? _state.LeftVolume : _state.RightVolume)) * 2;
	if(reductionFactor >= 30) {
		//45+dB of reduction, channel is muted
		return 0;
	}

	return (int16_t)_state.CurrentOutput * volumeReduction[reductionFactor];
}

uint16_t PcePsgChannel::GetTimer()
{
	uint16_t minTimer = _chIndex >= 4 ? _state.NoiseTimer : 0;
	if(_state.Enabled && !_state.DdaEnabled && (minTimer == 0 || _state.Timer < minTimer)) {
		return _state.Timer;
	}
	return minTimer;
}

void PcePsgChannel::Write(uint16_t addr, uint8_t value)
{
	switch(addr & 0x0F) {
		case 2: _state.Frequency = (_state.Frequency & 0xF00) | value; break;
		case 3: _state.Frequency = (_state.Frequency & 0xFF) | ((value & 0x0F) << 8); break;

		case 4:
			if(_state.Enabled != ((value & 0x80) != 0)) {
				_state.Timer = GetPeriod();
				_state.Enabled = (value & 0x80) != 0;
			}

			_state.DdaEnabled = (value & 0x40) != 0;
			_state.Amplitude = (value & 0x1F);
			
			if(_state.DdaEnabled) {
				if(_state.Enabled) {
					//Update channel output immediately when DDA is enabled
					_state.CurrentOutput = (int8_t)_state.DdaOutputValue - _outputOffset;
				} else {
					_state.WriteAddr = 0;
				}
			}
			break;

		case 5:
			_state.RightVolume = value & 0x0F;
			_state.LeftVolume = (value >> 4) & 0x0F;
			break;

		case 6:
			if(_state.DdaEnabled) {
				_state.DdaOutputValue = value & 0x1F;
				if(_state.Enabled) {
					//Update channel output immediately with the new value when DDA is enabled
					_state.CurrentOutput = (int8_t)_state.DdaOutputValue - _outputOffset;
				}
			} else if(!_state.Enabled) {
				_state.WaveData[_state.WriteAddr] = value & 0x1F;
				_state.WriteAddr = (_state.WriteAddr + 1) & 0x1F;
			}
			break;

		case 7:
			_state.NoiseEnabled = (value & 0x80) != 0;
			_state.NoiseFrequency = (value & 0x1F);
			break;
	}
}

void PcePsgChannel::Serialize(Serializer& s)
{
	SV(_state.Amplitude);
	SV(_state.CurrentOutput);
	SV(_state.DdaEnabled);
	SV(_state.DdaOutputValue);
	SV(_state.Enabled);
	SV(_state.Frequency);
	SV(_state.LeftVolume);
	SV(_state.NoiseEnabled);
	SV(_state.NoiseFrequency);
	SV(_state.NoiseLfsr);
	SV(_state.NoiseOutput);
	SV(_state.NoiseTimer);
	SV(_state.ReadAddr);
	SV(_state.RightVolume);
	SV(_state.Timer);
	SV(_state.WriteAddr);

	SVArray(_state.WaveData, 0x20);
}

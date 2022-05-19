#include "stdafx.h"
#include "PCE/PcePsgChannel.h"
#include "PCE/PcePsg.h"

PcePsgChannel::PcePsgChannel()
{
	for(int i = 0; i < 0x1000; i++) {
		_noiseData[i] = RandomHelper::GetBool() ? 0x1F : 0x00;
	}
}

void PcePsgChannel::Init(uint8_t index, PcePsg* psg)
{
	_chIndex = index;
	_psg = psg;
}

uint32_t PcePsgChannel::GetPeriod()
{
	if(_state.DdaEnabled) {
		return 0;
	} else if(_state.NoiseEnabled) {
		if(_state.NoiseFrequency == 0x1F) {
			return (0x1F + 1) * 128;
		} else {
			return ((~_state.NoiseFrequency) & 0x1F) * 128;
		}
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
	if(_state.Enabled) {
		if(_state.DdaEnabled) {
			_state.CurrentOutput = (int8_t)_state.DdaOutputValue - 0x10;
		} else {
			_state.Timer -= clocks;

			if(_state.Timer == 0) {
				_state.Timer = GetPeriod();
				if(_state.NoiseEnabled) {
					_noiseAddr = (_noiseAddr + 1) & 0xFFF;
				} else {
					_state.ReadAddr = (_state.ReadAddr + 1) & 0x1F;
				}
			}

			if(_state.NoiseEnabled) {
				_state.CurrentOutput = (int8_t)_noiseData[_noiseAddr] - 0x10;
			} else {
				_state.CurrentOutput = (int8_t)_state.WaveData[_state.ReadAddr] - 0x10;
			}
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
	if(_state.Enabled && !_state.DdaEnabled) {
		return _state.Timer;
	}
	return 0;
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

		if(!_state.Enabled && _state.DdaEnabled) {
			_state.WriteAddr = 0;
		}

		break;

	case 5:
		_state.RightVolume = value & 0x0F;
		_state.LeftVolume = (value >> 4) & 0x0F;
		break;

	case 6:
		if(_state.DdaEnabled) {
			_state.DdaOutputValue = value & 0x1F;
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

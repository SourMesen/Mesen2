#pragma once
#include "stdafx.h"
#include "GbTypes.h"
#include "../Utilities/ISerializable.h"
#include "../Utilities/Serializer.h"

class GbSquareChannel : public ISerializable
{
private:
	const uint8_t _dutySequences[4][8] = {
		{ 0, 1, 1, 1, 1, 1, 1, 1 },
		{ 0, 0, 1, 1, 1, 1, 1, 1 },
		{ 0, 0, 0, 0, 1, 1, 1, 1 },
		{ 0, 0, 0, 0, 0, 0, 1, 1 }
	};

	GbSquareState _state = {};

public:
	GbSquareState GetState()
	{
		return _state;
	}

	bool Enabled()
	{
		return _state.Enabled;
	}

	void Disable()
	{
		uint8_t len = _state.Length;
		_state = {};
		_state.Length = len;
	}

	void ClockSweepUnit()
	{
		if(!_state.SweepEnabled) {
			return;
		}

		if(_state.SweepTimer > 0 && _state.SweepPeriod > 0) {
			_state.SweepTimer--;
			if(_state.SweepTimer == 0) {
				_state.SweepTimer = _state.SweepPeriod;

				//When it generates a clock and the sweep's internal enabled flag is set and the sweep period is not zero, a new frequency is calculated and the overflow
				uint16_t newFreq = GetSweepTargetFrequency();

				if(_state.SweepShift > 0 && newFreq < 2048) {
					//If the new frequency is 2047 or less and the sweep shift is not zero, this new frequency is written back to the shadow frequency and square 1's frequency in NR13 and NR14,
					_state.Frequency = _state.SweepFreq;
					_state.SweepFreq = newFreq;
					
					newFreq = GetSweepTargetFrequency();
					if(newFreq >= 2048) {
						//then frequency calculation and overflow check are run AGAIN immediately using this new value, but this second new frequency is not written back.
						_state.SweepEnabled = false;
						_state.Enabled = false;
					}
				} else {
					_state.SweepEnabled = false;
					_state.Enabled = false;
				}
			}
		}
	}

	uint16_t GetSweepTargetFrequency()
	{
		uint16_t shiftResult = (_state.SweepFreq >> _state.SweepShift);
		if(_state.SweepNegate) {
			return _state.SweepFreq - shiftResult;
		} else {
			return _state.SweepFreq + shiftResult;
		}
	}

	void ClockLengthCounter()
	{
		if(_state.LengthEnabled && _state.Length > 0) {
			_state.Length--;
			if(_state.Length == 0) {
				//"Length becoming 0 should clear status"
				_state.Enabled = false;
			}
		}
	}

	void ClockEnvelope()
	{
		if(_state.EnvTimer > 0) {
			_state.EnvTimer--;

			if(_state.EnvTimer == 0) {
				if(_state.EnvRaiseVolume && _state.Volume < 0x0F) {
					_state.Volume++;
				} else if(!_state.EnvRaiseVolume && _state.Volume > 0) {
					_state.Volume--;
				}

				_state.EnvTimer = _state.EnvPeriod;
			}
		}
	}

	uint8_t GetOutput()
	{
		return _state.Output;
	}

	void Exec(uint32_t clocksToRun)
	{
		_state.Timer -= clocksToRun;
		if(_state.Enabled) {
			_state.Output = _dutySequences[_state.Duty][_state.DutyPos] * _state.Volume;
		} else {
			_state.Output = 0;
		}


		if(_state.Timer == 0) {
			_state.Timer = (2048 - _state.Frequency) * 4;
			_state.DutyPos = (_state.DutyPos + 1) & 0x07;
		}
	}

	uint8_t Read(uint16_t addr)
	{
		constexpr uint8_t openBusBits[5] = { 0x80, 0x3F, 0x00, 0xFF, 0xBF };
		
		uint8_t value = 0;
		switch(addr) {
			case 0: 
				value = (
					(_state.SweepPeriod << 4) |
					(_state.SweepNegate ? 0x08 : 0) |
					_state.SweepShift
				);
				break;

			case 1: value = _state.Duty << 6; break;
			
			case 2:
				value = (
					(_state.EnvVolume << 4) |
					(_state.EnvRaiseVolume ? 0x08 : 0) |
					_state.EnvPeriod
				);
				break;

			case 4: value = _state.LengthEnabled ? 0x40 : 0; break;
		}

		return value | openBusBits[addr];
	}

	void Write(uint16_t addr, uint8_t value)
	{
		switch(addr) {
			case 0:
				_state.SweepShift = value & 0x07;
				_state.SweepNegate = (value & 0x08) != 0;
				_state.SweepPeriod = (value & 0x70) >> 4;
				break;

			case 1: 
				_state.Length = 64 - (value & 0x3F);
				_state.Duty = (value & 0xC0) >> 6;
				break;

			case 2: 
				_state.EnvPeriod = value & 0x07;
				_state.EnvRaiseVolume = (value & 0x08) != 0;
				_state.EnvVolume = (value & 0xF0) >> 4;

				if(!(value & 0xF8)) {
					_state.Enabled = false;
				}
				break;

			case 3:
				_state.Frequency = (_state.Frequency & 0x700) | value;
				break;

			case 4: 
				_state.Frequency = (_state.Frequency & 0xFF) | ((value & 0x07) << 8);
				_state.LengthEnabled = (value & 0x40) != 0;
				
				if(value & 0x80) {
					//Writing a value to NRx4 with bit 7 set causes the following things to occur :

					//Channel is enabled, if volume is not 0 or raise volume flag is set
					_state.Enabled = _state.EnvRaiseVolume || _state.EnvVolume > 0;

					//Frequency timer is reloaded with period.
					_state.Timer = (2048 - _state.Frequency) * 4;

					//If length counter is zero, it is set to 64 (256 for wave channel).
					if(_state.Length == 0) {
						_state.Length = 64;
					}

					//Volume envelope timer is reloaded with period.
					_state.EnvTimer = _state.EnvPeriod;
					
					//Channel volume is reloaded from NRx2.
					_state.Volume = _state.EnvVolume;

					//Sweep-related
					//During a trigger event, several things occur:
					//Square 1's frequency is copied to the shadow register.
					//The sweep timer is reloaded.
					//The internal enabled flag is set if either the sweep period or shift are non-zero, cleared otherwise.
					//If the sweep shift is non-zero, frequency calculation and the overflow check are performed immediately.
					_state.SweepFreq = _state.Frequency;
					_state.SweepTimer = _state.SweepPeriod;
					_state.SweepEnabled = _state.SweepPeriod > 0 || _state.SweepShift > 0;
					
					if(_state.SweepShift > 0) {
						_state.SweepFreq = GetSweepTargetFrequency();
						if(_state.SweepFreq > 2047) {
							_state.SweepEnabled = false;
							_state.Enabled = false;
						}
					}
				}
				break;
		}
	}

	void Serialize(Serializer& s) override
	{
		s.Stream(
			_state.SweepPeriod, _state.SweepNegate, _state.SweepShift, _state.SweepTimer, _state.SweepEnabled, _state.SweepFreq,
			_state.Volume, _state.EnvVolume, _state.EnvRaiseVolume, _state.EnvPeriod, _state.EnvTimer, _state.Duty, _state.Frequency,
			_state.Length, _state.LengthEnabled, _state.Enabled, _state.Timer, _state.DutyPos, _state.Output
		);
	}
};
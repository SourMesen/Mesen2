#pragma once
#include "pch.h"
#include "NES/Mappers/FDS/BaseFdsChannel.h"

class ModChannel : public BaseFdsChannel
{
private:
	const int32_t ModReset = 0xFF;
	const int32_t _modLut[8] = { 0,1,2,4,ModReset,-4,-2,-1 };

	int8_t _counter = 0;
	bool _modulationDisabled = false;
	bool _forceCarryOut = false;

	uint32_t _modAccumulator = 0;		//18-bit accumulator
	uint8_t _modM2Counter = 0;
	uint8_t _modTable[32] = {};
	uint8_t _modTablePosition = 0;
	int32_t _output = 0;

protected:
	void Serialize(Serializer& s) override
	{
		BaseFdsChannel::Serialize(s);
		
		SVArray(_modTable, 32);
		SV(_counter); SV(_modulationDisabled); SV(_forceCarryOut); SV(_modTablePosition); SV(_modAccumulator); SV(_modM2Counter); SV(_output);
	}

public:
	virtual void WriteReg(uint16_t addr, uint8_t value) override
	{
		switch(addr) {
			case 0x4084:
			case 0x4086:
				BaseFdsChannel::WriteReg(addr, value);
				break;
			case 0x4085:
				UpdateCounter(value & 0x7F);
				break;
			case 0x4087:
				BaseFdsChannel::WriteReg(addr, value);
				_modulationDisabled = (value & 0x80) == 0x80;
				// "4087.6 forces a carry out from bit 11."
				_forceCarryOut = (value & 0x40) == 0x40;
				if(_modulationDisabled) {
					// "Bits 0-12 are reset by 4087.7=1. Bits 13-17 have no reset."
					_modAccumulator &= 0x3F000;
				}
				break;
		}
	}

	virtual bool TickEnvelope(uint8_t wavePosition) override
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

	void WriteModTable(uint8_t value)
	{
		//"This register has no effect unless the mod unit is disabled via the high bit of $4087."
		if(_modulationDisabled) {
			// "ghost" modtable address bit(0) makes mod unit step thru each entry twice
			_modTable[_modTablePosition] = value & 0x07;
			_modTable[_modTablePosition] = value & 0x07;
			// "Writing $4088 increments the address (bits 13-17) when 4087.7=1."
			_modAccumulator += 0x2000;
			_modTablePosition = (_modAccumulator >> 13) & 0x1F;
		}
	}

	void UpdateCounter(int8_t value)
	{
		_counter = value;
		if(_counter >= 64) {
			_counter -= 128;
		} else if(_counter < -64) {
			_counter += 128;
		}
	}

	bool IsEnabled()
	{
		return !_modulationDisabled && _frequency > 0;
	}

	bool TickModulator()
	{
		if(IsEnabled()) {
			if(++_modM2Counter == 16) {
				_modAccumulator += _frequency;
				if(_modAccumulator > 0x3FFFF) {
					_modAccumulator -= 0x40000;
				}

				// "On a carry out from bit 11, update the mod counter (increment $4085 with modtable)."
				// "4087.6 forces a carry out from bit 11."
				if((_modAccumulator & 0xFFF) < _frequency || _forceCarryOut) {
					int32_t offset = _modLut[_modTable[_modTablePosition]];
					UpdateCounter(offset == ModReset ? 0 : _counter + offset);
					_modTablePosition = (_modAccumulator >> 13) & 0x1F;
				}

				_modM2Counter = 0;
				return true;
			}
		}
		return false;
	}

	void UpdateOutput(uint16_t volumePitch)
	{
		// code from new info by loopy
		// https://forums.nesdev.org/viewtopic.php?p=232662#p232662
		// pitch   = $4082/4083 (12-bit unsigned pitch value)
		// counter = $4085 (7-bit signed mod counter)
		// gain    = $4084 (6-bit unsigned mod gain)

		int32_t temp = _counter * _gain;
		if((temp & 0x0f) && !(temp & 0x800))
			temp += 0x20;
		temp += 0x400;
		temp = (temp >> 4) & 0xff;
		_output = temp;
	}

	int32_t GetOutput()
	{
		return _output;
	}

	int8_t GetCounter()
	{
		return _counter;
	}

	uint32_t GetModAccumulator()
	{
		return _modAccumulator;
	}

	bool GetForceCarryOut()
	{
		return _forceCarryOut;
	}

	bool IsModulationDisabled()
	{
		return _modulationDisabled;
	}
};

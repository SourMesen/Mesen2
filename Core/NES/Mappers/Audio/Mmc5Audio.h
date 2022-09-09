#pragma once
#include "pch.h"
#include "NES/APU/SquareChannel.h"
#include "NES/APU/BaseExpansionAudio.h"
#include "NES/APU/NesApu.h"
#include "NES/NesConsole.h"
#include "NES/NesMemoryManager.h"

class Mmc5Square : public SquareChannel
{
	int8_t _currentOutput = 0;

private:
	virtual void InitializeSweep(uint8_t regValue) override
	{
		//"$5001 has no effect. The MMC5 pulse channels will not sweep, as they have no sweep unit."
	}

public:
	Mmc5Square(NesConsole* console) : SquareChannel(AudioChannel::MMC5, console, false)
	{
		_currentOutput = 0;
		_isMmc5Square = true;
		Reset(false);
	}

	int8_t GetOutput()
	{
		return _currentOutput;
	}

	void RunChannel()
	{
		if(_timer.GetTimer() == 0) {
			_dutyPos = (_dutyPos - 1) & 0x07;
			//"Frequency values less than 8 do not silence the MMC5 pulse channels; they can output ultrasonic frequencies."
			_currentOutput = _dutySequences[_duty][_dutyPos] * _envelope.GetVolume();
			_timer.SetTimer(_timer.GetPeriod());
		} else {
			_timer.SetTimer(_timer.GetTimer() - 1);
		}
	}
};

class Mmc5Audio : public BaseExpansionAudio
{
private:
	Mmc5Square _square1;
	Mmc5Square _square2;
	int16_t _audioCounter = 0;
	int16_t _lastOutput = 0;

	bool _pcmReadMode = false;
	bool _pcmIrqEnabled = false;
	uint8_t _pcmOutput = 0;

protected:
	void Serialize(Serializer& s) override
	{
		BaseExpansionAudio::Serialize(s);

		SV(_square1);
		SV(_square2);
		SV(_audioCounter); SV(_lastOutput); SV(_pcmReadMode); SV(_pcmIrqEnabled); SV(_pcmOutput);
	}

	void ClockAudio() override
	{
		_audioCounter--;
		_square1.RunChannel();
		_square2.RunChannel();
		if(_audioCounter <= 0) {
			//~240hz envelope/length counter
			_audioCounter = _console->GetMasterClockRate() / 240;
			_square1.TickLengthCounter();
			_square1.TickEnvelope();
			_square2.TickLengthCounter();
			_square2.TickEnvelope();
		}

		//"The sound output of the square channels are equivalent in volume to the corresponding APU channels"
		//"The polarity of all MMC5 channels is reversed compared to the APU."
		int16_t summedOutput = -(_square1.GetOutput() + _square2.GetOutput() + _pcmOutput);
		if(summedOutput != _lastOutput) {
			_console->GetApu()->AddExpansionAudioDelta(AudioChannel::MMC5, summedOutput - _lastOutput);
			_lastOutput = summedOutput;
		}

		_square1.ReloadLengthCounter();
		_square2.ReloadLengthCounter();
	}

public:
	Mmc5Audio(NesConsole* console) : BaseExpansionAudio(console), _square1(console), _square2(console)
	{
		_audioCounter = 0;
		_lastOutput = 0;
		_pcmReadMode = false;
		_pcmIrqEnabled = false;
		_pcmOutput = 0;
	}

	uint8_t ReadRegister(uint16_t addr)
	{
		switch(addr) {
			case 0x5010:
				//TODO: PCM IRQ
				return 0;

			case 0x5015:
				uint8_t status = 0;
				status |= _square1.GetStatus() ? 0x01 : 0x00;
				status |= _square2.GetStatus() ? 0x02 : 0x00;
				return status;
		}

		return _console->GetMemoryManager()->GetOpenBus();
	}

	void WriteRegister(uint16_t addr, uint8_t value)
	{
		switch(addr) {
			case 0x5000: case 0x5001: case 0x5002: case 0x5003:
				_square1.WriteRam(addr, value);
				break;

			case 0x5004: case 0x5005: case 0x5006: case 0x5007:
				_square2.WriteRam(addr, value);
				break;

			case 0x5010:
				//TODO: Read mode & PCM IRQs are not implemented
				_pcmReadMode = (value & 0x01) == 0x01;
				_pcmIrqEnabled = (value & 0x80) == 0x80;
				break;

			case 0x5011:
				if(!_pcmReadMode) {
					if(value != 0) {
						_pcmOutput = value;
					}
				}
				break;

			case 0x5015:
				_square1.SetEnabled((value & 0x01) == 0x01);
				_square2.SetEnabled((value & 0x02) == 0x02);
				break;
		}
	}

	void GetMapperStateEntries(vector<MapperStateEntry>& entries)
	{
		ApuSquareState sq1 = _square1.GetState();
		ApuSquareState sq2 = _square2.GetState();

		entries.push_back(MapperStateEntry("$5000-$5015", "MMC5 Audio"));

		entries.push_back(MapperStateEntry("$5000-$5003", "MMC5 Square 1"));

		entries.push_back(MapperStateEntry("$5000.0-3", "Envelope Volume", sq1.Envelope.Volume, MapperStateValueType::Number8));
		entries.push_back(MapperStateEntry("$5000.4", "Envelope - Constant Volume", sq1.Envelope.ConstantVolume));
		entries.push_back(MapperStateEntry("$5000.5", "Length Counter - Halted", sq1.LengthCounter.Halt));
		entries.push_back(MapperStateEntry("$5000.6-7", "Duty", sq1.Duty, MapperStateValueType::Number8));

		entries.push_back(MapperStateEntry("$5002/$5003.0-2", "Period", sq1.Period, MapperStateValueType::Number16));
		entries.push_back(MapperStateEntry("$5003.3-7", "Length Counter - Reload Value", sq1.LengthCounter.ReloadValue, MapperStateValueType::Number16));

		entries.push_back(MapperStateEntry("--", "Enabled", sq1.Enabled));
		entries.push_back(MapperStateEntry("--", "Timer", sq1.Timer, MapperStateValueType::Number16));
		entries.push_back(MapperStateEntry("--", "Frequency", std::to_string(sq1.Frequency) + " Hz"));
		entries.push_back(MapperStateEntry("--", "Duty Position", sq1.DutyPosition, MapperStateValueType::Number8));

		entries.push_back(MapperStateEntry("--", "Length Counter - Counter", sq1.LengthCounter.Counter, MapperStateValueType::Number8));

		entries.push_back(MapperStateEntry("--", "Envelope - Counter", sq1.Envelope.Counter, MapperStateValueType::Number8));
		entries.push_back(MapperStateEntry("--", "Envelope - Divider", sq1.Envelope.Divider, MapperStateValueType::Number8));

		entries.push_back(MapperStateEntry("--", "Output", sq1.OutputVolume, MapperStateValueType::Number8));

		entries.push_back(MapperStateEntry("$5004-$5007", "MMC5 Square 2"));

		entries.push_back(MapperStateEntry("$5004.0-3", "Envelope Volume", sq2.Envelope.Volume, MapperStateValueType::Number8));
		entries.push_back(MapperStateEntry("$5004.4", "Envelope - Constant Volume", sq2.Envelope.ConstantVolume));
		entries.push_back(MapperStateEntry("$5004.5", "Length Counter - Halted", sq2.LengthCounter.Halt));
		entries.push_back(MapperStateEntry("$5004.6-7", "Duty", sq2.Duty, MapperStateValueType::Number8));

		entries.push_back(MapperStateEntry("$5005/$5006.0-2", "Period", sq2.Period, MapperStateValueType::Number16));
		entries.push_back(MapperStateEntry("$5006.3-7", "Length Counter - Reload Value", sq2.LengthCounter.ReloadValue, MapperStateValueType::Number16));

		entries.push_back(MapperStateEntry("--", "Enabled", sq2.Enabled));
		entries.push_back(MapperStateEntry("--", "Timer", sq2.Timer, MapperStateValueType::Number16));
		entries.push_back(MapperStateEntry("--", "Frequency", std::to_string(sq2.Frequency) + " Hz"));
		entries.push_back(MapperStateEntry("--", "Duty Position", sq2.DutyPosition, MapperStateValueType::Number8));

		entries.push_back(MapperStateEntry("--", "Length Counter - Counter", sq2.LengthCounter.Counter, MapperStateValueType::Number8));

		entries.push_back(MapperStateEntry("--", "Envelope - Counter", sq2.Envelope.Counter, MapperStateValueType::Number8));
		entries.push_back(MapperStateEntry("--", "Envelope - Divider", sq2.Envelope.Divider, MapperStateValueType::Number8));

		entries.push_back(MapperStateEntry("--", "Output", sq2.OutputVolume, MapperStateValueType::Number8));

		entries.push_back(MapperStateEntry("$5010-$5011", "PCM"));
		entries.push_back(MapperStateEntry("$5010.0", "PCM Read Mode", _pcmReadMode));
		entries.push_back(MapperStateEntry("$5010.7", "PCM IRQ Enabled", _pcmIrqEnabled));
		entries.push_back(MapperStateEntry("$5011", "PCM Output", _pcmOutput, MapperStateValueType::Number8));
	}
};

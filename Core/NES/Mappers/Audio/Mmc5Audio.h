#pragma once
#include "stdafx.h"
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

		s.Stream(&_square1);
		s.Stream(&_square2);
		s.Stream(_audioCounter, _lastOutput, _pcmReadMode, _pcmIrqEnabled, _pcmOutput);
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
};
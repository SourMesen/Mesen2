#pragma once
#include "pch.h"
#include "NES/APU/NesApu.h"
#include "NES/APU/ApuTimer.h"
#include "NES/APU/ApuEnvelope.h"
#include "NES/NesConstants.h"
#include "NES/NesConsole.h"
#include "NES/INesMemoryHandler.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"

class NoiseChannel : public INesMemoryHandler, public ISerializable
{
private:	
	static constexpr uint16_t _noisePeriodLookupTableNtsc[16] = { 4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068 };
	static constexpr uint16_t _noisePeriodLookupTablePal[16] = { 4, 8, 14, 30, 60, 88, 118, 148, 188, 236, 354, 472, 708,  944, 1890, 3778 };

	NesConsole* _console = nullptr;
	ApuEnvelope _envelope;
	ApuTimer _timer;

	//On power-up, the shift register is loaded with the value 1.
	uint16_t _shiftRegister = 1;
	bool _modeFlag = false;

	bool IsMuted()
	{
		//The mixer receives the current envelope volume except when Bit 0 of the shift register is set, or The length counter is zero
		return (_shiftRegister & 0x01) == 0x01;
	}

public:
	NoiseChannel(NesConsole* console) : _envelope(AudioChannel::Noise, console), _timer(AudioChannel::Noise, console->GetSoundMixer())
	{
		_console = console;
	}

	void Run(uint32_t targetCycle)
	{
		while(_timer.Run(targetCycle)) {
			//Feedback is calculated as the exclusive-OR of bit 0 and one other bit: bit 6 if Mode flag is set, otherwise bit 1.
			bool mode = _console->GetNesConfig().DisableNoiseModeFlag ? false : _modeFlag;

			uint16_t feedback = (_shiftRegister & 0x01) ^ ((_shiftRegister >> (mode ? 6 : 1)) & 0x01);
			_shiftRegister >>= 1;
			_shiftRegister |= (feedback << 14);

			if(IsMuted()) {
				_timer.AddOutput(0);
			} else {
				_timer.AddOutput(_envelope.GetVolume());
			}
		}
	}

	void TickEnvelope()
	{
		_envelope.TickEnvelope();
	}

	void TickLengthCounter()
	{
		_envelope.LengthCounter.TickLengthCounter();
	}

	void ReloadLengthCounter()
	{
		_envelope.LengthCounter.ReloadCounter();
	}

	void EndFrame()
	{
		_timer.EndFrame();
	}

	void SetEnabled(bool enabled)
	{
		_envelope.LengthCounter.SetEnabled(enabled);
	}

	bool GetStatus()
	{
		return _envelope.LengthCounter.GetStatus();
	}

	void Reset(bool softReset)
	{
		_envelope.Reset(softReset);
		_timer.Reset(softReset);

		_timer.SetPeriod((NesApu::GetApuRegion(_console) == ConsoleRegion::Ntsc ? _noisePeriodLookupTableNtsc : _noisePeriodLookupTablePal)[0] - 1);
		_shiftRegister = 1;
		_modeFlag = false;
	}

	void Serialize(Serializer& s) override
	{
		SV(_shiftRegister); SV(_modeFlag);
		SV(_envelope);
		SV(_timer);
	}

	void GetMemoryRanges(MemoryRanges &ranges) override
	{
		ranges.AddHandler(MemoryOperation::Write, 0x400C, 0x400F);
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		_console->GetApu()->Run();

		switch(addr & 0x03) {
			case 0:		//400C
				_envelope.InitializeEnvelope(value);
				break;

			case 2:		//400E
				_timer.SetPeriod((NesApu::GetApuRegion(_console) == ConsoleRegion::Ntsc ? _noisePeriodLookupTableNtsc : _noisePeriodLookupTablePal)[value & 0x0F] - 1);
				_modeFlag = (value & 0x80) == 0x80;
				break;

			case 3:		//400F
				_envelope.LengthCounter.LoadLengthCounter(value >> 3);

				//The envelope is also restarted.
				_envelope.ResetEnvelope();
				break;
		}
	}

	uint8_t GetOutput()
	{
		return _timer.GetLastOutput();
	}

	ApuNoiseState GetState()
	{
		ApuNoiseState state;
		state.Enabled = _envelope.LengthCounter.IsEnabled();
		state.Envelope = _envelope.GetState();
		state.Frequency = (double)NesConstants::GetClockRate(NesApu::GetApuRegion(_console)) / (_timer.GetPeriod() + 1) / (_modeFlag ? 93 : 1);
		state.LengthCounter = _envelope.LengthCounter.GetState();
		state.ModeFlag = _modeFlag;
		state.OutputVolume = _timer.GetLastOutput();
		state.Period = _timer.GetPeriod();
		state.Timer = _timer.GetTimer();
		state.ShiftRegister = _shiftRegister;
		return state;
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		return 0;
	}
};
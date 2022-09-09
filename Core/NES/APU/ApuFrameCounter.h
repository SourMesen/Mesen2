#pragma once
#include "pch.h"
#include "NES/INesMemoryHandler.h"
#include "NES/NesConsole.h"
#include "NES/NesCpu.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"

enum class FrameType
{
	None = 0,
	QuarterFrame = 1,
	HalfFrame = 2,
};

class ApuFrameCounter : public INesMemoryHandler, public ISerializable
{
private:
	const int32_t _stepCyclesNtsc[2][6] = { { 7457, 14913, 22371, 29828, 29829, 29830},
														 { 7457, 14913, 22371, 29829, 37281, 37282} };
	const int32_t _stepCyclesPal[2][6] =  { { 8313, 16627, 24939, 33252, 33253, 33254},
														 { 8313, 16627, 24939, 33253, 41565, 41566} };
	const FrameType _frameType[2][6] = { { FrameType::QuarterFrame, FrameType::HalfFrame, FrameType::QuarterFrame, FrameType::None, FrameType::HalfFrame, FrameType::None },
													 { FrameType::QuarterFrame, FrameType::HalfFrame, FrameType::QuarterFrame, FrameType::None, FrameType::HalfFrame, FrameType::None } };

	NesConsole* _console = nullptr;
	int32_t _stepCycles[2][6] = {};
	int32_t _previousCycle = 0;
	uint32_t _currentStep = 0;
	uint32_t _stepMode = 0; //0: 4-step mode, 1: 5-step mode
	bool _inhibitIRQ = false;
	uint8_t _blockFrameCounterTick = 0;
	int16_t _newValue = 0;
	int8_t _writeDelayCounter = 0;

public:
	ApuFrameCounter(NesConsole* console)
	{
		_console = console;
		Reset(false);
	}
	
	void Reset(bool softReset)
	{
		_previousCycle = 0;

		//"After reset: APU mode in $4017 was unchanged", so we need to keep whatever value _stepMode has for soft resets
		if(!softReset) {
			_stepMode = 0;
		}

		_currentStep = 0;


		//"After reset or power-up, APU acts as if $4017 were written with $00 from 9 to 12 clocks before first instruction begins."
		//This is emulated in the CPU::Reset function 
		//Reset acts as if $00 was written to $4017
		_newValue = _stepMode ? 0x80 : 0x00;
		_writeDelayCounter = 3;
		_inhibitIRQ = false;

		_blockFrameCounterTick = 0;
	}

	void Serialize(Serializer &s) override
	{
		SV(_previousCycle); SV(_currentStep); SV(_stepMode); SV(_inhibitIRQ); SV(_blockFrameCounterTick); SV(_writeDelayCounter); SV(_newValue);

		if(!s.IsSaving()) {
			SetRegion(_console->GetRegion());
		}
	}

	void SetRegion(ConsoleRegion region)
	{
		switch(region) {
			case ConsoleRegion::Auto:
				//Auto should never be set here
				break;

			case ConsoleRegion::Ntsc:
			case ConsoleRegion::Dendy:
				memcpy(_stepCycles, _stepCyclesNtsc, sizeof(_stepCycles));
				break;
				
			case ConsoleRegion::Pal:
				memcpy(_stepCycles, _stepCyclesPal, sizeof(_stepCycles));
				break;
		}
	}

	uint32_t Run(int32_t &cyclesToRun)
	{
		uint32_t cyclesRan;
		
		if(_previousCycle + cyclesToRun >= _stepCycles[_stepMode][_currentStep]) {
			if(!_inhibitIRQ && _stepMode == 0 && _currentStep >= 3) {
				//Set irq on the last 3 cycles for 4-step mode
				_console->GetCpu()->SetIrqSource(IRQSource::FrameCounter);
			}

			FrameType type = _frameType[_stepMode][_currentStep];
			if(type != FrameType::None && !_blockFrameCounterTick) {
				_console->GetApu()->FrameCounterTick(type);
				
				//Do not allow writes to 4017 to clock the frame counter for the next cycle (i.e this odd cycle + the following even cycle)
				_blockFrameCounterTick = 2;
			}

			if(_stepCycles[_stepMode][_currentStep] < _previousCycle) {
				//This can happen when switching from PAL to NTSC, which can cause a freeze (endless loop in APU)
				cyclesRan = 0;
			} else {
				cyclesRan = _stepCycles[_stepMode][_currentStep] - _previousCycle;
			}
			
			cyclesToRun -= cyclesRan;

			_currentStep++;
			if(_currentStep == 6) {
				_currentStep = 0;
				_previousCycle = 0;
			} else {
				_previousCycle += cyclesRan;
			}
		} else {
			cyclesRan = cyclesToRun;
			cyclesToRun = 0;
			_previousCycle += cyclesRan;
		}

		if(_newValue >= 0) {
			_writeDelayCounter--;
			if(_writeDelayCounter == 0) {
				//Apply new value after the appropriate number of cycles has elapsed
				_stepMode = ((_newValue & 0x80) == 0x80) ? 1 : 0;

				_writeDelayCounter = -1;
				_currentStep = 0;
				_previousCycle = 0;
				_newValue = -1;

				if(_stepMode && !_blockFrameCounterTick) {
					//"Writing to $4017 with bit 7 set will immediately generate a clock for both the quarter frame and the half frame units, regardless of what the sequencer is doing."
					_console->GetApu()->FrameCounterTick(FrameType::HalfFrame);
					_blockFrameCounterTick = 2;
				}
			}
		}
		
		if(_blockFrameCounterTick > 0) {
			_blockFrameCounterTick--;
		}

		return cyclesRan;
	}

	bool NeedToRun(uint32_t cyclesToRun)
	{
		//Run APU when:
		// -A new value is pending
		// -The "blockFrameCounterTick" process is running
		// -We're at the before-last or last tick of the current step
		return _newValue >= 0 || _blockFrameCounterTick > 0 || (_previousCycle + (int32_t)cyclesToRun >= _stepCycles[_stepMode][_currentStep] - 1);
	}

	void GetMemoryRanges(MemoryRanges &ranges) override
	{
		ranges.AddHandler(MemoryOperation::Write, 0x4017);
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		return 0;
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		_console->GetApu()->Run();
		_newValue = value;

		//Reset sequence after $4017 is written to
		if(_console->GetCpu()->GetCycleCount() & 0x01) {
			//"If the write occurs between APU cycles, the effects occur 4 CPU cycles after the write cycle. "
			_writeDelayCounter = 4;
		} else {
			//"If the write occurs during an APU cycle, the effects occur 3 CPU cycles after the $4017 write cycle"
			_writeDelayCounter = 3;
		}

		_inhibitIRQ = (value & 0x40) == 0x40;
		if(_inhibitIRQ) {
			_console->GetCpu()->ClearIrqSource(IRQSource::FrameCounter);
		}
	}

	ApuFrameCounterState GetState()
	{
		ApuFrameCounterState state;
		state.IrqEnabled = !_inhibitIRQ;
		state.SequencePosition = std::min<uint8_t>(_currentStep, _stepMode ? 5 : 4);
		state.FiveStepMode = _stepMode == 1;
		return state;
	}
};
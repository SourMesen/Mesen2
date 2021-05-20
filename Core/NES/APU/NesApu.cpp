#include "stdafx.h"
#include "NES/APU/NesApu.h"
#include "NES/APU/SquareChannel.h"
#include "NES/APU/TriangleChannel.h"
#include "NES/APU/NoiseChannel.h"
#include "NES/APU/DeltaModulationChannel.h"
#include "NES/APU/ApuFrameCounter.h"
#include "NES/NesCpu.h"
#include "NES/NesConsole.h"
#include "NES/NesTypes.h"
#include "NES/NesMemoryManager.h"
#include "NES/NesSoundMixer.h"
#include "Shared/Emulator.h"
#include "Utilities/Serializer.h"

NesApu::NesApu(NesConsole* console)
{
	_region = ConsoleRegion::Auto;
	_apuEnabled = true;
	_needToRun = false;

	_console = console;
	_mixer = _console->GetSoundMixer();
	_settings = _console->GetEmulator()->GetSettings();

	_squareChannel[0].reset(new SquareChannel(AudioChannel::Square1, _console, true));
	_squareChannel[1].reset(new SquareChannel(AudioChannel::Square2, _console, false));
	_triangleChannel.reset(new TriangleChannel(_console));
	_noiseChannel.reset(new NoiseChannel(_console));
	_deltaModulationChannel.reset(new DeltaModulationChannel(_console));
	_frameCounter.reset(new ApuFrameCounter(_console));

	_console->GetMemoryManager()->RegisterIODevice(_squareChannel[0].get());
	_console->GetMemoryManager()->RegisterIODevice(_squareChannel[1].get());
	_console->GetMemoryManager()->RegisterIODevice(_frameCounter.get());
	_console->GetMemoryManager()->RegisterIODevice(_triangleChannel.get());
	_console->GetMemoryManager()->RegisterIODevice(_noiseChannel.get());
	_console->GetMemoryManager()->RegisterIODevice(_deltaModulationChannel.get());

	Reset(false);
}

NesApu::~NesApu()
{
}

void NesApu::SetRegion(ConsoleRegion region, bool forceInit)
{
	//Finish the current apu frame before switching model
	Run();
	_frameCounter->SetRegion(region);
}

void NesApu::FrameCounterTick(FrameType type)
{
	//Quarter & half frame clock envelope & linear counter
	_squareChannel[0]->TickEnvelope();
	_squareChannel[1]->TickEnvelope();
	_triangleChannel->TickLinearCounter();
	_noiseChannel->TickEnvelope();

	if(type == FrameType::HalfFrame) {
		//Half frames clock length counter & sweep
		_squareChannel[0]->TickLengthCounter();
		_squareChannel[1]->TickLengthCounter();
		_triangleChannel->TickLengthCounter();
		_noiseChannel->TickLengthCounter();

		_squareChannel[0]->TickSweep();
		_squareChannel[1]->TickSweep();
	}
}

uint8_t NesApu::GetStatus()
{
	uint8_t status = 0;
	status |= _squareChannel[0]->GetStatus() ? 0x01 : 0x00;
	status |= _squareChannel[1]->GetStatus() ? 0x02 : 0x00;
	status |= _triangleChannel->GetStatus() ? 0x04 : 0x00;
	status |= _noiseChannel->GetStatus() ? 0x08 : 0x00;
	status |= _deltaModulationChannel->GetStatus() ? 0x10 : 0x00;
	status |= _console->GetCpu()->HasIrqSource(IRQSource::FrameCounter) ? 0x40 : 0x00;
	status |= _console->GetCpu()->HasIrqSource(IRQSource::DMC) ? 0x80 : 0x00;

	return status;
}

uint8_t NesApu::ReadRam(uint16_t addr)
{
	//$4015 read
	Run();

	uint8_t status = GetStatus();

	//Reading $4015 clears the Frame Counter interrupt flag.
	_console->GetCpu()->ClearIrqSource(IRQSource::FrameCounter);

	return status;
}

uint8_t NesApu::PeekRam(uint16_t addr)
{
	if(_console->GetEmulator()->GetEmulationThreadId() == std::this_thread::get_id()) {
		//Only run the Apu (to catch up) if we're running this in the emulation thread (not 100% accurate, but we can't run the Apu from any other thread without locking)
		Run();
	}
	return GetStatus();
}

void NesApu::WriteRam(uint16_t addr, uint8_t value)
{
	//$4015 write
	Run();

	//Writing to $4015 clears the DMC interrupt flag.
	//This needs to be done before setting the enabled flag for the DMC (because doing so can trigger an IRQ)
	_console->GetCpu()->ClearIrqSource(IRQSource::DMC);

	_squareChannel[0]->SetEnabled((value & 0x01) == 0x01);
	_squareChannel[1]->SetEnabled((value & 0x02) == 0x02);
	_triangleChannel->SetEnabled((value & 0x04) == 0x04);
	_noiseChannel->SetEnabled((value & 0x08) == 0x08);
	_deltaModulationChannel->SetEnabled((value & 0x10) == 0x10);
}

void NesApu::GetMemoryRanges(MemoryRanges &ranges)
{
	ranges.AddHandler(MemoryOperation::Read, 0x4015);
	ranges.AddHandler(MemoryOperation::Write, 0x4015);
}

void NesApu::Run()
{
	//Update framecounter and all channels
	//This is called:
	//-At the end of a frame
	//-Before Apu registers are read/written to
	//-When a DMC or FrameCounter interrupt needs to be fired
	int32_t cyclesToRun = _currentCycle - _previousCycle;

	while(cyclesToRun > 0) {
		_previousCycle += _frameCounter->Run(cyclesToRun);

		//Reload counters set by writes to 4003/4008/400B/400F after running the frame counter to allow the length counter to be clocked first
		//This fixes the test "len_reload_timing" (tests 4 & 5)
		_squareChannel[0]->ReloadLengthCounter();
		_squareChannel[1]->ReloadLengthCounter();
		_noiseChannel->ReloadLengthCounter();
		_triangleChannel->ReloadLengthCounter();

		_squareChannel[0]->Run(_previousCycle);
		_squareChannel[1]->Run(_previousCycle);
		_noiseChannel->Run(_previousCycle);
		_triangleChannel->Run(_previousCycle);
		_deltaModulationChannel->Run(_previousCycle);
	}
}

void NesApu::SetNeedToRun()
{
	_needToRun = true;
}

bool NesApu::NeedToRun(uint32_t currentCycle)
{
	if(_deltaModulationChannel->NeedToRun() || _needToRun) {
		//Need to run whenever we alter the length counters
		//Need to run every cycle when DMC is running to get accurate emulation (CPU stalling, interaction with sprite DMA, etc.)
		_needToRun = false;
		return true;
	}

	uint32_t cyclesToRun = currentCycle - _previousCycle;
	return _frameCounter->NeedToRun(cyclesToRun) || _deltaModulationChannel->IrqPending(cyclesToRun);
}

void NesApu::Exec()
{
	_currentCycle++;
	if(_currentCycle == NesSoundMixer::CycleLength - 1) {
		EndFrame();
	} else if(NeedToRun(_currentCycle)) {
		Run();
	}
}

void NesApu::EndFrame()
{
	Run();
	_squareChannel[0]->EndFrame();
	_squareChannel[1]->EndFrame();
	_triangleChannel->EndFrame();
	_noiseChannel->EndFrame();
	_deltaModulationChannel->EndFrame();

	_mixer->PlayAudioBuffer(_currentCycle);

	_currentCycle = 0;
	_previousCycle = 0;
}

void NesApu::ProcessCpuClock()
{
	if(_apuEnabled) {
		Exec();
	}
}

void NesApu::Reset(bool softReset)
{
	_apuEnabled = true;
	_currentCycle = 0;
	_previousCycle = 0;
	_squareChannel[0]->Reset(softReset);
	_squareChannel[1]->Reset(softReset);
	_triangleChannel->Reset(softReset);
	_noiseChannel->Reset(softReset);
	_deltaModulationChannel->Reset(softReset);
	_frameCounter->Reset(softReset);
}

void NesApu::Serialize(Serializer& s)
{
	if(s.IsSaving()) {
		//End the Apu frame - makes it simpler to restore sound after a state reload
		EndFrame();
	} else {
		_previousCycle = 0;
		_currentCycle = 0;
	}

	s.Stream(_squareChannel[0].get());
	s.Stream(_squareChannel[1].get());
	s.Stream(_triangleChannel.get());
	s.Stream(_noiseChannel.get());
	s.Stream(_deltaModulationChannel.get());
}

void NesApu::AddExpansionAudioDelta(AudioChannel channel, int16_t delta)
{
	_mixer->AddDelta(channel, _currentCycle, delta);
}

void NesApu::SetApuStatus(bool enabled)
{
	_apuEnabled = enabled;
}

bool NesApu::IsApuEnabled()
{
	//Adding extra lines before/after NMI temporarely turns off the Apu
	//This appears to result in less side-effects than spreading out the Apu's
	//load over the entire PPU frame, like what was done before.
	//This is most likely due to the timing of the Frame Counter & DMC IRQs.
	return _apuEnabled;
}

ConsoleRegion NesApu::GetApuRegion(NesConsole* console)
{
	ConsoleRegion region = console->GetRegion();
	if(region == ConsoleRegion::Ntsc || region == ConsoleRegion::Dendy) {
		//Dendy APU works with NTSC timings
		return ConsoleRegion::Ntsc;
	} else {
		return region;
	}
}

uint16_t NesApu::GetDmcReadAddress()
{
	return _deltaModulationChannel->GetDmcReadAddress();
}

void NesApu::SetDmcReadBuffer(uint8_t value)
{
	_deltaModulationChannel->SetDmcReadBuffer(value);
}

ApuState NesApu::GetState()
{
	ApuState state;
	state.Dmc = _deltaModulationChannel->GetState();
	state.FrameCounter = _frameCounter->GetState();
	state.Noise = _noiseChannel->GetState();
	state.Square1 = _squareChannel[0]->GetState();
	state.Square2 = _squareChannel[1]->GetState();
	state.Triangle = _triangleChannel->GetState();
	return state;
}

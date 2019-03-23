#include "stdafx.h"
#include "Spc.h"
#include "Console.h"
#include "MemoryManager.h"
#include "SoundMixer.h"
#include "SPC_DSP.h"
#include "../Utilities/Serializer.h"

Spc::Spc(shared_ptr<Console> console, vector<uint8_t> &spcRomData)
{
	_console = console;
	_soundBuffer = new int16_t[Spc::SampleBufferSize];
	_immediateMode = false;
	_operandA = 0;
	_operandB = 0;

	_dsp.reset(new SPC_DSP());
	_dsp->init(_ram);
	_dsp->reset();
	_dsp->set_output(_soundBuffer, Spc::SampleBufferSize >> 1);

	memcpy(_spcBios, spcRomData.data(), 64);
	memset(_ram, 0, sizeof(_ram));

	_state = {};
	_state.WriteEnabled = true;
	_state.TimersEnabled = true;
	_state.RomEnabled = true;
	_state.SP = 0xFF;
	_state.PC = ReadWord(Spc::ResetVector);
}

Spc::~Spc()
{
	delete[] _soundBuffer;
}

void Spc::Reset()
{
	_state.Timer0.Reset();
	_state.Timer1.Reset();
	_state.Timer2.Reset();
	_state.PC = ReadWord(Spc::ResetVector);

	_dsp->soft_reset();
	_dsp->set_output(_soundBuffer, Spc::SampleBufferSize >> 1);
}

void Spc::Idle()
{
	IncCycleCount(-1);
}

void Spc::DummyRead()
{
	Read(_state.PC);
}

void Spc::DummyRead(uint16_t addr)
{
	Read(addr);
}

void Spc::IncCycleCount(int32_t addr)
{
	static constexpr uint8_t cpuWait[4] = { 2, 4, 10, 20 };
	static constexpr uint8_t timerMultiplier[4] = { 2, 4, 8, 16 };

	uint8_t speedSelect;
	if(addr < 0 || ((addr & 0xFFF0) == 0x00F0) || (addr >= 0xFFC0 && _state.RomEnabled)) {
		//Use internal speed (bits 4-5) for idle cycles, register access or IPL rom access
		speedSelect = _state.InternalSpeed;
	} else {
		speedSelect = _state.ExternalSpeed;
	}

	_state.Cycle += cpuWait[speedSelect];
	_dsp->run();
	
	uint8_t timerInc = timerMultiplier[speedSelect];
	_state.Timer0.Run(timerInc);
	_state.Timer1.Run(timerInc);
	_state.Timer2.Run(timerInc);
}

uint8_t Spc::Read(uint16_t addr, MemoryOperationType type)
{
	IncCycleCount(addr);

	if(addr >= 0xFFC0 && _state.RomEnabled) {
		return _spcBios[addr & 0x3F];
	}

	switch(addr) {
		case 0xF0: return 0;
		case 0xF1: return 0;

		case 0xF2: return _state.DspReg;
		case 0xF3: return _dsp->read(_state.DspReg & 0x7F);
			
		case 0xF4: return _state.CpuRegs[0];
		case 0xF5: return _state.CpuRegs[1];
		case 0xF6: return _state.CpuRegs[2];
		case 0xF7: return _state.CpuRegs[3];

		case 0xF8: return _state.RamReg[0];
		case 0xF9: return _state.RamReg[1];

		case 0xFA: return 0;
		case 0xFB: return 0;
		case 0xFC: return 0;

		case 0xFD: return _state.Timer0.GetOutput();
		case 0xFE: return _state.Timer1.GetOutput();
		case 0xFF: return _state.Timer2.GetOutput();
	}

	return _ram[addr];
}

void Spc::Write(uint16_t addr, uint8_t value, MemoryOperationType type)
{
	IncCycleCount(addr);

	//Writes always affect the underlying RAM
	if(_state.WriteEnabled) {
		_ram[addr] = value;
	}

	switch(addr) {
		case 0xF0: 
			if(!CheckFlag(SpcFlags::DirectPage)) {
				_state.InternalSpeed = (value >> 6) & 0x03;
				_state.ExternalSpeed = (value >> 4) & 0x03;
				_state.TimersEnabled = (value & 0x09) == 0x08;
				_state.WriteEnabled = value & 0x02;

				_dsp->setEchoWriteEnabled(_state.WriteEnabled);

				_state.Timer0.SetGlobalEnabled(_state.TimersEnabled);
				_state.Timer1.SetGlobalEnabled(_state.TimersEnabled);
				_state.Timer2.SetGlobalEnabled(_state.TimersEnabled);
			}
			break;

		case 0xF1:
			if(value & SpcControlFlags::ClearPortsA) {
				_state.CpuRegs[0] = _state.CpuRegs[1] = 0;
			}
			if(value & SpcControlFlags::ClearPortsB) {
				_state.CpuRegs[2] = _state.CpuRegs[3] = 0;
			}

			_state.Timer0.SetEnabled((value & SpcControlFlags::Timer0) != 0);
			_state.Timer1.SetEnabled((value & SpcControlFlags::Timer1) != 0);
			_state.Timer2.SetEnabled((value & SpcControlFlags::Timer2) != 0);

			_state.RomEnabled = (value & SpcControlFlags::EnableRom) != 0;
			break;

		case 0xF2: _state.DspReg = value; break;
		case 0xF3: 
			if(_state.DspReg < 128) {
				_dsp->write(_state.DspReg, value);
			}
			break;

		case 0xF4: _state.OutputReg[0] = value; break;
		case 0xF5: _state.OutputReg[1] = value; break;
		case 0xF6: _state.OutputReg[2] = value; break;
		case 0xF7: _state.OutputReg[3] = value; break;
		case 0xF8: _state.RamReg[0] = value; break;
		case 0xF9: _state.RamReg[1] = value; break;

		case 0xFA: _state.Timer0.SetTarget(value); break;
		case 0xFB: _state.Timer1.SetTarget(value); break;
		case 0xFC: _state.Timer2.SetTarget(value); break;

		case 0xFD: break;
		case 0xFE: break;
		case 0xFF: break;
	}
}

uint8_t Spc::CpuReadRegister(uint16_t addr)
{
	Run();
	return _state.OutputReg[addr & 0x03];
}

void Spc::CpuWriteRegister(uint32_t addr, uint8_t value)
{
	Run();
	_state.CpuRegs[addr & 0x03] = value;
}

void Spc::Run()
{
	int64_t masterClock = _console->GetMemoryManager()->GetMasterClock();
	//TODO: This will overflow after 100+ hours, needs to be fixed
	uint64_t targetCycle = (masterClock * (uint64_t)1024000 / (uint64_t)_console->GetMasterClockRate()) * 2;
	while(_state.Cycle < targetCycle) {
		Exec();
	}
}

void Spc::ProcessEndFrame()
{
	Run();

	int sampleCount = _dsp->sample_count();
	_console->GetSoundMixer()->PlayAudioBuffer(_soundBuffer, sampleCount / 2);
	_dsp->set_output(_soundBuffer, Spc::SampleBufferSize >> 1);
}

void Spc::Serialize(Serializer &s)
{
	s.Stream(_state.A, _state.Cycle, _state.PC, _state.PS, _state.SP, _state.X, _state.Y);
	s.Stream(_state.CpuRegs[0], _state.CpuRegs[1], _state.CpuRegs[2], _state.CpuRegs[3]);
	s.Stream(_state.OutputReg[0], _state.OutputReg[1], _state.OutputReg[2], _state.OutputReg[3]);
	s.Stream(_state.RamReg[0], _state.RamReg[1]);
	s.Stream(_state.ExternalSpeed, _state.InternalSpeed, _state.WriteEnabled, _state.TimersEnabled);
	s.Stream(_state.DspReg, _state.RomEnabled);

	_state.Timer0.Serialize(s);
	_state.Timer1.Serialize(s);
	_state.Timer2.Serialize(s);

	ArrayInfo<uint8_t> ram { _ram, Spc::SpcRamSize };
	s.Stream(ram, _state.DspReg, _state.RomEnabled);

	uint8_t dspState[SPC_DSP::state_size];
	memset(dspState, 0, SPC_DSP::state_size);
	if(s.IsSaving()) {
		uint8_t *out = dspState;
		_dsp->copy_state(&out, [](uint8_t** output, void* in, size_t size) {
			memcpy(*output, in, size);
			*output += size;
		});

		s.StreamArray(dspState, SPC_DSP::state_size);
	} else {
		s.StreamArray(dspState, SPC_DSP::state_size);

		uint8_t *in = dspState;
		_dsp->copy_state(&in, [](uint8_t** input, void* output, size_t size) {
			memcpy(output, *input, size);
			*input += size;
		});

		_dsp->set_output(_soundBuffer, Spc::SampleBufferSize >> 1);
	}
}

uint8_t Spc::GetOpCode()
{
	return Read(_state.PC++, MemoryOperationType::ExecOpCode);
}

uint8_t Spc::ReadOperandByte()
{
	return Read(_state.PC++, MemoryOperationType::ExecOperand);
}

uint16_t Spc::ReadWord(uint16_t addr, MemoryOperationType type)
{
	uint8_t lsb = Read(addr, type);
	uint8_t msb = Read(addr + 1, type);
	return (msb << 8) | lsb;
}

void Spc::ClearFlags(uint8_t flags)
{
	_state.PS &= ~flags;
}

void Spc::SetFlags(uint8_t flags)
{
	_state.PS |= flags;
}

bool Spc::CheckFlag(uint8_t flag)
{
	return (_state.PS & flag) == flag;
}

void Spc::SetZeroNegativeFlags(uint8_t value)
{
	ClearFlags(SpcFlags::Zero | SpcFlags::Negative);
	if(value == 0) {
		SetFlags(SpcFlags::Zero);
	} else if(value & 0x80) {
		SetFlags(SpcFlags::Negative);
	}
}

void Spc::SetZeroNegativeFlags16(uint16_t value)
{
	ClearFlags(SpcFlags::Zero | SpcFlags::Negative);
	if(value == 0) {
		SetFlags(SpcFlags::Zero);
	} else if(value & 0x8000) {
		SetFlags(SpcFlags::Negative);
	}
}

uint8_t Spc::GetByteValue()
{
	if(_immediateMode) {
		return (uint8_t)_operandA;
	} else {
		return Read(_operandA);
	}
}

uint16_t Spc::GetWordValue()
{
	if(_immediateMode) {
		return (uint16_t)_operandA;
	} else {
		return ReadWord(_operandA);
	}
}

void Spc::Push(uint8_t value)
{
	Write(0x100 | _state.SP, value);
	_state.SP--;
}

uint8_t Spc::Pop()
{
	_state.SP++;
	return Read(0x100 | _state.SP);
}

void Spc::PushWord(uint16_t value)
{
	Push(value >> 8);
	Push((uint8_t)value);
}

uint16_t Spc::PopWord()
{
	uint8_t lo = Pop();
	uint8_t hi = Pop();
	return lo | hi << 8;
}

uint16_t Spc::GetDirectAddress(uint8_t offset)
{
	return (CheckFlag(SpcFlags::DirectPage) ? 0x100 : 0) + offset;
}

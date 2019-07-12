#include "stdafx.h"
#include "Spc.h"
#include "Console.h"
#include "MemoryManager.h"
#include "SoundMixer.h"
#include "EmuSettings.h"
#include "SPC_DSP.h"
#include "../Utilities/Serializer.h"

Spc::Spc(shared_ptr<Console> console)
{
	_console = console;
	_memoryManager = console->GetMemoryManager();
	_soundBuffer = new int16_t[Spc::SampleBufferSize];

	_ram = new uint8_t[Spc::SpcRamSize];
	_console->GetSettings()->InitializeRam(_ram, Spc::SpcRamSize);

	_dsp.reset(new SPC_DSP());
	_dsp->init(_ram);
	_dsp->reset();
	_dsp->set_output(_soundBuffer, Spc::SampleBufferSize >> 1);

	_state = {};
	_state.WriteEnabled = true;
	_state.TimersEnabled = true;
	_state.RomEnabled = true;
	_state.SP = 0xFF;
	_state.PC = ReadWord(Spc::ResetVector);
	_state.StopState = CpuStopState::Running;

	_opCode = 0;
	_opStep = SpcOpStep::ReadOpCode;
	_opSubStep = 0;
	_tmp1 = 0;
	_tmp2 = 0;
	_tmp3 = 0;
	_operandA = 0;
	_operandB = 0;

	_clockRatio = (double)2048000 / _console->GetMasterClockRate();
}

#ifndef DUMMYSPC
Spc::~Spc()
{
	delete[] _soundBuffer;
	delete[] _ram;
}
#endif

void Spc::Reset()
{
	_state.StopState = CpuStopState::Running;

	_state.Timer0.Reset();
	_state.Timer1.Reset();
	_state.Timer2.Reset();
	
	_state.RomEnabled = true;
	_state.Cycle = 0;
	_state.PC = ReadWord(Spc::ResetVector);
	
	_opCode = 0;
	_opStep = SpcOpStep::ReadOpCode;
	_opSubStep = 0;
	_tmp1 = 0;
	_tmp2 = 0;
	_tmp3 = 0;
	_operandA = 0;
	_operandB = 0;

	_dsp->soft_reset();
	_dsp->set_output(_soundBuffer, Spc::SampleBufferSize >> 1);
}

void Spc::Idle()
{
	IncCycleCount(-1);
}

void Spc::DummyRead()
{
	Read(_state.PC, MemoryOperationType::DummyRead);
}

void Spc::DummyRead(uint16_t addr)
{
	Read(addr, MemoryOperationType::DummyRead);
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
#ifndef DUMMYSPC
	_dsp->run();
#endif

	uint8_t timerInc = timerMultiplier[speedSelect];
	_state.Timer0.Run(timerInc);
	_state.Timer1.Run(timerInc);
	_state.Timer2.Run(timerInc);
}

uint8_t Spc::DebugRead(uint16_t addr)
{
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

		case 0xFD: return _state.Timer0.DebugRead();
		case 0xFE: return _state.Timer1.DebugRead();
		case 0xFF: return _state.Timer2.DebugRead();

		default: return _ram[addr];
	}
}

void Spc::DebugWrite(uint16_t addr, uint8_t value)
{
	_ram[addr] = value;
}

uint8_t Spc::Read(uint16_t addr, MemoryOperationType type)
{
	IncCycleCount(addr);

	uint8_t value;
	if(addr >= 0xFFC0 && _state.RomEnabled) {
		value = _spcBios[addr & 0x3F];
	} else {
		switch(addr) {
			case 0xF0: value = 0; break;
			case 0xF1: value = 0; break;

			case 0xF2: value = _state.DspReg; break;
			case 0xF3: 
				#ifndef DUMMYSPC
				value = _dsp->read(_state.DspReg & 0x7F);
				#else
				value = 0;
				#endif
				break;

			case 0xF4: value = _state.CpuRegs[0]; break;
			case 0xF5: value = _state.CpuRegs[1]; break;
			case 0xF6: value = _state.CpuRegs[2]; break;
			case 0xF7: value = _state.CpuRegs[3]; break;

			case 0xF8: value = _state.RamReg[0]; break;
			case 0xF9: value = _state.RamReg[1]; break;

			case 0xFA: value = 0; break;
			case 0xFB: value = 0; break;
			case 0xFC: value = 0; break;

			case 0xFD: value = _state.Timer0.GetOutput(); break;
			case 0xFE: value = _state.Timer1.GetOutput(); break;
			case 0xFF: value = _state.Timer2.GetOutput(); break;

			default: value = _ram[addr]; break;
		}
	}

#ifndef DUMMYSPC
	_console->ProcessSpcRead(addr, value, type);
#else 
	LogRead(addr, value);
#endif

	return value;
}

void Spc::Write(uint16_t addr, uint8_t value, MemoryOperationType type)
{
	IncCycleCount(addr);

#ifdef DUMMYSPC
	LogWrite(addr, value);
#else

	//Writes always affect the underlying RAM
	if(_state.WriteEnabled) {
		_console->ProcessSpcWrite(addr, value, type);
		_ram[addr] = value;
	}

	switch(addr) {
		case 0xF0: 
			if(!CheckFlag(SpcFlags::DirectPage)) {
				_state.InternalSpeed = (value >> 6) & 0x03;
				_state.ExternalSpeed = (value >> 4) & 0x03;
				_state.TimersEnabled = (value & 0x09) == 0x08;
				_state.WriteEnabled = value & 0x02;

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
#endif
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
	if(_state.StopState != CpuStopState::Running) {
		//STOP or SLEEP were executed - execution is stopped forever.
		return;
	}

	uint64_t targetCycle = (uint64_t)(_memoryManager->GetMasterClock() * _clockRatio);
	while(_state.Cycle < targetCycle) {
		if(_opStep == SpcOpStep::ReadOpCode) {
			_opCode = GetOpCode();
			_opStep = SpcOpStep::Addressing;
			_opSubStep = 0;
		} else {
			Exec();
		}
	}
}

void Spc::ProcessEndFrame()
{
	Run();

	_clockRatio = (double)2048000 / _console->GetMasterClockRate();

	int sampleCount = _dsp->sample_count();
	if(sampleCount != 0) {
		_console->GetSoundMixer()->PlayAudioBuffer(_soundBuffer, sampleCount / 2);
	}
	_dsp->set_output(_soundBuffer, Spc::SampleBufferSize >> 1);
}

SpcState Spc::GetState()
{
	return _state;
}

AddressInfo Spc::GetAbsoluteAddress(uint16_t addr)
{
	if(addr < 0xFFC0 || !_state.RomEnabled) {
		return AddressInfo { addr, SnesMemoryType::SpcRam };
	}
	return AddressInfo { addr & 0x3F, SnesMemoryType::SpcRom };
}

int Spc::GetRelativeAddress(AddressInfo &absAddress)
{
	if(absAddress.Type == SnesMemoryType::SpcRom) {
		if(_state.RomEnabled) {
			return 0xFFC0 | (absAddress.Address & 0x3F);
		}
	} else {
		if(absAddress.Address < 0xFFC0 || !_state.RomEnabled) {
			return absAddress.Address;
		}
	}
	return -1;
}

uint8_t* Spc::GetSpcRam()
{
	return _ram;
}

uint8_t* Spc::GetSpcRom()
{
	return _spcBios;
}

void Spc::Serialize(Serializer &s)
{
	s.Stream(_state.A, _state.Cycle, _state.PC, _state.PS, _state.SP, _state.X, _state.Y);
	s.Stream(_state.CpuRegs[0], _state.CpuRegs[1], _state.CpuRegs[2], _state.CpuRegs[3]);
	s.Stream(_state.OutputReg[0], _state.OutputReg[1], _state.OutputReg[2], _state.OutputReg[3]);
	s.Stream(_state.RamReg[0], _state.RamReg[1]);
	s.Stream(_state.ExternalSpeed, _state.InternalSpeed, _state.WriteEnabled, _state.TimersEnabled);
	s.Stream(_state.DspReg, _state.RomEnabled, _clockRatio);

	_state.Timer0.Serialize(s);
	_state.Timer1.Serialize(s);
	_state.Timer2.Serialize(s);

	ArrayInfo<uint8_t> ram { _ram, Spc::SpcRamSize };
	s.Stream(ram);

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

	s.Stream(_operandA, _operandB, _tmp1, _tmp2, _tmp3, _opCode, _opStep, _opSubStep);
}

uint8_t Spc::GetOpCode()
{
	uint8_t value = Read(_state.PC, MemoryOperationType::ExecOpCode);
	_state.PC++;
	return value;
}

uint8_t Spc::ReadOperandByte()
{
	uint8_t value = Read(_state.PC, MemoryOperationType::ExecOperand);
	_state.PC++;
	return value;
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
	return Read(_operandA);
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

uint16_t Spc::GetDirectAddress(uint8_t offset)
{
	return (CheckFlag(SpcFlags::DirectPage) ? 0x100 : 0) + offset;
}

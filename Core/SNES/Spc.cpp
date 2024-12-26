#include "pch.h"
#include "SNES/Spc.h"
#include "SNES/SnesConsole.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/SpcFileData.h"
#ifndef DUMMYSPC
#include "SNES/DSP/Dsp.h"
#else
#undef Spc
#undef DUMMYSPC
#include "SNES/DSP/Dsp.h"
#define Spc DummySpc
#define DUMMYSPC
#endif
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/Audio/SoundMixer.h"
#include "Utilities/Serializer.h"
#include "Shared/MemoryOperationType.h"

Spc::Spc(SnesConsole* console)
{
	_emu = console->GetEmulator();
	_console = console;
	_memoryManager = console->GetMemoryManager();

	_ram = new uint8_t[Spc::SpcRamSize];
	_emu->RegisterMemory(MemoryType::SpcRam, _ram, Spc::SpcRamSize);
	_console->InitializeRam(_ram, Spc::SpcRamSize);

	_emu->RegisterMemory(MemoryType::SpcRom, _spcBios, Spc::SpcRomSize);

#ifndef DUMMYSPC
	_dsp.reset(new Dsp(_emu, console, this));
#endif

	_state = {};
	_state.WriteEnabled = true;
	_state.TimersEnabled = true;
	_state.RomEnabled = true;
	_state.SP = 0xFF;
	_state.PC = ReadWord(Spc::ResetVector);
	_state.StopState = SnesCpuStopState::Running;

	_opCode = 0;
	_opStep = SpcOpStep::ReadOpCode;
	_opSubStep = 0;
	_tmp1 = 0;
	_tmp2 = 0;
	_tmp3 = 0;
	_operandA = 0;
	_operandB = 0;
	_enabled = true;
	_spcSampleRate = Spc::SpcSampleRate + _emu->GetSettings()->GetSnesConfig().SpcClockSpeedAdjustment;

	UpdateClockRatio();
}

#ifndef DUMMYSPC
Spc::~Spc()
{
	delete[] _ram;
}
#endif

void Spc::Reset()
{
	_state.StopState = SnesCpuStopState::Running;

	_state.Timer0.Reset();
	_state.Timer1.Reset();
	_state.Timer2.Reset();

	//Resetting appears to reset the values the main CPU can read (not doing this causes a freeze in Kaite Tsukette Asoberu Dezaemon)
	_state.OutputReg[0] = 0;
	_state.OutputReg[1] = 0;
	_state.OutputReg[2] = 0;
	_state.OutputReg[3] = 0;

	//Reset the values the SPC can read from the port, too (not doing this freezes Ranma Chounai Gekitou Hen on reset)
	_state.NewCpuRegs[0] = _state.CpuRegs[0] = 0;
	_state.NewCpuRegs[1] = _state.CpuRegs[1] = 0;
	_state.NewCpuRegs[2] = _state.CpuRegs[2] = 0;
	_state.NewCpuRegs[3] = _state.CpuRegs[3] = 0;

	_state.RomEnabled = true;
	_state.Cycle = 0;
	_state.PC = ReadWord(Spc::ResetVector);
	_state.A = 0;
	_state.X = 0;
	_state.Y = 0;
	_state.SP = 0xFF;

	//Clear P (and other flags) - if P is set after reset, the IPL ROM doesn't work properly
	_state.PS = 0;
	
	_opCode = 0;
	_opStep = SpcOpStep::ReadOpCode;
	_opSubStep = 0;
	_tmp1 = 0;
	_tmp2 = 0;
	_tmp3 = 0;
	_operandA = 0;
	_operandB = 0;

	_dsp->Reset();
}

void Spc::SetSpcState(bool enabled)
{
	//Used by overclocking logic to disable SPC during the extra scanlines added to the PPU
	if(_enabled != enabled) {
		if(enabled) {
			//When re-enabling, adjust the cycle counter to prevent running extra cycles
			UpdateClockRatio();
		} else {
			//Catch up SPC before disabling it
			Run();
		}
		_enabled = enabled;
	}
}

void Spc::UpdateClockRatio()
{
	_clockRatio = (double)(_spcSampleRate * 64) / _console->GetMasterClockRate();

	//If the target cycle is off by more than 20 cycles, reset the counter to match what was expected
	//This can happen due to overclocking (which disables the SPC for some scanlines) or if the SPC's 
	//internal sample rate is changed between versions (e.g 32000hz -> 32040hz)
	uint64_t targetCycle = (uint64_t)(_memoryManager->GetMasterClock() * _clockRatio);
	if(std::abs((int64_t)targetCycle - (int64_t)_state.Cycle) > 20) {
		_state.Cycle = targetCycle;
	}
}

void Spc::ExitExecLoop()
{
#ifndef DUMMYSPC
	_state.Cycle = _memoryManager->GetMasterClock() * _clockRatio;
#endif
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
	_dsp->Exec();
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
		case 0xF3: return _dsp->Read(_state.DspReg & 0x7F);

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

void Spc::DebugWriteDspReg(uint8_t addr, uint8_t value)
{
	_dsp->Write(addr, value);
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
				value = _dsp->Read(_state.DspReg & 0x7F);
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
	_emu->ProcessMemoryRead<CpuType::Spc>(addr, value, type);
#else 
	LogMemoryOperation(addr, value, type);
#endif

	return value;
}

void Spc::Write(uint16_t addr, uint8_t value, MemoryOperationType type)
{
	IncCycleCount(addr);

#ifdef DUMMYSPC
	LogMemoryOperation(addr, value, type);
#else

	//Writes always affect the underlying RAM
	if(_state.WriteEnabled) {
		if(_emu->ProcessMemoryWrite<CpuType::Spc>(addr, value, type)) {
			_ram[addr] = value;
		}
	}

	switch(addr) {
		case 0xF0: 
			if(!CheckFlag(SpcFlags::DirectPage)) {
				_state.InternalSpeed = (value >> 6) & 0x03;
				_state.ExternalSpeed = (value >> 4) & 0x03;
				_state.TimersEnabled = (value & 0x08) != 0;
				_state.TimersDisabled = (value & 0x01) != 0;
				_state.WriteEnabled = value & 0x02;

				bool timersEnabled = _state.TimersEnabled && !_state.TimersDisabled;
				_state.Timer0.SetGlobalEnabled(timersEnabled);
				_state.Timer1.SetGlobalEnabled(timersEnabled);
				_state.Timer2.SetGlobalEnabled(timersEnabled);
			}
			break;

		case 0xF1:
			if(value & SpcControlFlags::ClearPortsA) {
				_state.CpuRegs[0] = _state.CpuRegs[1] = 0;
				_state.NewCpuRegs[0] = _state.NewCpuRegs[1] = 0;
			}
			if(value & SpcControlFlags::ClearPortsB) {
				_state.CpuRegs[2] = _state.CpuRegs[3] = 0;
				_state.NewCpuRegs[2] = _state.NewCpuRegs[3] = 0;
			}

			_state.Timer0.SetEnabled((value & SpcControlFlags::Timer0) != 0);
			_state.Timer1.SetEnabled((value & SpcControlFlags::Timer1) != 0);
			_state.Timer2.SetEnabled((value & SpcControlFlags::Timer2) != 0);

			_state.RomEnabled = (value & SpcControlFlags::EnableRom) != 0;
			break;

		case 0xF2: _state.DspReg = value; break;
		case 0xF3: 
			if(_state.DspReg < 128) {
				_dsp->Write(_state.DspReg, value);
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
	if(_state.NewCpuRegs[addr & 0x03] != value) {
		_state.NewCpuRegs[addr & 0x03] = value;

		//If the CPU's write lands in the first half of the SPC cycle (each cycle is 2 clocks) then the SPC 
		//can see the new value immediately, otherwise it only sees the new value on the following cycle.
		//The delay is needed for Kishin Kishin Douji Zenki to boot.
		//However, always delaying to the next SPC cycle causes Kawasaki Superbike Challenge to freeze on boot.
		//Delaying only when the write occurs in the SPC cycle's second half allows both games to work (at the default 32040hz.)
		//This solution behaves as if the CPU values were latched/updated every 2mhz tick (which matches the SPC's input clock)
		if(_memoryManager->GetMasterClock() * _clockRatio - _state.Cycle <= 1) {
			_state.CpuRegs[addr & 0x03] = value;
		} else {
			_pendingCpuRegUpdate = true;
		}
	}
}

uint8_t Spc::DspReadRam(uint16_t addr)
{
	uint8_t value = _ram[addr];
#ifndef DUMMYSPC
	_emu->ProcessMemoryRead<CpuType::Spc, 1, MemoryAccessFlags::DspAccess>(addr, value, MemoryOperationType::Read);
#endif
	return value;
}

void Spc::DspWriteRam(uint16_t addr, uint8_t value)
{
#ifndef DUMMYSPC
	_emu->ProcessMemoryWrite<CpuType::Spc, 1, MemoryAccessFlags::DspAccess>(addr, value, MemoryOperationType::Write);
#endif
	_ram[addr] = value;
}

void Spc::ProcessEndFrame()
{
	Run();

	UpdateClockRatio();

	uint16_t sampleCount = _dsp->GetSampleCount();
	if(sampleCount != 0) {
		_emu->GetSoundMixer()->PlayAudioBuffer(_dsp->GetSamples(), sampleCount / 2, _spcSampleRate);
	}
	_dsp->ResetOutput();
}

SpcState& Spc::GetState()
{
	return _state;
}

DspState& Spc::GetDspState()
{
	return _dsp->GetState();
}

bool Spc::IsMuted()
{
	return _dsp->IsMuted();
}

AddressInfo Spc::GetAbsoluteAddress(uint16_t addr)
{
	if(addr < 0xFFC0 || !_state.RomEnabled) {
		return AddressInfo { addr, MemoryType::SpcRam };
	}
	return AddressInfo { addr & 0x3F, MemoryType::SpcRom };
}

int Spc::GetRelativeAddress(AddressInfo &absAddress)
{
	if(absAddress.Type == MemoryType::SpcRom) {
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
	if(s.IsSaving() && s.GetFormat() != SerializeFormat::Map) {
		//Catch up SPC to main CPU before creating the state
		Run();
	}

	SV(_state.A); SV(_state.Cycle); SV(_state.PC); SV(_state.PS); SV(_state.SP); SV(_state.X); SV(_state.Y);
	SV(_state.CpuRegs[0]); SV(_state.CpuRegs[1]); SV(_state.CpuRegs[2]); SV(_state.CpuRegs[3]);
	SV(_state.OutputReg[0]); SV(_state.OutputReg[1]); SV(_state.OutputReg[2]); SV(_state.OutputReg[3]);
	SV(_state.RamReg[0]); SV(_state.RamReg[1]);
	SV(_state.ExternalSpeed); SV(_state.InternalSpeed); SV(_state.WriteEnabled); SV(_state.TimersEnabled);
	SV(_state.DspReg); SV(_state.RomEnabled); SV(_clockRatio); SV(_state.TimersDisabled);

	s.PushNamePrefix("timer0", -1);
	_state.Timer0.Serialize(s);
	s.PopNamePrefix();

	s.PushNamePrefix("timer1", -1);
	_state.Timer1.Serialize(s);
	s.PopNamePrefix();

	s.PushNamePrefix("timer2", -1);
	_state.Timer2.Serialize(s);
	s.PopNamePrefix();

	SVArray(_ram, Spc::SpcRamSize);

	SV(_dsp);

	if(s.GetFormat() != SerializeFormat::Map) {
		if(!s.IsSaving()) {
			UpdateClockRatio();
		}

		SV(_operandA); SV(_operandB); SV(_tmp1); SV(_tmp2); SV(_tmp3); SV(_opCode); SV(_opStep); SV(_opSubStep); SV(_enabled);

		SVArray(_state.NewCpuRegs, 4);
		SV(_pendingCpuRegUpdate);
	}
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

void Spc::LoadSpcFile(SpcFileData* data)
{
	memcpy(_ram, data->SpcRam, Spc::SpcRamSize);

	if(data->HasExtraRam) {
		bool extraRamContainsIpl = memcmp(data->SpcExtraRam, _spcBios, 0x40) == 0;
		if(!extraRamContainsIpl) {
			bool ramContainsIpl = memcmp(data->SpcRam + 0xFFC0, _spcBios, 0x40) == 0;
			bool isSpcRamEmpty = true;
			bool isExtraRamEmpty = true;
			for(int i = 0; i < 0x40; i++) {
				if(data->SpcExtraRam[i] != 0 && data->SpcExtraRam[i] != 0xFF) {
					isExtraRamEmpty = false;
				}
				if(data->SpcRam[i+0xFFC0] != 0 && data->SpcRam[i + 0xFFC0] != 0xFF) {
					isSpcRamEmpty = false;
				}
			}

			if(ramContainsIpl || (isSpcRamEmpty && !isExtraRamEmpty)) {
				//Use extra ram section only if the main spc FFC0-FFFF section is empty or contains the IPL code
				memcpy(_ram + 0xFFC0, data->SpcExtraRam, 0x40);
			}
		}
	}

	_dsp->LoadSpcFileRegs(data->DspRegs);

	_state.PC = data->PC;
	_state.A = data->A;
	_state.X = data->X;
	_state.Y = data->Y;
	_state.PS = data->PS;
	_state.SP = data->SP;

	Write(0xF1, data->ControlReg);
	_state.DspReg = data->DspRegSelect;

	_state.CpuRegs[0] = data->CpuRegs[0];
	_state.CpuRegs[1] = data->CpuRegs[1];
	_state.CpuRegs[2] = data->CpuRegs[2];
	_state.CpuRegs[3] = data->CpuRegs[3];
	
	_state.RamReg[0] = data->RamRegs[0];
	_state.RamReg[1] = data->RamRegs[1];

	_state.Timer0.SetTarget(data->TimerTarget[0]);
	_state.Timer1.SetTarget(data->TimerTarget[1]);
	_state.Timer2.SetTarget(data->TimerTarget[2]);

	_state.Timer0.SetOutput(data->TimerOutput[0]);
	_state.Timer1.SetOutput(data->TimerOutput[0]);
	_state.Timer2.SetOutput(data->TimerOutput[0]);
}
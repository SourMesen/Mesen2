#include "pch.h"
#include "GBA/GbaCpu.h"
#include "GBA/GbaMemoryManager.h"
#include "GBA/GbaRomPrefetch.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Utilities/Serializer.h"

void GbaCpu::Init(Emulator* emu, GbaMemoryManager* memoryManager, GbaRomPrefetch* prefetch)
{
	_emu = emu;
	_memoryManager = memoryManager;
	_prefetch = prefetch;
	
	_state = {};
	_state.Pipeline.ReloadRequested = true;

	if(_emu->GetSettings()->GetGbaConfig().SkipBootScreen) {
		_state.R[13] = 0x3007F00;
		_state.R[14] = 0x8000000;
		_state.R[15] = 0x8000000;
		_state.UserRegs[4] = 0x300FCA0;
		_state.UserRegs[5] = 0x3007F00;
		_state.UserRegs[6] = 0x00000C0;
		_state.IrqRegs[0] = 0x3007FA0;
		_state.SupervisorRegs[0] = 0x3007FE0;
		_state.CPSR.Mode = GbaCpuMode::System;
	} else {
		_state.CPSR.Mode = GbaCpuMode::Supervisor;
		_state.CPSR.IrqDisable = true;
		_state.CPSR.FiqDisable = true;
	}
}

GbaCpu::~GbaCpu()
{
}

void GbaCpu::StaticInit()
{
	InitArmOpTable();
	InitThumbOpTable();
}

void GbaCpu::SwitchMode(GbaCpuMode mode)
{
	//High bit of mode is always set according to psr test
	mode = (GbaCpuMode)((int)mode | 0x10);

	if(_state.CPSR.Mode == mode) {
		return;
	}

	GbaCpuMode orgMode = _state.CPSR.Mode;
	switch(orgMode) {
		default:
		case GbaCpuMode::System:
		case GbaCpuMode::User:
			memcpy(_state.UserRegs, &_state.R[8], 7 * sizeof(uint32_t));
			break;

		case GbaCpuMode::Fiq:
			memcpy(_state.FiqRegs, &_state.R[8], 7 * sizeof(uint32_t));
			break;
		
		case GbaCpuMode::Irq:
			memcpy(_state.UserRegs, &_state.R[8], 5 * sizeof(uint32_t));
			memcpy(_state.IrqRegs, &_state.R[13], 2 * sizeof(uint32_t));
			break;

		case GbaCpuMode::Supervisor:
			memcpy(_state.UserRegs, &_state.R[8], 5 * sizeof(uint32_t));
			memcpy(_state.SupervisorRegs, &_state.R[13], 2 * sizeof(uint32_t));
			break;

		case GbaCpuMode::Abort:
			memcpy(_state.UserRegs, &_state.R[8], 5 * sizeof(uint32_t));
			memcpy(_state.AbortRegs, &_state.R[13], 2 * sizeof(uint32_t));
			break;

		case GbaCpuMode::Undefined:
			memcpy(_state.UserRegs, &_state.R[8], 5 * sizeof(uint32_t));
			memcpy(_state.UndefinedRegs, &_state.R[13], 2 * sizeof(uint32_t));
			break;
	}

	_state.CPSR.Mode = mode;

	if(mode != GbaCpuMode::Fiq) {
		memcpy(&_state.R[8], _state.UserRegs, 7 * sizeof(uint32_t));
		switch(mode) {
			case GbaCpuMode::Irq: memcpy(&_state.R[13], _state.IrqRegs, 2 * sizeof(uint32_t)); break;
			case GbaCpuMode::Supervisor: memcpy(&_state.R[13], _state.SupervisorRegs, 2 * sizeof(uint32_t)); break;
			case GbaCpuMode::Abort: memcpy(&_state.R[13], _state.AbortRegs, 2 * sizeof(uint32_t)); break;
			case GbaCpuMode::Undefined: memcpy(&_state.R[13], _state.UndefinedRegs, 2 * sizeof(uint32_t)); break;
		}
	} else {
		memcpy(&_state.R[8], _state.FiqRegs, 7 * sizeof(uint32_t));
	}
}

void GbaCpu::ReloadPipeline()
{
	GbaCpuPipeline& pipe = _state.Pipeline;
	pipe.Mode = GbaAccessMode::Prefetch | (_state.CPSR.Thumb ? GbaAccessMode::HalfWord : GbaAccessMode::Word);

	pipe.ReloadRequested = false;
	pipe.Fetch.Address = _state.R[15] = _state.R[15] & (_state.CPSR.Thumb ? ~0x01 : ~0x03);
	_prefetch->ForceNonSequential(_state.R[15]);

	pipe.Fetch.OpCode = ReadCode(pipe.Mode, pipe.Fetch.Address);
	pipe.Execute = pipe.Decode;
	pipe.Decode = pipe.Fetch;

	pipe.Fetch.Address = _state.R[15] = _state.CPSR.Thumb ? (_state.R[15] + 2) : (_state.R[15] + 4);
	pipe.Fetch.OpCode = ReadCode(pipe.Mode, pipe.Fetch.Address);
}

void GbaCpu::CheckForIrqs()
{
	uint32_t originalPc = _state.Pipeline.Execute.Address;
	bool thumb = _state.CPSR.Thumb;
	ProcessException(GbaCpuMode::Irq, GbaCpuVector::Irq);
	if(thumb) {
		_state.R[14] += 2;
	}
	ProcessPipeline();
	_emu->ProcessInterrupt<CpuType::Gba>(originalPc, _state.Pipeline.Execute.Address, false);
}

void GbaCpu::ProcessException(GbaCpuMode mode, GbaCpuVector vector)
{
#ifndef DUMMYCPU
	GbaCpuFlags cpsr = _state.CPSR;
	SwitchMode(mode);
	GetSpsr() = cpsr;
	_state.CPSR.Thumb = false;
	_state.CPSR.IrqDisable = true;
	_state.R[14] = _state.Pipeline.Decode.Address;
	_state.R[15] = (uint32_t)vector;
	_state.Pipeline.ReloadRequested = true;
#endif
}

uint32_t GbaCpu::ReadCode(GbaAccessModeVal mode, uint32_t addr)
{
#ifndef DUMMYCPU
	//Next access should be sequential
	//This is done before the call to Read() because e.g if DMA pauses the CPU and 
	//runs, the next access will not be sequential (force-nseq-access test)
	_state.Pipeline.Mode |= GbaAccessMode::Sequential;
	return _memoryManager->Read(mode, addr);
#else
	uint32_t value = _memoryManager->DebugCpuRead(mode, addr);
	LogMemoryOperation(addr, value, mode, MemoryOperationType::ExecOpCode);
	return value;
#endif
}

uint32_t GbaCpu::Read(GbaAccessModeVal mode, uint32_t addr)
{
#ifndef DUMMYCPU
	_state.Pipeline.Mode &= ~GbaAccessMode::Sequential;
	return _memoryManager->Read(mode, addr);
#else
	uint32_t value = _memoryManager->DebugCpuRead(mode, addr);
	LogMemoryOperation(addr, value, mode, MemoryOperationType::Read);
	return value;
#endif
}

void GbaCpu::Write(GbaAccessModeVal mode, uint32_t addr, uint32_t value)
{
#ifndef DUMMYCPU
	_state.Pipeline.Mode &= ~GbaAccessMode::Sequential;
	_memoryManager->Write(mode, addr, value);
#else
	LogMemoryOperation(addr, value, mode, MemoryOperationType::Write);
#endif
}

void GbaCpu::Idle()
{
#ifndef DUMMYCPU
	_state.Pipeline.Mode &= ~GbaAccessMode::Sequential;
	_memoryManager->ProcessIdleCycle();
#endif
}

void GbaCpu::Idle(uint8_t cycleCount)
{
	switch(cycleCount) {
		case 4: Idle(); [[fallthrough]];
		case 3: Idle(); [[fallthrough]];
		case 2: Idle(); [[fallthrough]];
		case 1: Idle(); break;
	}
}

uint32_t GbaCpu::R(uint8_t reg)
{
	return _state.R[reg];
}

GbaCpuFlags& GbaCpu::GetSpsr()
{
	switch(_state.CPSR.Mode) {
		default:
		case GbaCpuMode::User: return _state.CPSR;
		case GbaCpuMode::Fiq: return _state.FiqSpsr;
		case GbaCpuMode::Irq: return _state.IrqSpsr;
		case GbaCpuMode::Supervisor: return _state.SupervisorSpsr;
		case GbaCpuMode::Abort: return _state.AbortSpsr;
		case GbaCpuMode::Undefined: return _state.UndefinedSpsr;
		case GbaCpuMode::System: return _state.CPSR;
	}
}

uint32_t GbaCpu::Add(uint32_t op1, uint32_t op2, bool carry, bool updateFlags)
{
	uint32_t result = op1 + op2 + (uint32_t)carry;
	if(updateFlags) {
		uint32_t overflow = ~(op1 ^ op2) & (op1 ^ result) & (1 << 31);
		_state.CPSR.Negative = result & (1 << 31);
		_state.CPSR.Zero = (uint32_t)result == 0;
		_state.CPSR.Overflow = overflow;
		_state.CPSR.Carry = (op1 ^ op2 ^ overflow ^ result) & (1 << 31);
	}
	return (uint32_t)result;
}

uint32_t GbaCpu::Sub(uint32_t op1, uint32_t op2, bool carry, bool updateFlags)
{
	return Add(op1, ~op2, carry, updateFlags);
}

uint32_t GbaCpu::LogicalOp(uint32_t result, bool carry, bool updateFlags)
{
	//"If the S bit is set(and Rd is not R15, see below) the V flag in the CPSR will be unaffected, the C
	//flag will be set to the carry out from the barrel shifter (or preserved when the shift
	//operation is LSL #0), the Z flag will be set if and only if the result is all zeros, and the N
	//flag will be set to the logical value of bit 31 of the result."
	if(updateFlags) {
		_state.CPSR.Carry = carry;
		_state.CPSR.Zero = result == 0;
		_state.CPSR.Negative = result & (1 << 31);
	}
	return result;
}

uint32_t GbaCpu::RotateRight(uint32_t value, uint32_t shift)
{
	return (value >> shift) | (value << (32 - shift));
}

uint32_t GbaCpu::RotateRight(uint32_t value, uint32_t shift, bool& carry)
{
	carry = (value >> (shift - 1)) & 1;
	return (value >> shift) | (value << (32 - shift));
}

uint32_t GbaCpu::ShiftLsl(uint32_t value, uint8_t shift, bool& carry)
{
	if(shift) {
		carry = shift < 33 ? (value & (1 << (32 - shift))) : 0;
		value = shift < 32 ? (value << shift) : 0;
	}
	return value;
}

uint32_t GbaCpu::ShiftLsr(uint32_t value, uint8_t shift, bool& carry)
{
	if(shift) {
		carry = shift < 33 ? (value & (1 << (shift - 1))) : 0;
		value = shift < 32 ? (value >> shift) : 0;
	}
	return value;
}

uint32_t GbaCpu::ShiftAsr(uint32_t value, uint8_t shift, bool& carry)
{
	if(shift) {
		carry = shift < 33 ? (value & (1 << (shift - 1))) : (value & (1 << 31));
		value = shift < 32 ? ((int32_t)value >> shift) : ((int32_t)value >> 31);
	}
	return value;
}

uint32_t GbaCpu::ShiftRor(uint32_t value, uint8_t shift, bool& carry)
{
	if(shift) {
		shift &= 0x1F;
		if(shift) {
			value = (value << (32 - shift)) | (value >> shift);
		}
		carry = value & (1 << 31);
	}
	return value;
}

uint32_t GbaCpu::ShiftRrx(uint32_t value, bool& carry)
{
	bool orgCarry = carry;
	carry = value & 0x01;
	return (value >> 1) | (orgCarry << 31);
}

void GbaCpu::PowerOn()
{
	ProcessPipeline();
}

void GbaCpu::SetProgramCounter(uint32_t addr, bool thumb)
{
	//Used by debugger - set new PC and reload pipeline (using debugger reads)
	_state.R[15] = addr;
	_state.CPSR.Thumb = thumb;

	GbaCpuPipeline& pipe = _state.Pipeline;
	pipe.Mode = GbaAccessMode::Prefetch | (_state.CPSR.Thumb ? GbaAccessMode::HalfWord : GbaAccessMode::Word) | GbaAccessMode::Sequential;
	pipe.ReloadRequested = false;
	pipe.Execute.Address = _state.R[15] = _state.R[15] & (_state.CPSR.Thumb ? ~0x01 : ~0x03);
	pipe.Execute.OpCode = _memoryManager->DebugCpuRead(pipe.Mode, pipe.Execute.Address);
	pipe.Decode.Address = _state.R[15] = _state.CPSR.Thumb ? (_state.R[15] + 2) : (_state.R[15] + 4);
	pipe.Decode.OpCode = _memoryManager->DebugCpuRead(pipe.Mode, pipe.Decode.Address);
	pipe.Fetch.Address = _state.R[15] = _state.CPSR.Thumb ? (_state.R[15] + 2) : (_state.R[15] + 4);
	pipe.Fetch.OpCode = _memoryManager->DebugCpuRead(pipe.Mode, pipe.Fetch.Address);
}

GbaCpuState& GbaCpu::GetState()
{
	_state.CycleCount = _memoryManager->GetMasterClock();
	return _state;
}

void GbaCpu::Serialize(Serializer& s)
{
	SV(_state.Pipeline.Fetch.Address);
	SV(_state.Pipeline.Fetch.OpCode);
	SV(_state.Pipeline.Decode.Address);
	SV(_state.Pipeline.Decode.OpCode);
	SV(_state.Pipeline.Execute.Address);
	SV(_state.Pipeline.Execute.OpCode);
	SV(_state.Pipeline.ReloadRequested);
	SV(_state.Pipeline.Mode);

	SV(_state.CPSR.Mode);
	SV(_state.CPSR.Thumb);
	SV(_state.CPSR.FiqDisable);
	SV(_state.CPSR.IrqDisable);
	SV(_state.CPSR.Overflow);
	SV(_state.CPSR.Carry);
	SV(_state.CPSR.Zero);
	SV(_state.CPSR.Negative);

	SV(_state.Stopped);
	SVArray(_state.R, 16);
	SVArray(_state.UserRegs, 7);
	SVArray(_state.FiqRegs, 7);
	SVArray(_state.IrqRegs, 2);
	SVArray(_state.SupervisorRegs, 2);
	SVArray(_state.AbortRegs, 2);
	SVArray(_state.UndefinedRegs, 2);

	SV(_state.FiqSpsr.Mode);
	SV(_state.FiqSpsr.Thumb);
	SV(_state.FiqSpsr.FiqDisable);
	SV(_state.FiqSpsr.IrqDisable);
	SV(_state.FiqSpsr.Overflow);
	SV(_state.FiqSpsr.Carry);
	SV(_state.FiqSpsr.Zero);
	SV(_state.FiqSpsr.Negative);

	SV(_state.IrqSpsr.Mode);
	SV(_state.IrqSpsr.Thumb);
	SV(_state.IrqSpsr.FiqDisable);
	SV(_state.IrqSpsr.IrqDisable);
	SV(_state.IrqSpsr.Overflow);
	SV(_state.IrqSpsr.Carry);
	SV(_state.IrqSpsr.Zero);
	SV(_state.IrqSpsr.Negative);

	SV(_state.SupervisorSpsr.Mode);
	SV(_state.SupervisorSpsr.Thumb);
	SV(_state.SupervisorSpsr.FiqDisable);
	SV(_state.SupervisorSpsr.IrqDisable);
	SV(_state.SupervisorSpsr.Overflow);
	SV(_state.SupervisorSpsr.Carry);
	SV(_state.SupervisorSpsr.Zero);
	SV(_state.SupervisorSpsr.Negative);

	SV(_state.AbortSpsr.Mode);
	SV(_state.AbortSpsr.Thumb);
	SV(_state.AbortSpsr.FiqDisable);
	SV(_state.AbortSpsr.IrqDisable);
	SV(_state.AbortSpsr.Overflow);
	SV(_state.AbortSpsr.Carry);
	SV(_state.AbortSpsr.Zero);
	SV(_state.AbortSpsr.Negative);

	SV(_state.UndefinedSpsr.Mode);
	SV(_state.UndefinedSpsr.Thumb);
	SV(_state.UndefinedSpsr.FiqDisable);
	SV(_state.UndefinedSpsr.IrqDisable);
	SV(_state.UndefinedSpsr.Overflow);
	SV(_state.UndefinedSpsr.Carry);
	SV(_state.UndefinedSpsr.Zero);
	SV(_state.UndefinedSpsr.Negative);

	SV(_state.CycleCount);
}

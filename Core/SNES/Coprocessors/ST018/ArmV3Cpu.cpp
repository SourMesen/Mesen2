#include "pch.h"
#include "SNES/Coprocessors/ST018/ArmV3Cpu.h"
#include "SNES/Coprocessors/ST018/St018.h"
#include "GBA/GbaCpuMultiply.h"
#include "Shared/ArmEnums.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Utilities/Serializer.h"

ArmOpCategory ArmV3Cpu::_armCategory[0x1000];
ArmV3Cpu::Func ArmV3Cpu::_armTable[0x1000];

void ArmV3Cpu::Init(Emulator* emu, St018* st018)
{
	_emu = emu;
	_st018 = st018;
}

ArmV3Cpu::~ArmV3Cpu()
{
}

void ArmV3Cpu::StaticInit()
{
	InitArmOpTable();
}

void ArmV3Cpu::SwitchMode(ArmV3CpuMode mode)
{
	//High bit of mode is always set according to psr test
	mode = (ArmV3CpuMode)((int)mode | 0x10);

	if(_state.CPSR.Mode == mode) {
		return;
	}

	ArmV3CpuMode orgMode = _state.CPSR.Mode;
	switch(orgMode) {
		default:
		case ArmV3CpuMode::System:
		case ArmV3CpuMode::User:
			memcpy(_state.UserRegs, &_state.R[8], 7 * sizeof(uint32_t));
			break;

		case ArmV3CpuMode::Fiq:
			memcpy(_state.FiqRegs, &_state.R[8], 7 * sizeof(uint32_t));
			break;

		case ArmV3CpuMode::Irq:
			memcpy(_state.UserRegs, &_state.R[8], 5 * sizeof(uint32_t));
			memcpy(_state.IrqRegs, &_state.R[13], 2 * sizeof(uint32_t));
			break;

		case ArmV3CpuMode::Supervisor:
			memcpy(_state.UserRegs, &_state.R[8], 5 * sizeof(uint32_t));
			memcpy(_state.SupervisorRegs, &_state.R[13], 2 * sizeof(uint32_t));
			break;

		case ArmV3CpuMode::Abort:
			memcpy(_state.UserRegs, &_state.R[8], 5 * sizeof(uint32_t));
			memcpy(_state.AbortRegs, &_state.R[13], 2 * sizeof(uint32_t));
			break;

		case ArmV3CpuMode::Undefined:
			memcpy(_state.UserRegs, &_state.R[8], 5 * sizeof(uint32_t));
			memcpy(_state.UndefinedRegs, &_state.R[13], 2 * sizeof(uint32_t));
			break;
	}

	_state.CPSR.Mode = mode;

	if(mode != ArmV3CpuMode::Fiq) {
		memcpy(&_state.R[8], _state.UserRegs, 7 * sizeof(uint32_t));
		switch(mode) {
			case ArmV3CpuMode::Irq: memcpy(&_state.R[13], _state.IrqRegs, 2 * sizeof(uint32_t)); break;
			case ArmV3CpuMode::Supervisor: memcpy(&_state.R[13], _state.SupervisorRegs, 2 * sizeof(uint32_t)); break;
			case ArmV3CpuMode::Abort: memcpy(&_state.R[13], _state.AbortRegs, 2 * sizeof(uint32_t)); break;
			case ArmV3CpuMode::Undefined: memcpy(&_state.R[13], _state.UndefinedRegs, 2 * sizeof(uint32_t)); break;
		}
	} else {
		memcpy(&_state.R[8], _state.FiqRegs, 7 * sizeof(uint32_t));
	}
}

void ArmV3Cpu::ReloadPipeline()
{
	ArmV3CpuPipeline& pipe = _state.Pipeline;
	pipe.Mode = ArmV3AccessMode::Prefetch | ArmV3AccessMode::Word;

	pipe.ReloadRequested = false;
	pipe.Fetch.Address = _state.R[15] = _state.R[15] & ~0x03;

	pipe.Fetch.OpCode = ReadCode(pipe.Mode, pipe.Fetch.Address);
	pipe.Execute = pipe.Decode;
	pipe.Decode = pipe.Fetch;

	pipe.Fetch.Address = _state.R[15] = (_state.R[15] + 4);
	pipe.Fetch.OpCode = ReadCode(pipe.Mode, pipe.Fetch.Address);
}

void ArmV3Cpu::ProcessException(ArmV3CpuMode mode, ArmV3CpuVector vector)
{
#ifndef DUMMYCPU
	ArmV3CpuFlags cpsr = _state.CPSR;
	SwitchMode(mode);
	GetSpsr() = cpsr;
	_state.CPSR.IrqDisable = true;
	_state.R[14] = _state.Pipeline.Decode.Address;
	_state.R[15] = (uint32_t)vector;
	_state.Pipeline.ReloadRequested = true;
#endif
}

uint32_t ArmV3Cpu::ReadCode(ArmV3AccessModeVal mode, uint32_t addr)
{
#ifndef DUMMYCPU
	//Next access should be sequential
	//This is done before the call to Read() because e.g if DMA pauses the CPU and 
	//runs, the next access will not be sequential (force-nseq-access test)
	_state.Pipeline.Mode |= ArmV3AccessMode::Sequential;
	return _st018->ReadCpu(mode, addr);
#else
	uint32_t value = _st018->DebugCpuRead(mode, addr);
	LogMemoryOperation(addr, value, mode, MemoryOperationType::ExecOpCode);
	return value;
#endif
}

uint32_t ArmV3Cpu::Read(ArmV3AccessModeVal mode, uint32_t addr)
{
#ifndef DUMMYCPU
	_state.Pipeline.Mode &= ~ArmV3AccessMode::Sequential;
	return _st018->ReadCpu(mode, addr);
#else
	uint32_t value = _st018->DebugCpuRead(mode, addr);
	LogMemoryOperation(addr, value, mode, MemoryOperationType::Read);
	return value;
#endif
}

void ArmV3Cpu::Write(ArmV3AccessModeVal mode, uint32_t addr, uint32_t value)
{
#ifndef DUMMYCPU
	_state.Pipeline.Mode &= ~ArmV3AccessMode::Sequential;
	_st018->WriteCpu(mode, addr, value);
#else
	LogMemoryOperation(addr, value, mode, MemoryOperationType::Write);
#endif
}

void ArmV3Cpu::Idle()
{
#ifndef DUMMYCPU
	_state.Pipeline.Mode &= ~ArmV3AccessMode::Sequential;
	_st018->ProcessIdleCycle();
#endif
}

void ArmV3Cpu::Idle(uint8_t cycleCount)
{
	switch(cycleCount) {
		case 4: Idle(); [[fallthrough]];
		case 3: Idle(); [[fallthrough]];
		case 2: Idle(); [[fallthrough]];
		case 1: Idle(); break;
	}
}

uint32_t ArmV3Cpu::R(uint8_t reg)
{
	return _state.R[reg];
}

ArmV3CpuFlags& ArmV3Cpu::GetSpsr()
{
	switch(_state.CPSR.Mode) {
		default:
		case ArmV3CpuMode::User: return _state.CPSR;
		case ArmV3CpuMode::Fiq: return _state.FiqSpsr;
		case ArmV3CpuMode::Irq: return _state.IrqSpsr;
		case ArmV3CpuMode::Supervisor: return _state.SupervisorSpsr;
		case ArmV3CpuMode::Abort: return _state.AbortSpsr;
		case ArmV3CpuMode::Undefined: return _state.UndefinedSpsr;
		case ArmV3CpuMode::System: return _state.CPSR;
	}
}

uint32_t ArmV3Cpu::Add(uint32_t op1, uint32_t op2, bool carry, bool updateFlags)
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

uint32_t ArmV3Cpu::Sub(uint32_t op1, uint32_t op2, bool carry, bool updateFlags)
{
	return Add(op1, ~op2, carry, updateFlags);
}

uint32_t ArmV3Cpu::LogicalOp(uint32_t result, bool carry, bool updateFlags)
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

uint32_t ArmV3Cpu::RotateRight(uint32_t value, uint32_t shift)
{
	return (value >> shift) | (value << (32 - shift));
}

uint32_t ArmV3Cpu::RotateRight(uint32_t value, uint32_t shift, bool& carry)
{
	carry = (value >> (shift - 1)) & 1;
	return (value >> shift) | (value << (32 - shift));
}

uint32_t ArmV3Cpu::ShiftLsl(uint32_t value, uint8_t shift, bool& carry)
{
	if(shift) {
		carry = shift < 33 ? (value & (1 << (32 - shift))) : 0;
		value = shift < 32 ? (value << shift) : 0;
	}
	return value;
}

uint32_t ArmV3Cpu::ShiftLsr(uint32_t value, uint8_t shift, bool& carry)
{
	if(shift) {
		carry = shift < 33 ? (value & (1 << (shift - 1))) : 0;
		value = shift < 32 ? (value >> shift) : 0;
	}
	return value;
}

uint32_t ArmV3Cpu::ShiftAsr(uint32_t value, uint8_t shift, bool& carry)
{
	if(shift) {
		carry = shift < 33 ? (value & (1 << (shift - 1))) : (value & (1 << 31));
		value = shift < 32 ? ((int32_t)value >> shift) : ((int32_t)value >> 31);
	}
	return value;
}

uint32_t ArmV3Cpu::ShiftRor(uint32_t value, uint8_t shift, bool& carry)
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

uint32_t ArmV3Cpu::ShiftRrx(uint32_t value, bool& carry)
{
	bool orgCarry = carry;
	carry = value & 0x01;
	return (value >> 1) | (orgCarry << 31);
}

void ArmV3Cpu::PowerOn(bool forReset)
{
	uint64_t cycleCount = _state.CycleCount;
	_state = {};
	_state.Pipeline.ReloadRequested = true;

	_state.CPSR.Mode = ArmV3CpuMode::Supervisor;
	_state.CPSR.IrqDisable = true;
	_state.CPSR.FiqDisable = true;

	ProcessPipeline();

	if(forReset) {
		_state.CycleCount = cycleCount;
	}
}

void ArmV3Cpu::SetProgramCounter(uint32_t addr)
{
	//Used by debugger - set new PC and reload pipeline (using debugger reads)
	_state.R[15] = addr;

	ArmV3CpuPipeline& pipe = _state.Pipeline;
	pipe.Mode = ArmV3AccessMode::Prefetch | ArmV3AccessMode::Word | ArmV3AccessMode::Sequential;
	pipe.ReloadRequested = false;
	pipe.Execute.Address = _state.R[15] = _state.R[15] & ~0x03;
	pipe.Execute.OpCode = _st018->DebugCpuRead(pipe.Mode, pipe.Execute.Address);
	pipe.Decode.Address = _state.R[15] = _state.R[15] + 4;
	pipe.Decode.OpCode = _st018->DebugCpuRead(pipe.Mode, pipe.Decode.Address);
	pipe.Fetch.Address = _state.R[15] = _state.R[15] + 4;
	pipe.Fetch.OpCode = _st018->DebugCpuRead(pipe.Mode, pipe.Fetch.Address);
}

ArmV3CpuState& ArmV3Cpu::GetState()
{
	return _state;
}

ArmOpCategory ArmV3Cpu::GetArmOpCategory(uint32_t opCode)
{
	uint16_t opType = ((opCode & 0x0FF00000) >> 16) | ((opCode & 0xF0) >> 4);
	return _armCategory[opType];
}

void ArmV3Cpu::ArmBranch()
{
	bool withLink = (_opCode & (1 << 24)) != 0;
	int32_t offset = (((int32_t)_opCode << 8) >> 6); //sign extend + shift right by 2
	if(withLink) {
		_state.R[14] = _state.R[15] - 4;
	}

	_state.R[15] += offset;
	_state.Pipeline.ReloadRequested = true;
}

void ArmV3Cpu::ArmMsr()
{
	//----_00i1_0d10_mmmm_1111_oooo_oooo_oooo
	//i: immediate
	//d: destination psr
	//o: operand
	//m: mask
	bool immediate = _opCode & (1 << 25);
	bool writeToSpsr = _opCode & (1 << 22);
	uint8_t mask = (_opCode >> 16) & 0x0F;

	if(writeToSpsr && (_state.CPSR.Mode == ArmV3CpuMode::User || _state.CPSR.Mode == ArmV3CpuMode::System)) {
		return;
	}

	uint32_t value;
	if(immediate) {
		value = _opCode & 0xFF;
		uint8_t shift = (_opCode >> 8) & 0x0F;
		if(shift) {
			value = RotateRight(value, shift * 2);
		}
	} else {
		value = R(_opCode & 0x0F);
	}

	ArmV3CpuFlags& flags = writeToSpsr ? GetSpsr() : _state.CPSR;
	if(mask & 0x08) {
		flags.Negative = value & (1 << 31);
		flags.Zero = value & (1 << 30);
		flags.Carry = value & (1 << 29);
		flags.Overflow = value & (1 << 28);
	}

	if(mask & 0x01) {
		if(writeToSpsr || _state.CPSR.Mode != ArmV3CpuMode::User) {
			if(!writeToSpsr) {
				SwitchMode((ArmV3CpuMode)(value & 0x1F));
			} else {
				flags.Mode = (ArmV3CpuMode)(value & 0x1F);
			}
			flags.FiqDisable = value & (1 << 6);
			flags.IrqDisable = value & (1 << 7);
		}
	}
}

void ArmV3Cpu::ArmMrs()
{
	//----_0001_0s00_1111_dddd_0000_0000_0000
	//s: source psr
	//d: destination reg
	bool useSpsr = _opCode & (1 << 22);
	uint8_t rd = (_opCode >> 12) & 0x0F;
	SetR(rd, useSpsr ? GetSpsr().ToInt32() : _state.CPSR.ToInt32());
}

void ArmV3Cpu::ArmDataProcessing()
{
	//Data Processing
	//----_00io_ooos_nnnn_dddd_pppp_pppp_pppp
	//i: immediate
	//o: opcode
	//s: set flags
	//n: 1st operand reg
	//d: destination reg
	//p: 2nd operand

	bool immediate = (_opCode & (1 << 25)) != 0;
	uint8_t rn = (_opCode >> 16) & 0x0F;
	uint32_t op1 = R(rn);
	uint8_t dstReg = (_opCode >> 12) & 0x0F;
	bool updateFlags = (_opCode & (1 << 20)) != 0;

	uint32_t op2;
	bool carry = _state.CPSR.Carry;
	if(immediate) {
		uint8_t shift = (_opCode >> 8) & 0x0F;
		op2 = (_opCode & 0xFF);
		if(shift) {
			op2 = RotateRight(op2, shift * 2, carry);
		}
	} else {
		uint8_t shiftType = (_opCode >> 5) & 0x03;
		uint8_t shift;
		uint8_t rm = _opCode & 0x0F;
		op2 = R(rm);

		bool useRegValue = _opCode & (1 << 4);
		if(useRegValue) {
			//Shift amount in register
			Idle();
			uint8_t rs = (_opCode >> 8) & 0x0F;
			shift = R(rs) + (rs == 15 ? 4 : 0);
			if(rm == 15) {
				op2 += 4;
			}
			if(rn == 15) {
				op1 += 4;
			}
		} else {
			shift = (_opCode >> 7) & 0x1F;
		}

		switch(shiftType) {
			case 0: op2 = ShiftLsl(op2, shift, carry); break;
			case 1: op2 = ShiftLsr(op2, (useRegValue || shift) ? shift : 32, carry); break;
			case 2: op2 = ShiftAsr(op2, (useRegValue || shift) ? shift : 32, carry); break;
			case 3: op2 = (!useRegValue && shift == 0) ? ShiftRrx(op2, carry) : ShiftRor(op2, shift, carry); break;
		}
	}

	switch((ArmAluOperation)((_opCode >> 21) & 0x0F)) {
		case ArmAluOperation::And: SetR(dstReg, LogicalOp(op1 & op2, carry, updateFlags)); break;
		case ArmAluOperation::Eor: SetR(dstReg, LogicalOp(op1 ^ op2, carry, updateFlags)); break;
		case ArmAluOperation::Sub: SetR(dstReg, Sub(op1, op2, true, updateFlags)); break;
		case ArmAluOperation::Rsb: SetR(dstReg, Sub(op2, op1, true, updateFlags)); break;
		case ArmAluOperation::Add: SetR(dstReg, Add(op1, op2, false, updateFlags)); break;
		case ArmAluOperation::Adc: SetR(dstReg, Add(op1, op2, _state.CPSR.Carry, updateFlags)); break;
		case ArmAluOperation::Sbc: SetR(dstReg, Sub(op1, op2, _state.CPSR.Carry, updateFlags)); break;
		case ArmAluOperation::Rsc: SetR(dstReg, Sub(op2, op1, _state.CPSR.Carry, updateFlags)); break;
		case ArmAluOperation::Tst: LogicalOp(op1 & op2, carry, true); break;
		case ArmAluOperation::Teq: LogicalOp(op1 ^ op2, carry, true); break;
		case ArmAluOperation::Cmp: Sub(op1, op2, true, true); break;
		case ArmAluOperation::Cmn: Add(op1, op2, false, true); break;
		case ArmAluOperation::Orr: SetR(dstReg, LogicalOp(op1 | op2, carry, updateFlags)); break;
		case ArmAluOperation::Mov: SetR(dstReg, LogicalOp(op2, carry, updateFlags)); break;
		case ArmAluOperation::Bic: SetR(dstReg, LogicalOp(op1 & ~op2, carry, updateFlags)); break;
		case ArmAluOperation::Mvn: SetR(dstReg, LogicalOp(~op2, carry, updateFlags)); break;
	}

	if(dstReg == 15 && updateFlags) {
		ArmV3CpuFlags spsr = GetSpsr();
		SwitchMode(spsr.Mode);
		_state.CPSR = spsr;
	}
}

void ArmV3Cpu::ArmMultiply()
{
	//Multiply and Multiply-Accumulate (MUL, MLA)
	//----_0000_00as_dddd_nnnn_ssss_1001_mmmm
	uint8_t rd = (_opCode >> 16) & 0x0F;
	uint8_t rn = (_opCode >> 12) & 0x0F;
	uint8_t rs = (_opCode >> 8) & 0x0F;
	uint8_t rm = _opCode & 0x0F;
	bool updateFlags = (_opCode & (1 << 20)) != 0;
	bool multAndAcc = (_opCode & (1 << 21)) != 0;

	MultiplicationOutput output;
	if(multAndAcc) {
		output = GbaCpuMultiply::mla(R(rm), R(rs), R(rn));
	} else {
		output = GbaCpuMultiply::mul(R(rm), R(rs));
	}

	Idle(output.CycleCount);
	if(multAndAcc) {
		Idle();
	}

	uint32_t result = output.Output;
	SetR(rd, result);

	if(updateFlags) {
		_state.CPSR.Carry = output.Carry;
		_state.CPSR.Zero = result == 0;
		_state.CPSR.Negative = (result & (1 << 31));
	}
}

void ArmV3Cpu::ArmMultiplyLong()
{
	//Multiply Long and Multiply-Accumulate Long (MULL,MLAL)
	//----_0000_1uas_hhhh_llll_ssss_1001_mmmm
	uint8_t rh = (_opCode >> 16) & 0x0F;
	uint8_t rl = (_opCode >> 12) & 0x0F;
	uint8_t rs = (_opCode >> 8) & 0x0F;
	uint8_t rm = _opCode & 0x0F;

	bool updateFlags = (_opCode & (1 << 20)) != 0;
	bool multAndAcc = (_opCode & (1 << 21)) != 0;
	bool sign = (_opCode & (1 << 22)) != 0;

	Idle();

	MultiplicationOutput output;
	if(sign) {
		if(multAndAcc) {
			output = GbaCpuMultiply::smlal(R(rl), R(rh), R(rm), R(rs));
		} else {
			output = GbaCpuMultiply::smull(R(rm), R(rs));
		}
	} else {
		if(multAndAcc) {
			output = GbaCpuMultiply::umlal(R(rl), R(rh), R(rm), R(rs));
		} else {
			output = GbaCpuMultiply::umull(R(rm), R(rs));
		}
	}

	Idle(output.CycleCount);
	if(multAndAcc) {
		Idle();
	}

	uint64_t result = output.Output;

	SetR(rl, (uint32_t)result);
	SetR(rh, (uint32_t)(result >> 32));

	if(updateFlags) {
		_state.CPSR.Carry = output.Carry;
		_state.CPSR.Zero = result == 0;
		_state.CPSR.Negative = (result & ((uint64_t)1 << 63));
	}
}

void ArmV3Cpu::ArmSingleDataTransfer()
{
	//Single Data Transfer (LDR, STR)
	//----_01ip_ubwl_nnnn_dddd_oooo_oooo_oooo
	//i: immediate (when cleared)
	//p: post/pre: add offset after/before transfer
	//u: down/up: subtract/add offset from/to base
	//b: byte transfer
	//w: write-back: write address into base
	//l: store/load
	//n: base register
	//d: src/dst register
	//o: offset (immediate or register)
	bool immediate = (_opCode & (1 << 25)) == 0;
	bool pre = (_opCode & (1 << 24)) != 0;
	bool up = (_opCode & (1 << 23)) != 0;
	bool byte = (_opCode & (1 << 22)) != 0;
	bool writeBack = (_opCode & (1 << 21)) != 0;
	bool load = (_opCode & (1 << 20)) != 0;
	uint8_t rn = (_opCode >> 16) & 0x0F;
	uint8_t rd = (_opCode >> 12) & 0x0F;

	uint32_t addr = R(rn);

	int32_t offset;
	if(immediate) {
		offset = _opCode & 0xFFF;
	} else {
		uint8_t shiftType = (_opCode >> 5) & 0x03;
		uint8_t shift;
		uint8_t rm = _opCode & 0x0F;
		shift = (_opCode >> 7) & 0x1F;

		offset = R(rm);
		bool carry = _state.CPSR.Carry;
		switch(shiftType) {
			case 0: offset = ShiftLsl(offset, shift, carry); break;
			case 1: offset = ShiftLsr(offset, shift ? shift : 32, carry); break;
			case 2: offset = ShiftAsr(offset, shift ? shift : 32, carry); break;
			case 3: offset = shift == 0 ? ShiftRrx(offset, carry) : ShiftRor(offset, shift, carry); break;
		}
	}

	if(pre) {
		addr += up ? offset : -offset;
	}

	ArmV3AccessModeVal mode = byte ? ArmV3AccessMode::Byte : ArmV3AccessMode::Word;
	if(load) {
		SetR(rd, Read(mode, addr));
		Idle();
	} else {
		Write(mode, addr, R(rd) + (rd == 15 ? 4 : 0));
	}

	if(!pre) {
		addr += up ? offset : -offset;
	}

	if((rd != rn || !load) && (writeBack || !pre)) {
		SetR(rn, addr);
	}
}

void ArmV3Cpu::ArmBlockDataTransfer()
{
	//Block Data Transfer (LDM, STM)
	//----_100p_uswl_nnnn_rrrr_rrrr_rrrr_rrrr
	//p: post/pre: add offset after/before transfer
	//u: down/up: subtract/add offset from/to base
	//s: psr & force user bit
	//w: write-back: write address into base
	//l: store/load
	//n: base register
	//r: register list
	bool pre = (_opCode & (1 << 24)) != 0;
	bool up = (_opCode & (1 << 23)) != 0;
	bool psrForceUser = (_opCode & (1 << 22)) != 0;
	bool writeBack = (_opCode & (1 << 21)) != 0;
	bool load = (_opCode & (1 << 20)) != 0;
	uint8_t rn = (_opCode >> 16) & 0x0F;
	uint16_t regMask = (uint16_t)_opCode;

	uint32_t base = R(rn) + (rn == 15 ? 4 : 0);
	uint32_t addr = base;

	uint8_t regCount = 0;
	for(int i = 0; i < 16; i++) {
		regCount += (regMask & (1 << i)) ? 1 : 0;
	}

	if(!regMask) {
		//Glitch when mask is empty - only R15 is stored/loaded, but address changes as if all 16 were written/loaded
		regCount = 16;
		regMask = 0x8000;
	}

	if(!up) {
		addr -= (regCount - (pre ? 0 : 1)) * 4;
	} else if(pre) {
		addr += 4;
	}

	if(writeBack && load) {
		SetR(rn, base + (regCount * (up ? 4 : -4)));
	}

	ArmV3CpuMode orgMode = _state.CPSR.Mode;
	if(psrForceUser && (!load || (load && !(regMask & 0x8000)))) {
		SwitchMode(ArmV3CpuMode::User);
	}

	bool firstReg = true;
	ArmV3AccessModeVal mode = ArmV3AccessMode::Word;
	for(int i = 0; i < 16; i++) {
		if(regMask & (1 << i)) {
			if(!load) {
				Write(mode, addr, R(i) + (i == 15 ? 4 : 0));
			}

			if(firstReg && writeBack) {
				//Write-back happens at this point, based on the gba-tests/arm test 522
				SetR(rn, base + (regCount * (up ? 4 : -4)));
				firstReg = false;
			}

			if(load) {
				//LDM doesn't appear to be affected by the rotation that is usually applied to unaligned reads? based on gba-tests/arm test 508
				SetR(i, Read(mode | ArmV3AccessMode::NoRotate, addr));
			}

			mode |= ArmV3AccessMode::Sequential;
			addr += 4;
		}
	}

	if(load) {
		Idle();
	}

	SwitchMode(orgMode);

	if(psrForceUser && load && (regMask & 0x8000)) {
		ArmV3CpuFlags spsr = GetSpsr();
		SwitchMode(spsr.Mode);
		_state.CPSR = spsr;
	}
}

void ArmV3Cpu::ArmSingleDataSwap()
{
	//Single Data Swap (SWP)
	//----_0001_0b00_nnnn_dddd_0000_1001_mmmm
	bool byte = _opCode & (1 << 22);
	uint8_t rn = (_opCode >> 16) & 0x0F;
	uint8_t rd = (_opCode >> 12) & 0x0F;
	uint8_t rm = _opCode & 0x0F;

	uint32_t mode = byte ? ArmV3AccessMode::Byte : ArmV3AccessMode::Word;
	uint32_t val = Read(mode, R(rn));
	Idle();
	Write(mode, R(rn), R(rm));
	SetR(rd, val);
}

void ArmV3Cpu::ArmSoftwareInterrupt()
{
	ProcessException(ArmV3CpuMode::Supervisor, ArmV3CpuVector::SoftwareIrq);
}

void ArmV3Cpu::ArmInvalidOp()
{
#ifndef DUMMYCPU
	ProcessException(ArmV3CpuMode::Undefined, ArmV3CpuVector::Undefined);
#endif
}

bool ArmV3Cpu::CheckConditions(uint32_t condCode)
{
	/*Code Suffix Flags Meaning
		0000 EQ Z set equal
		0001 NE Z clear not equal
		0010 CS C set unsigned higher or same
		0011 CC C clear unsigned lower
		0100 MI N set negative
		0101 PL N clear positive or zero
		0110 VS V set overflow
		0111 VC V clear no overflow
		1000 HI C set and Z clear unsigned higher
		1001 LS C clear or Z set unsigned lower or same
		1010 GE N equals V greater or equal
		1011 LT N not equal to V less than
		1100 GT Z clear AND(N equals V) greater than
		1101 LE Z set OR(N not equal to V) less than or equal
		1110 AL(ignored) always
	*/
	switch(condCode) {
		case 0: return _state.CPSR.Zero;
		case 1: return !_state.CPSR.Zero;
		case 2: return _state.CPSR.Carry;
		case 3: return !_state.CPSR.Carry;
		case 4: return _state.CPSR.Negative;
		case 5: return !_state.CPSR.Negative;
		case 6: return _state.CPSR.Overflow;
		case 7: return !_state.CPSR.Overflow;
		case 8: return _state.CPSR.Carry && !_state.CPSR.Zero;
		case 9: return !_state.CPSR.Carry || _state.CPSR.Zero;
		case 10: return _state.CPSR.Negative == _state.CPSR.Overflow;
		case 11: return _state.CPSR.Negative != _state.CPSR.Overflow;
		case 12: return !_state.CPSR.Zero && (_state.CPSR.Negative == _state.CPSR.Overflow);
		case 13: return _state.CPSR.Zero || (_state.CPSR.Negative != _state.CPSR.Overflow);
		case 14: return true;
		case 15: return false;
	}

	return true;
}

void ArmV3Cpu::InitArmOpTable()
{
	auto addEntry = [=](int i, Func func, ArmOpCategory category) {
		_armTable[i] = func;
		_armCategory[i] = category;
	};

	for(int i = 0; i < 0x1000; i++) {
		addEntry(i, &ArmV3Cpu::ArmInvalidOp, ArmOpCategory::InvalidOp);
	}

	//Data Processing / PSR Transfer (MRS, MSR)
	//----_00??_????_----_----_----_????_----
	for(int i = 0; i <= 0x3FF; i++) {
		ArmAluOperation operation = (ArmAluOperation)((i >> 5) & 0x0F);
		bool setConditionCodes = (i & 0x10) != 0;
		if(!setConditionCodes && operation >= ArmAluOperation::Tst && operation <= ArmAluOperation::Cmn) {
			if(i & 0x020) {
				addEntry(0x000 | i, &ArmV3Cpu::ArmMsr, ArmOpCategory::Msr);
			} else {
				addEntry(0x000 | i, &ArmV3Cpu::ArmMrs, ArmOpCategory::Mrs);
			}
		} else {
			addEntry(0x000 | i, &ArmV3Cpu::ArmDataProcessing, ArmOpCategory::DataProcessing);
		}
	}

	//Branch and Branch with Link (B, BL)
	//----_101?_????_----_----_----_????_----
	for(int i = 0; i <= 0x1FF; i++) {
		addEntry(0xA00 | i, &ArmV3Cpu::ArmBranch, ArmOpCategory::Branch);
	}

	//Single Data Transfer (LDR, STR)
	//----_01??_????_----_----_----_????_----
	for(int i = 0; i <= 0x3FF; i++) {
		addEntry(0x400 | i, &ArmV3Cpu::ArmSingleDataTransfer, ArmOpCategory::SingleDataTransfer);
	}

	//Block Data Transfer (LDM, STM)
	//----_100?_????_----_----_----_????_----
	for(int i = 0; i <= 0x1FF; i++) {
		addEntry(0x800 | i, &ArmV3Cpu::ArmBlockDataTransfer, ArmOpCategory::BlockDataTransfer);
	}

	//Multiply and Multiply-Accumulate (MUL, MLA)
	//----_0000_00??_----_----_----_1001_----
	for(int i = 0; i <= 0x03; i++) {
		addEntry(0x009 | (i << 4), &ArmV3Cpu::ArmMultiply, ArmOpCategory::Multiply);
	}

	//Multiply Long and Multiply-Accumulate Long (MULL,MLAL)
	//----_0000_1???_----_----_----_1001_----
	for(int i = 0; i <= 0x07; i++) {
		addEntry(0x089 | (i << 4), &ArmV3Cpu::ArmMultiplyLong, ArmOpCategory::MultiplyLong);
	}

	//Single Data Swap (SWP)
	//----_0001_0000_----_----_----_1001_----
	addEntry(0x109, &ArmV3Cpu::ArmSingleDataSwap, ArmOpCategory::SingleDataSwap); //word
	addEntry(0x149, &ArmV3Cpu::ArmSingleDataSwap, ArmOpCategory::SingleDataSwap); //byte

	for(int i = 0; i <= 0xFF; i++) {
		addEntry(0xF00 + i, &ArmV3Cpu::ArmSoftwareInterrupt, ArmOpCategory::SoftwareInterrupt);
	}
}

void ArmV3Cpu::Serialize(Serializer& s)
{
	SV(_state.CycleCount);

	SV(_state.Pipeline.Fetch.Address);
	SV(_state.Pipeline.Fetch.OpCode);
	SV(_state.Pipeline.Decode.Address);
	SV(_state.Pipeline.Decode.OpCode);
	SV(_state.Pipeline.Execute.Address);
	SV(_state.Pipeline.Execute.OpCode);
	SV(_state.Pipeline.ReloadRequested);
	SV(_state.Pipeline.Mode);

	SV(_state.CPSR.Mode);
	SV(_state.CPSR.FiqDisable);
	SV(_state.CPSR.IrqDisable);
	SV(_state.CPSR.Overflow);
	SV(_state.CPSR.Carry);
	SV(_state.CPSR.Zero);
	SV(_state.CPSR.Negative);

	SVArray(_state.R, 16);
	SVArray(_state.UserRegs, 7);
	SVArray(_state.FiqRegs, 7);
	SVArray(_state.IrqRegs, 2);
	SVArray(_state.SupervisorRegs, 2);
	SVArray(_state.AbortRegs, 2);
	SVArray(_state.UndefinedRegs, 2);

	SV(_state.FiqSpsr.Mode);
	SV(_state.FiqSpsr.FiqDisable);
	SV(_state.FiqSpsr.IrqDisable);
	SV(_state.FiqSpsr.Overflow);
	SV(_state.FiqSpsr.Carry);
	SV(_state.FiqSpsr.Zero);
	SV(_state.FiqSpsr.Negative);

	SV(_state.IrqSpsr.Mode);
	SV(_state.IrqSpsr.FiqDisable);
	SV(_state.IrqSpsr.IrqDisable);
	SV(_state.IrqSpsr.Overflow);
	SV(_state.IrqSpsr.Carry);
	SV(_state.IrqSpsr.Zero);
	SV(_state.IrqSpsr.Negative);

	SV(_state.SupervisorSpsr.Mode);
	SV(_state.SupervisorSpsr.FiqDisable);
	SV(_state.SupervisorSpsr.IrqDisable);
	SV(_state.SupervisorSpsr.Overflow);
	SV(_state.SupervisorSpsr.Carry);
	SV(_state.SupervisorSpsr.Zero);
	SV(_state.SupervisorSpsr.Negative);

	SV(_state.AbortSpsr.Mode);
	SV(_state.AbortSpsr.FiqDisable);
	SV(_state.AbortSpsr.IrqDisable);
	SV(_state.AbortSpsr.Overflow);
	SV(_state.AbortSpsr.Carry);
	SV(_state.AbortSpsr.Zero);
	SV(_state.AbortSpsr.Negative);

	SV(_state.UndefinedSpsr.Mode);
	SV(_state.UndefinedSpsr.FiqDisable);
	SV(_state.UndefinedSpsr.IrqDisable);
	SV(_state.UndefinedSpsr.Overflow);
	SV(_state.UndefinedSpsr.Carry);
	SV(_state.UndefinedSpsr.Zero);
	SV(_state.UndefinedSpsr.Negative);
}

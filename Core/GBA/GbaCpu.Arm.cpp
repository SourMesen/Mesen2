#include "pch.h"
#include "GBA/GbaCpu.h"
#include "GBA/GbaMemoryManager.h"
#include "GBA/GbaCpuMultiply.h"
#include "Shared/Emulator.h"

ArmOpCategory GbaCpu::_armCategory[0x1000];
GbaCpu::Func GbaCpu::_armTable[0x1000];

ArmOpCategory GbaCpu::GetArmOpCategory(uint32_t opCode)
{
	uint16_t opType = ((opCode & 0x0FF00000) >> 16) | ((opCode & 0xF0) >> 4);
	return _armCategory[opType];
}

void GbaCpu::ArmBranchExchangeRegister()
{
	uint32_t value = R(_opCode & 0x0F);
	_state.CPSR.Thumb = (value & 0x01) != 0;
	_state.R[15] = value;
	_state.Pipeline.ReloadRequested = true;
}

void GbaCpu::ArmBranch()
{
	bool withLink = (_opCode & (1 << 24)) != 0;
	int32_t offset = (((int32_t)_opCode << 8) >> 6); //sign extend + shift right by 2
	if(withLink) {
		_state.R[14] = _state.R[15] - 4;
	} 

	_state.R[15] += offset;
	_state.Pipeline.ReloadRequested = true;
}

void GbaCpu::ArmMsr()
{
	//----_00i1_0d10_mmmm_1111_oooo_oooo_oooo
	//i: immediate
	//d: destination psr
	//o: operand
	//m: mask
	bool immediate = _opCode & (1 << 25);
	bool writeToSpsr = _opCode & (1 << 22);
	uint8_t mask = (_opCode >> 16) & 0x0F;

	if(writeToSpsr && (_state.CPSR.Mode == GbaCpuMode::User || _state.CPSR.Mode == GbaCpuMode::System)) {
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

	GbaCpuFlags& flags = writeToSpsr ? GetSpsr() : _state.CPSR;
	if(mask & 0x08) {
		flags.Negative = value & (1 << 31);
		flags.Zero = value & (1 << 30);
		flags.Carry = value & (1 << 29);
		flags.Overflow = value & (1 << 28);
	}
	
	if(mask & 0x01) {
		if(writeToSpsr || _state.CPSR.Mode != GbaCpuMode::User) {
			if(!writeToSpsr) {
				SwitchMode((GbaCpuMode)(value & 0x1F));
			} else {
				flags.Mode = (GbaCpuMode)(value & 0x1F);
			}
			flags.Thumb = value & (1 << 5);
			flags.FiqDisable = value & (1 << 6);
			flags.IrqDisable = value & (1 << 7);
		}
	}
}

void GbaCpu::ArmMrs()
{
	//----_0001_0s00_1111_dddd_0000_0000_0000
	//s: source psr
	//d: destination reg
	bool useSpsr = _opCode & (1 << 22);
	uint8_t rd = (_opCode >> 12) & 0x0F;
	SetR(rd, useSpsr ? GetSpsr().ToInt32() : _state.CPSR.ToInt32());
}

void GbaCpu::ArmDataProcessing()
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
		GbaCpuFlags spsr = GetSpsr();
		SwitchMode(spsr.Mode);
		_state.CPSR = spsr;
	}
}

void GbaCpu::ArmMultiply()
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

void GbaCpu::ArmMultiplyLong()
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

void GbaCpu::ArmSingleDataTransfer()
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

	GbaAccessModeVal mode = byte ? GbaAccessMode::Byte : GbaAccessMode::Word;
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

void GbaCpu::ArmSignedHalfDataTransfer()
{
	//Halfword and Signed Data Transfer	(LDRH / STRH / LDRSB / LDRSH)
	//----_000p_uiwl_nnnn_dddd_0000_1sh1_mmmm
	bool pre = (_opCode & (1 << 24)) != 0;
	bool up = (_opCode & (1 << 23)) != 0;
	bool immediate = (_opCode & (1 << 22)) != 0;
	bool writeBack = (_opCode & (1 << 21)) != 0;
	bool load = (_opCode & (1 << 20)) != 0;
	uint8_t rn = (_opCode >> 16) & 0x0F;
	uint8_t rd = (_opCode >> 12) & 0x0F;
	
	bool sign = (_opCode & (1 << 6)) != 0;
	bool half = (_opCode & (1 << 5)) != 0;

	int32_t offset;
	if(immediate) {
		offset = ((_opCode >> 4) & 0xF0) | (_opCode & 0x0F);
	} else {
		uint8_t rm = _opCode & 0x0F;
		offset = (int32_t)R(rm);
	}

	uint32_t addr = R(rn);

	if(pre) {
		addr += up ? offset : -offset;
	}

	if(load) {
		if(half) {
			SetR(rd, Read(GbaAccessMode::HalfWord | (sign ? GbaAccessMode::Signed : 0), addr));
		} else {
			SetR(rd, Read(GbaAccessMode::Byte | (sign ? GbaAccessMode::Signed : 0), addr));
		}
		Idle();
	} else {
		if(half) {
			Write(GbaAccessMode::HalfWord | (sign ? GbaAccessMode::Signed : 0), addr, R(rd));
		} else {
			Write(GbaAccessMode::Byte | (sign ? GbaAccessMode::Signed : 0), addr, R(rd));
		}
	}

	if(!pre) {
		addr += up ? offset : -offset;
	}

	if((rd != rn || !load) && (writeBack || !pre)) {
		SetR(rn, addr);
	}
}

void GbaCpu::ArmBlockDataTransfer()
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

	GbaCpuMode orgMode = _state.CPSR.Mode;
	if(psrForceUser && (!load || (load && !(regMask & 0x8000)))) {
		SwitchMode(GbaCpuMode::User);
	}

	bool firstReg = true;
	GbaAccessModeVal mode = GbaAccessMode::Word;
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
				SetR(i, Read(mode | GbaAccessMode::NoRotate, addr));
			}
			
			mode |= GbaAccessMode::Sequential;
			addr += 4;
		}
	}

	if(load) {
		Idle();
	}

	SwitchMode(orgMode);

	if(psrForceUser && load && (regMask & 0x8000)) {
		GbaCpuFlags spsr = GetSpsr();
		SwitchMode(spsr.Mode);
		_state.CPSR = spsr;
	}
}

void GbaCpu::ArmSingleDataSwap()
{
	//Single Data Swap (SWP)
	//----_0001_0b00_nnnn_dddd_0000_1001_mmmm
	bool byte = _opCode & (1 << 22);
	uint8_t rn = (_opCode >> 16) & 0x0F;
	uint8_t rd = (_opCode >> 12) & 0x0F;
	uint8_t rm = _opCode & 0x0F;

	uint32_t mode = byte ? GbaAccessMode::Byte : GbaAccessMode::Word;
#ifndef DUMMYCPU
	_memoryManager->LockBus();
#endif
	uint32_t val = Read(mode, R(rn));
	Idle();
	Write(mode, R(rn), R(rm));
#ifndef DUMMYCPU
	_memoryManager->UnlockBus();
#endif
	SetR(rd, val);
}

void GbaCpu::ArmSoftwareInterrupt()
{
	ProcessException(GbaCpuMode::Supervisor, GbaCpuVector::SoftwareIrq);
}

void GbaCpu::ArmInvalidOp()
{
#ifndef DUMMYCPU
	ProcessException(GbaCpuMode::Undefined, GbaCpuVector::Undefined);
	_emu->BreakIfDebugging(CpuType::Gba, BreakSource::GbaInvalidOpCode);
#endif
}

bool GbaCpu::CheckConditions(uint32_t condCode)
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

void GbaCpu::InitArmOpTable()
{
	auto addEntry = [=](int i, Func func, ArmOpCategory category) {
		_armTable[i] = func;
		_armCategory[i] = category;
	};

	for(int i = 0; i < 0x1000; i++) {
		addEntry(i, &GbaCpu::ArmInvalidOp, ArmOpCategory::InvalidOp);
	}

	//Data Processing / PSR Transfer (MRS, MSR)
	//----_00??_????_----_----_----_????_----
	for(int i = 0; i <= 0x3FF; i++) {
		ArmAluOperation operation = (ArmAluOperation)((i >> 5) & 0x0F);
		bool setConditionCodes = (i & 0x10) != 0;
		if(!setConditionCodes && operation >= ArmAluOperation::Tst && operation <= ArmAluOperation::Cmn) {
			if(i & 0x020) {
				addEntry(0x000 | i, &GbaCpu::ArmMsr, ArmOpCategory::Msr);
			} else {
				addEntry(0x000 | i, &GbaCpu::ArmMrs, ArmOpCategory::Mrs);
			}
		} else {
			addEntry(0x000 | i, &GbaCpu::ArmDataProcessing, ArmOpCategory::DataProcessing);
		}
	}

	//Branch and Exchange (BX)
	//----_0001_0010_----_----_----_0001_----
	addEntry(0x121, &GbaCpu::ArmBranchExchangeRegister, ArmOpCategory::BranchExchangeRegister);

	//Branch and Branch with Link (B, BL)
	//----_101?_????_----_----_----_????_----
	for(int i = 0; i <= 0x1FF; i++) {
		addEntry(0xA00 | i, &GbaCpu::ArmBranch, ArmOpCategory::Branch);
	}

	//Single Data Transfer (LDR, STR)
	//----_01??_????_----_----_----_????_----
	for(int i = 0; i <= 0x3FF; i++) {
		addEntry(0x400 | i, &GbaCpu::ArmSingleDataTransfer, ArmOpCategory::SingleDataTransfer);
	}

	//Halfword and Signed Data Transfer (LDRH / STRH / LDRSB / LDRSH)
	//----_000?_????_----_----_----_1??1_----
	for(int i = 0; i <= 0x7F; i++) {
		addEntry(((i & 0x7C) << 2) | ((i & 0x03) << 1) | 0x09, &GbaCpu::ArmSignedHalfDataTransfer, ArmOpCategory::SignedHalfDataTransfer);
	}

	//Block Data Transfer (LDM, STM)
	//----_100?_????_----_----_----_????_----
	for(int i = 0; i <= 0x1FF; i++) {
		addEntry(0x800 | i, &GbaCpu::ArmBlockDataTransfer, ArmOpCategory::BlockDataTransfer);
	}

	//Multiply and Multiply-Accumulate (MUL, MLA)
	//----_0000_00??_----_----_----_1001_----
	for(int i = 0; i <= 0x03; i++) {
		addEntry(0x009 | (i << 4), &GbaCpu::ArmMultiply, ArmOpCategory::Multiply);
	}

	//Multiply Long and Multiply-Accumulate Long (MULL,MLAL)
	//----_0000_1???_----_----_----_1001_----
	for(int i = 0; i <= 0x07; i++) {
		addEntry(0x089 | (i << 4), &GbaCpu::ArmMultiplyLong, ArmOpCategory::MultiplyLong);
	}

	//Single Data Swap (SWP)
	//----_0001_0000_----_----_----_1001_----
	addEntry(0x109, &GbaCpu::ArmSingleDataSwap, ArmOpCategory::SingleDataSwap); //word
	addEntry(0x149, &GbaCpu::ArmSingleDataSwap, ArmOpCategory::SingleDataSwap); //byte

	for(int i = 0; i <= 0xFF; i++) {
		addEntry(0xF00 + i, &GbaCpu::ArmSoftwareInterrupt, ArmOpCategory::SoftwareInterrupt);
	}
}

#include "pch.h"
#include "GBA/GbaCpu.h"
#include "GBA/GbaMemoryManager.h"
#include "GBA/GbaCpuMultiply.h"

GbaThumbOpCategory GbaCpu::_thumbCategory[0x100];
GbaCpu::Func GbaCpu::_thumbTable[0x100];

GbaThumbOpCategory GbaCpu::GetThumbOpCategory(uint16_t _opCode)
{
	return _thumbCategory[_opCode >> 8];
}

void GbaCpu::ThumbMoveShiftedRegister()
{
	uint8_t op = (_opCode >> 11) & 0x03;
	uint8_t shift = (_opCode >> 6) & 0x1F;
	uint8_t rs = (_opCode >> 3) & 0x07;
	uint8_t rd = _opCode & 0x07;

	bool carry = _state.CPSR.Carry;
	switch(op) {
		default:
		case 0: SetR(rd, ShiftLsl(R(rs), shift, carry)); break;
		case 1: SetR(rd, ShiftLsr(R(rs), shift ? shift : 32, carry)); break;
		case 2: SetR(rd, ShiftAsr(R(rs), shift ? shift : 32, carry)); break;
	}

	LogicalOp(_state.R[rd], carry, true);
}

void GbaCpu::ThumbAddSubtract()
{
	bool sub = _opCode & (1 << 9);
	bool immediate = _opCode & (1 << 10);
	uint8_t rnImmediate = (_opCode >> 6) & 0x7;
	uint8_t rs = (_opCode >> 3) & 0x07;
	uint8_t rd = _opCode & 0x07;

	uint32_t op2 = immediate ? rnImmediate : R(rnImmediate);
	if(sub) {
		SetR(rd, Sub(R(rs), op2, true, true));
	} else {
		SetR(rd, Add(R(rs), op2, false, true));
	}
}

void GbaCpu::ThumbMoveCmpAddSub()
{
	uint8_t op = (_opCode >> 11) & 0x03;
	uint8_t rd = (_opCode >> 8) & 0x07;
	uint8_t imm = _opCode & 0xFF;
	
	bool carry = _state.CPSR.Carry;
	switch(op) {
		default:
		case 0: SetR(rd, LogicalOp(imm, carry, true)); break; //MOV
		case 1: Sub(_state.R[rd], imm, true, true); break; //CMP
		case 2: SetR(rd, Add(R(rd), imm, false, true)); break; //ADD
		case 3: SetR(rd, Sub(R(rd), imm, true, true)); break; //SUB
	}
}

void GbaCpu::ThumbAluOperation()
{
	uint8_t op = (_opCode >> 6) & 0x0F;
	uint8_t rs = (_opCode >> 3) & 0x07;
	uint8_t rd = _opCode & 0x07;

	uint32_t op1 = R(rd);
	uint32_t op2 = R(rs);

	bool carry = _state.CPSR.Carry;
	switch(op) {
		case 0: SetR(rd, LogicalOp(op1 & op2, carry, true)); break;
		case 1: SetR(rd, LogicalOp(op1 ^ op2, carry, true)); break;
		
		case 2:
			Idle();
			SetR(rd, ShiftLsl(op1, op2, carry));
			LogicalOp(_state.R[rd], carry, true);
			break;

		case 3:
			Idle();
			SetR(rd, ShiftLsr(op1, op2, carry));
			LogicalOp(_state.R[rd], carry, true);
			break;

		case 4:
			Idle();
			SetR(rd, ShiftAsr(op1, op2, carry));
			LogicalOp(_state.R[rd], carry, true);
			break;

		case 5: SetR(rd, Add(op1, op2, carry, true)); break;
		case 6: SetR(rd, Sub(op1, op2, carry, true)); break;
		
		case 7:
			Idle();
			SetR(rd, ShiftRor(op1, op2, carry));
			LogicalOp(_state.R[rd], carry, true);
			break;

		case 8: LogicalOp(op1 & op2, carry, true); break;
		case 9: SetR(rd, Sub(0, op2, true, true)); break;
		case 10: Sub(op1, op2, true, true); break;
		case 11: Add(op1, op2, false, true); break;
		case 12: SetR(rd, LogicalOp(op1 | op2, carry, true)); break;
		
		case 13: {
			//MUL
			MultiplicationOutput output = GbaCpuMultiply::mul(op2, op1);
			Idle(output.CycleCount);

			SetR(rd, output.Output);
			_state.CPSR.Carry = output.Carry;
			_state.CPSR.Zero = _state.R[rd] == 0;
			_state.CPSR.Negative = (_state.R[rd] & (1 << 31));
			break;
		}

		case 14: SetR(rd, LogicalOp(op1 & ~op2, carry, true)); break;
		case 15: SetR(rd, LogicalOp(~op2, carry, true)); break;
	}
}

void GbaCpu::ThumbHiRegBranchExch()
{
	uint8_t op = (_opCode >> 8) & 0x03;
	uint8_t rs = ((_opCode >> 3) & 0x07) | ((_opCode & 0x40) >> 3);
	uint8_t rd = (_opCode & 0x07) | ((_opCode & 0x80) >> 4);
	
	bool carry = _state.CPSR.Carry;
	switch(op) {
		default:
		case 0: SetR(rd, Add(R(rd), R(rs), false, false)); break; //ADD
		case 1: Sub(R(rd), R(rs), true, true); break; //CMP
		case 2: SetR(rd, LogicalOp(R(rs), carry, false)); break; //MOV
		
		case 3:
			//BX
			uint32_t value = R(rs);
			_state.CPSR.Thumb = (value & 0x01) != 0;
			SetR(15, value);
			break;
	}
}

void GbaCpu::ThumbPcRelLoad()
{
	uint8_t rd = (_opCode >> 8) & 0x07;
	uint8_t immValue = _opCode & 0xFF;

	SetR(rd, Read(GbaAccessMode::Word, (R(15) & ~0x03) + (immValue << 2)));
	Idle();
}

void GbaCpu::ThumbLoadStoreRegOffset()
{
	uint8_t ro = (_opCode >> 6) & 0x07;
	uint8_t rb = (_opCode >> 3) & 0x07;
	uint8_t rd = _opCode & 0x07;
	bool byte = _opCode & (1 << 10);
	bool load = _opCode & (1 << 11);

	GbaAccessModeVal mode = byte ? GbaAccessMode::Byte : GbaAccessMode::Word;
	if(load) {
		SetR(rd, Read(mode, R(rb) + R(ro)));
		Idle();
	} else {
		Write(mode, R(rb) + R(ro), R(rd));
	}
}

void GbaCpu::ThumbLoadStoreSignExtended()
{
	uint8_t ro = (_opCode >> 6) & 0x07;
	uint8_t rb = (_opCode >> 3) & 0x07;
	uint8_t rd = _opCode & 0x07;
	bool sign = _opCode & (1 << 10);
	bool half = _opCode & (1 << 11);

	if(!sign && !half) {
		Write(GbaAccessMode::HalfWord, R(rb) + R(ro), R(rd));
	} else {
		GbaAccessModeVal mode = half ? GbaAccessMode::HalfWord : GbaAccessMode::Byte;
		if(sign) {
			mode |= GbaAccessMode::Signed;
		}

		SetR(rd, Read(mode, R(rb) + R(ro)));
		Idle();
	}
}

void GbaCpu::ThumbLoadStoreImmOffset()
{
	uint8_t rb = (_opCode >> 3) & 0x07;
	uint8_t rd = _opCode & 0x07;
	uint8_t offset = (_opCode >> 6) & 0x1F;
	bool load = _opCode & (1 << 11);
	bool byte = _opCode & (1 << 12);

	if(!byte) {
		offset <<= 2;
	}

	GbaAccessModeVal mode = byte ? GbaAccessMode::Byte : GbaAccessMode::Word;
	if(load) {
		SetR(rd, Read(mode, R(rb) + offset));
		Idle();
	} else {
		Write(mode, _state.R[rb] + offset, R(rd));
	}
}

void GbaCpu::ThumbLoadStoreHalfWord()
{
	uint8_t rb = (_opCode >> 3) & 0x07;
	uint8_t rd = _opCode & 0x07;
	uint8_t offset = ((_opCode >> 6) & 0x1F) << 1;
	bool load = _opCode & (1 << 11);

	if(load) {
		SetR(rd, Read(GbaAccessMode::HalfWord, R(rb) + offset));
		Idle();
	} else {
		Write(GbaAccessMode::HalfWord, R(rb) + offset, R(rd));
	}
}

void GbaCpu::ThumbSpRelLoadStore()
{
	uint8_t rd = (_opCode >> 8) & 0x07;
	uint16_t immValue = (_opCode & 0xFF) << 2;
	bool load = _opCode & (1 << 11);

	if(load) {
		SetR(rd, Read(GbaAccessMode::Word, _state.R[13] + immValue));
		Idle();
	} else {
		Write(GbaAccessMode::Word, _state.R[13] + immValue, R(rd));
	}
}

void GbaCpu::ThumbLoadAddress()
{
	uint8_t rd = (_opCode >> 8) & 0x07;
	uint16_t immValue = (_opCode & 0xFF) << 2;
	bool useSp = _opCode & (1 << 11);

	if(useSp) {
		SetR(rd, _state.R[13] + immValue);
	} else {
		SetR(rd, (R(15) & ~0x02) + immValue);
	}
}

void GbaCpu::ThumbAddOffsetToSp()
{
	uint16_t immValue = (_opCode & 0x7F) << 2;
	bool sign = _opCode & (1 << 7);

	_state.R[13] += sign ? -immValue : immValue;
}

void GbaCpu::ThumbPushPopReg()
{
	uint8_t regMask = _opCode & 0xFF;
	bool storeLrLoadPc = _opCode & (1 << 8);
	bool load = _opCode & (1 << 11);

	uint32_t sp = _state.R[13];
	if(!load) {
		uint8_t regCount = 0;
		for(int i = 0; i < 8; i++) {
			if(regMask & (1 << i)) {
				regCount++;
			}
		}
		sp -= regCount * 4 + (storeLrLoadPc ? 4 : 0);
		_state.R[13] = sp;
	}

	for(int i = 0; i < 8; i++) {
		if(regMask & (1 << i)) {
			if(load) {
				SetR(i, Read(GbaAccessMode::Word | GbaAccessMode::NoRotate, sp));
			} else {
				Write(GbaAccessMode::Word, sp, R(i));
			}
			sp += 4;
		}
	}

	if(storeLrLoadPc) {
		if(load) {
			SetR(15, Read(GbaAccessMode::Word | GbaAccessMode::NoRotate, sp));
		} else {
			Write(GbaAccessMode::Word, sp, _state.R[14]);
		}
		sp += 4;
	}

	if(load) {
		Idle();
		_state.R[13] = sp;
	}
}

void GbaCpu::ThumbMultipleLoadStore()
{
	uint16_t regMask = _opCode & 0xFF;
	uint8_t rb = (_opCode >> 8) & 0x07;
	bool load = _opCode & (1 << 11);

	uint32_t base = R(rb);
	uint32_t addr = base;
	
	uint8_t regCount = 0;
	for(int i = 0; i < 8; i++) {
		if(regMask & (1 << i)) {
			regCount++;
		}
	}

	if(!regMask) {
		//Glitch when mask is empty - only R15 is stored/loaded, but address changes as if all 16 were written/loaded
		regCount = 16;
		regMask = 0x8000;
	}

	bool firstReg = true;
	GbaAccessModeVal mode = GbaAccessMode::Word;
	for(int i = 0; i < 16; i++) {
		if(regMask & (1 << i)) {
			if(!load) {
				Write(mode, addr, R(i) + (i == 15 ? 2 : 0));
			}

			if(firstReg) {
				//Write-back happens at this point, based on the gba-tests/thumb test 230
				SetR(rb, base + (regCount * 4));
				firstReg = false;
			}

			if(load) {
				SetR(i, Read(mode | GbaAccessMode::NoRotate, addr));
			}

			mode |= GbaAccessMode::Sequential;
			addr += 4;
		}
	}

	if(load) {
		Idle();
	}
}

void GbaCpu::ThumbConditionalBranch()
{
	int16_t offset = ((int16_t)(int8_t)(_opCode & 0xFF)) << 1;
	uint8_t cond = (_opCode >> 8) & 0x0F;
	
	if(CheckConditions(cond)) {
		SetR(15, _state.R[15] + offset);
	}
}

void GbaCpu::ThumbSoftwareInterrupt()
{
	ProcessException(GbaCpuMode::Supervisor, GbaCpuVector::SoftwareIrq);
}

void GbaCpu::ThumbUnconditionalBranch()
{
	int16_t offset = ((int16_t)((_opCode & 0x7FF) << 5)) >> 4;
	SetR(15, _state.R[15] + offset);
}

void GbaCpu::ThumbLongBranchLink()
{
	uint16_t offset = _opCode & 0x7FF;
	bool high = _opCode & (1 << 11);
	if(!high) {
		int32_t relOffset = ((int32_t)offset << 21) >> 9;
		_state.R[14] = R(15) + relOffset;
	} else {
		uint32_t addr = _state.R[14] + (offset << 1);
		_state.R[14] = (_state.R[15] - 2) | 0x01;
		SetR(15, addr);
	}
}

void GbaCpu::InitThumbOpTable()
{
	auto addEntry = [=](int i, Func func, GbaThumbOpCategory category) {
		_thumbTable[i] = func;
		_thumbCategory[i] = category;
	};

	for(int i = 0; i < 0x100; i++) {
		addEntry(i, &GbaCpu::ArmInvalidOp, GbaThumbOpCategory::InvalidOp);
	}

	//Move shifted register
	//000?_????
	for(int i = 0; i <= 0x1F; i++) {
		addEntry(0x00 | i, &GbaCpu::ThumbMoveShiftedRegister, GbaThumbOpCategory::MoveShiftedRegister);
	}

	//Add/subtract
	//0001_1???
	for(int i = 0; i <= 0x7; i++) {
		addEntry(0x18 | i, &GbaCpu::ThumbAddSubtract, GbaThumbOpCategory::AddSubtract);
	}
	
	//Move/compare/add/subtract immediate
	//001?_????
	for(int i = 0; i <= 0x1F; i++) {
		addEntry(0x20 | i, &GbaCpu::ThumbMoveCmpAddSub, GbaThumbOpCategory::MoveCmpAddSub);
	}

	//ALU operations
	//0100_00??
	for(int i = 0; i <= 0x3; i++) {
		addEntry(0x40 | i, &GbaCpu::ThumbAluOperation, GbaThumbOpCategory::AluOperation);
	}

	//Hi reg operation / branch exchange
	//0100_01??
	for(int i = 0; i <= 0x3; i++) {
		addEntry(0x44 | i, &GbaCpu::ThumbHiRegBranchExch, GbaThumbOpCategory::HiRegBranchExch);
	}

	//PC-relative load
	//0100_1???
	for(int i = 0; i <= 0x7; i++) {
		addEntry(0x48 | i, &GbaCpu::ThumbPcRelLoad, GbaThumbOpCategory::PcRelLoad);
	}
	
	//Load/store with register offset
	//0101_??0?
	for(int i = 0; i <= 0x7; i++) {
		addEntry(0x50 | ((i & 0x06) << 1) | (i & 0x01), &GbaCpu::ThumbLoadStoreRegOffset, GbaThumbOpCategory::LoadStoreRegOffset);
	}

	//Load/store sign-extended	byte / halfword
	//0101_??1?
	for(int i = 0; i <= 0x7; i++) {
		addEntry(0x52 | ((i & 0x06) << 1) | (i & 0x01), &GbaCpu::ThumbLoadStoreSignExtended, GbaThumbOpCategory::LoadStoreSignExtended);
	}

	//Load/store with immediate offset
	//011?_????
	for(int i = 0; i <= 0x1F; i++) {
		addEntry(0x60 | i, &GbaCpu::ThumbLoadStoreImmOffset, GbaThumbOpCategory::LoadStoreImmOffset);
	}

	//Load/store half word
	//1000_????
	for(int i = 0; i <= 0xF; i++) {
		addEntry(0x80 | i, &GbaCpu::ThumbLoadStoreHalfWord, GbaThumbOpCategory::LoadStoreHalfWord);
	}

	//SP-relative load/store
	//1001_????
	for(int i = 0; i <= 0xF; i++) {
		addEntry(0x90 | i, &GbaCpu::ThumbSpRelLoadStore, GbaThumbOpCategory::SpRelLoadStore);
	}

	//Load address
	//1010_????
	for(int i = 0; i <= 0xF; i++) {
		addEntry(0xA0 | i, &GbaCpu::ThumbLoadAddress, GbaThumbOpCategory::LoadAddress);
	}

	//Add offset to stack pointer
	//1011_0000
	addEntry(0xB0, &GbaCpu::ThumbAddOffsetToSp, GbaThumbOpCategory::AddOffsetToSp);

	//Push/pop registers
	//1011_?10?
	for(int i = 0; i <= 0x3; i++) {
		addEntry(0xB4 | ((i & 0x02) << 2) | (i & 0x01), &GbaCpu::ThumbPushPopReg, GbaThumbOpCategory::PushPopReg);
	}

	//Multiple load/store
	//1100_????
	for(int i = 0; i <= 0xF; i++) {
		addEntry(0xC0 | i, &GbaCpu::ThumbMultipleLoadStore, GbaThumbOpCategory::MultipleLoadStore);
	}

	//Conditional branch
	//1101_????
	for(int i = 0; i <= 0xF; i++) {
		addEntry(0xD0 | i, &GbaCpu::ThumbConditionalBranch, GbaThumbOpCategory::ConditionalBranch);
	}

	//Software interrupt
	//1101_1111
	addEntry(0xDF, &GbaCpu::ThumbSoftwareInterrupt, GbaThumbOpCategory::SoftwareInterrupt);

	//Unconditional branch
	//1110_0???
	for(int i = 0; i <= 0x7; i++) {
		addEntry(0xE0 | i, &GbaCpu::ThumbUnconditionalBranch, GbaThumbOpCategory::UnconditionalBranch);
	}

	//Long branch with link
	//1111_????
	for(int i = 0; i <= 0xF; i++) {
		addEntry(0xF0 | i, &GbaCpu::ThumbLongBranchLink, GbaThumbOpCategory::LongBranchLink);
	}
}

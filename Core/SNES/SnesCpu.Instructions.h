/************************
Add/subtract operations
*************************/
void SnesCpu::Add8(uint8_t value)
{
	uint32_t result;
	if(CheckFlag(ProcFlags::Decimal)) {
		result = (_state.A & 0x0F) + (value & 0x0F) + (_state.PS & ProcFlags::Carry);
		if(result > 0x09) result += 0x06;
		result = (_state.A & 0xF0) + (value & 0xF0) + (result > 0x0F ? 0x10 : 0) + (result & 0x0F);
	} else {
		result = (_state.A & 0xFF) + value + (_state.PS & ProcFlags::Carry);
	}

	if(~(_state.A ^ value) & (_state.A ^ result) & 0x80) {
		SetFlags(ProcFlags::Overflow);
	} else {
		ClearFlags(ProcFlags::Overflow);
	}

	if(CheckFlag(ProcFlags::Decimal) && result > 0x9F) {
		result += 0x60;
	}

	ClearFlags(ProcFlags::Carry | ProcFlags::Negative | ProcFlags::Zero);
	SetZeroNegativeFlags((uint8_t)result);

	if(result > 0xFF) {
		SetFlags(ProcFlags::Carry);
	}

	_state.A = (_state.A & 0xFF00) | (uint8_t)result;
}

void SnesCpu::Add16(uint16_t value)
{
	uint32_t result;
	if(CheckFlag(ProcFlags::Decimal)) {
		result = (_state.A & 0x0F) + (value & 0x0F) + (_state.PS & ProcFlags::Carry);

		if(result > 0x09) result += 0x06;
		result = (_state.A & 0xF0) + (value & 0xF0) + (result > 0x0F ? 0x10 : 0) + (result & 0x0F);

		if(result > 0x9F) result += 0x60;
		result = (_state.A & 0xF00) + (value & 0xF00) + (result > 0xFF ? 0x100 : 0) + (result & 0xFF);

		if(result > 0x9FF) result += 0x600;
		result = (_state.A & 0xF000) + (value & 0xF000) + (result > 0xFFF ? 0x1000 : 0) + (result & 0xFFF);
	} else {
		result = _state.A + value + (_state.PS & ProcFlags::Carry);
	}

	if(~(_state.A ^ value) & (_state.A ^ result) & 0x8000) {
		SetFlags(ProcFlags::Overflow);
	} else {
		ClearFlags(ProcFlags::Overflow);
	}

	if(CheckFlag(ProcFlags::Decimal) && result > 0x9FFF) {
		result += 0x6000;
	}

	ClearFlags(ProcFlags::Carry | ProcFlags::Negative | ProcFlags::Zero);
	SetZeroNegativeFlags((uint16_t)result);

	if(result > 0xFFFF) {
		SetFlags(ProcFlags::Carry);
	}

	_state.A = (uint16_t)result;
}

void SnesCpu::ADC()
{
	if(CheckFlag(ProcFlags::MemoryMode8)) {
		Add8(GetByteValue());
	} else {
		Add16(GetWordValue());
	}
}

void SnesCpu::Sub8(uint8_t value)
{
	int32_t result;
	if(CheckFlag(ProcFlags::Decimal)) {
		result = (_state.A & 0x0F) + (value & 0x0F) + (_state.PS & ProcFlags::Carry);
		if(result <= 0x0F) result -= 0x06;
		result = (_state.A & 0xF0) + (value & 0xF0) + (result > 0x0F ? 0x10 : 0) + (result & 0x0F);
	} else {
		result = (_state.A & 0xFF) + value + (_state.PS & ProcFlags::Carry);
	}

	if(~(_state.A ^ value) & (_state.A ^ result) & 0x80) {
		SetFlags(ProcFlags::Overflow);
	} else {
		ClearFlags(ProcFlags::Overflow);
	}

	if(CheckFlag(ProcFlags::Decimal) && result <= 0xFF) {
		result -= 0x60;
	}

	ClearFlags(ProcFlags::Carry | ProcFlags::Negative | ProcFlags::Zero);
	SetZeroNegativeFlags((uint8_t)result);

	if(result > 0xFF) {
		SetFlags(ProcFlags::Carry);
	}

	_state.A = (_state.A & 0xFF00) | (uint8_t)result;
}

void SnesCpu::Sub16(uint16_t value)
{
	int32_t result;
	if(CheckFlag(ProcFlags::Decimal)) {
		result = (_state.A & 0x0F) + (value & 0x0F) + (_state.PS & ProcFlags::Carry);

		if(result <= 0x0F) result -= 0x06;
		result = (_state.A & 0xF0) + (value & 0xF0) + (result > 0x0F ? 0x10 : 0) + (result & 0x0F);

		if(result <= 0xFF) result -= 0x60;
		result = (_state.A & 0xF00) + (value & 0xF00) + (result > 0xFF ? 0x100 : 0) + (result & 0xFF);

		if(result <= 0xFFF) result -= 0x600;
		result = (_state.A & 0xF000) + (value & 0xF000) + (result > 0xFFF ? 0x1000 : 0) + (result & 0xFFF);
	} else {
		result = _state.A + value + (_state.PS & ProcFlags::Carry);
	}

	if(~(_state.A ^ value) & (_state.A ^ result) & 0x8000) {
		SetFlags(ProcFlags::Overflow);
	} else {
		ClearFlags(ProcFlags::Overflow);
	}

	if(CheckFlag(ProcFlags::Decimal) && result <= 0xFFFF) {
		result -= 0x6000;
	}

	ClearFlags(ProcFlags::Carry | ProcFlags::Negative | ProcFlags::Zero);
	SetZeroNegativeFlags((uint16_t)result);

	if(result > 0xFFFF) {
		SetFlags(ProcFlags::Carry);
	}

	_state.A = (uint16_t)result;
}

void SnesCpu::SBC()
{
	if(CheckFlag(ProcFlags::MemoryMode8)) {
		Sub8(~GetByteValue());
	} else {
		Sub16(~GetWordValue());
	}
}

/****************
Branch operations
****************/
void SnesCpu::BCC()
{
	BranchRelative(!CheckFlag(ProcFlags::Carry));
}

void SnesCpu::BCS()
{
	BranchRelative(CheckFlag(ProcFlags::Carry));
}

void SnesCpu::BEQ()
{
	BranchRelative(CheckFlag(ProcFlags::Zero));
}

void SnesCpu::BMI()
{
	BranchRelative(CheckFlag(ProcFlags::Negative));
}

void SnesCpu::BNE()
{
	BranchRelative(!CheckFlag(ProcFlags::Zero));
}

void SnesCpu::BPL()
{
	BranchRelative(!CheckFlag(ProcFlags::Negative));
}

void SnesCpu::BRA()
{
	BranchRelative(true);
}

void SnesCpu::BRL()
{
	_state.PC = (uint16_t)(_state.PC + (int16_t)_operand);
	IdleTakeBranch();
}

void SnesCpu::BVC()
{
	BranchRelative(!CheckFlag(ProcFlags::Overflow));
}

void SnesCpu::BVS()
{
	BranchRelative(CheckFlag(ProcFlags::Overflow));
}

void SnesCpu::BranchRelative(bool branch)
{
	if(branch) {
		int8_t offset = _operand;
		Idle();
		if(_state.EmulationMode && ((uint16_t)(_state.PC + offset) & 0xFF00) != (_state.PC & 0xFF00)) {
			//Extra cycle in emulation mode if crossing a page
			Idle();
		}
		_state.PC = (uint16_t)(_state.PC + offset);
		IdleTakeBranch();
	}
}

/***************************
Set/clear flag instructions
****************************/
void SnesCpu::CLC()
{
	ClearFlags(ProcFlags::Carry);
}

void SnesCpu::CLD()
{
	ClearFlags(ProcFlags::Decimal);
}

void SnesCpu::CLI()
{
	ClearFlags(ProcFlags::IrqDisable);
}

void SnesCpu::CLV()
{
	ClearFlags(ProcFlags::Overflow);
}

void SnesCpu::SEC()
{
	SetFlags(ProcFlags::Carry);
}

void SnesCpu::SED()
{
	SetFlags(ProcFlags::Decimal);
}

void SnesCpu::SEI()
{
	SetFlags(ProcFlags::IrqDisable);
}

void SnesCpu::REP()
{
	Idle();
	ClearFlags((uint8_t)_operand);
	if(_state.EmulationMode) {
		SetFlags(ProcFlags::MemoryMode8 | ProcFlags::IndexMode8);
	}
}

void SnesCpu::SEP()
{
	Idle();
	SetFlags((uint8_t)_operand);
	if(CheckFlag(ProcFlags::IndexMode8)) {
		//Truncate X/Y when 8-bit indexes are enabled
		_state.Y &= 0xFF;
		_state.X &= 0xFF;
	}
}

/******************************
Increment/decrement operations
*******************************/
void SnesCpu::DEX()
{
	IncDecReg(_state.X, -1);
}

void SnesCpu::DEY()
{
	IncDecReg(_state.Y, -1);
}

void SnesCpu::INX()
{
	IncDecReg(_state.X, 1);
}

void SnesCpu::INY()
{
	IncDecReg(_state.Y, 1);
}

void SnesCpu::DEC()
{
	IncDec(-1);
}

void SnesCpu::INC()
{
	IncDec(1);
}

void SnesCpu::DEC_Acc()
{
	SetRegister(_state.A, _state.A - 1, CheckFlag(ProcFlags::MemoryMode8));
}

void SnesCpu::INC_Acc()
{
	SetRegister(_state.A, _state.A + 1, CheckFlag(ProcFlags::MemoryMode8));
}

void SnesCpu::IncDecReg(uint16_t& reg, int8_t offset)
{
	SetRegister(reg, reg + offset, CheckFlag(ProcFlags::IndexMode8));
}

void SnesCpu::IncDec(int8_t offset)
{
	if(CheckFlag(ProcFlags::MemoryMode8)) {
		uint8_t value = GetByteValue();
		IdleOrDummyWrite(_operand, value);
		value += offset;
		SetZeroNegativeFlags(value);
		Write(_operand, value);
	} else {
		uint16_t value = GetWordValue();
		Idle();
		value += offset;
		SetZeroNegativeFlags(value);
		WriteWordRmw(_operand, value);
	}
}

/********************
Compare instructions
*********************/
void SnesCpu::Compare(uint16_t reg, bool eightBitMode)
{
	if(eightBitMode) {
		uint8_t value = GetByteValue();
		if((uint8_t)reg >= value) {
			SetFlags(ProcFlags::Carry);
		} else {
			ClearFlags(ProcFlags::Carry);
		}
		uint8_t result = (uint8_t)reg - value;
		SetZeroNegativeFlags(result);
	} else {
		uint16_t value = GetWordValue();
		if(reg >= value) {
			SetFlags(ProcFlags::Carry);
		} else {
			ClearFlags(ProcFlags::Carry);
		}

		uint16_t result = reg - value;
		SetZeroNegativeFlags(result);
	}
}

void SnesCpu::CMP()
{
	Compare(_state.A, CheckFlag(ProcFlags::MemoryMode8));
}

void SnesCpu::CPX()
{
	Compare(_state.X, CheckFlag(ProcFlags::IndexMode8));
}

void SnesCpu::CPY()
{
	Compare(_state.Y, CheckFlag(ProcFlags::IndexMode8));
}

/*****************
Jump instructions
******************/
void SnesCpu::JML()
{
	_state.K = (_operand >> 16) & 0xFF;
	_state.PC = (uint16_t)_operand;
	IdleEndJump();
}

void SnesCpu::JMP()
{
	_state.PC = (uint16_t)_operand;
	IdleEndJump();
}

void SnesCpu::JSL()
{
	uint8_t b1 = ReadOperandByte();
	uint8_t b2 = ReadOperandByte();

	PushByte(_state.K, false);
	Idle();

	uint8_t b3 = ReadOperandByte();

	PushWord(_state.PC - 1, false);

	_state.K = b3;
	_state.PC = b1 | (b2 << 8);
	RestrictStackPointerValue();
	IdleEndJump();
}

void SnesCpu::JSR()
{
	PushWord(_state.PC - 1);
	_state.PC = (uint16_t)_operand;
	IdleEndJump();
}

void SnesCpu::JMP_AbsIdxXInd()
{
	uint16_t baseAddr = ReadOperandWord() + _state.X;
	Idle();

	uint8_t lsb = ReadData(GetProgramAddress(baseAddr));
	uint8_t msb = ReadData(GetProgramAddress(baseAddr + 1));

	_operand = (uint16_t)GetProgramAddress(lsb | (msb << 8));
	JMP();
}

void SnesCpu::JSR_AbsIdxXInd()
{
	uint8_t lsb = ReadOperandByte();
	PushWord(_state.PC, false);
	uint8_t msb = ReadOperandByte();

	uint16_t baseAddr = (lsb | (msb << 8)) + _state.X;
	Idle();

	lsb = ReadData(GetProgramAddress(baseAddr));
	msb = ReadData(GetProgramAddress(baseAddr + 1));

	_state.PC = (uint16_t)GetProgramAddress(lsb | (msb << 8));
	RestrictStackPointerValue();
	IdleEndJump();
}

void SnesCpu::RTI()
{
	Idle();
	Idle();

	if(_state.EmulationMode) {
		SetPS(PopByte() | ProcFlags::MemoryMode8 | ProcFlags::IndexMode8);
		_state.PC = PopWord();
	} else {
		SetPS(PopByte());
		_state.PC = PopWord();
		_state.K = PopByte();
	}
	IdleEndJump();
}

void SnesCpu::RTL()
{
	Idle();
	Idle();

	_state.PC = PopWord(false);
	_state.PC++;
	_state.K = PopByte(false);
	RestrictStackPointerValue();
	IdleEndJump();
}

void SnesCpu::RTS()
{
	Idle();
	Idle();

	_state.PC = PopWord();
	Idle();
	_state.PC++;
	IdleEndJump();
}

/**********
Interrupts
***********/
void SnesCpu::ProcessInterrupt(uint16_t vector, bool forHardwareInterrupt)
{
	if(forHardwareInterrupt) {
		//IRQ/NMI waste 2 cycles here.  BRK/COP do not (because they do those 2 cycles while loading the OP code + signature byte)
		ReadCode(_state.PC);
		Idle();
	}

	if(_state.EmulationMode) {
		PushWord(_state.PC);
		PushByte(_state.PS | 0x20);

		SetFlags(ProcFlags::IrqDisable);
		ClearFlags(ProcFlags::Decimal);

		_state.K = 0;
		_state.PC = ReadVector(vector);
	} else {
		PushByte(_state.K);
		PushWord(_state.PC);
		PushByte(_state.PS);

		SetFlags(ProcFlags::IrqDisable);
		ClearFlags(ProcFlags::Decimal);

		_state.K = 0;
		_state.PC = ReadVector(vector);
	}
}

void SnesCpu::BRK()
{
	ProcessInterrupt(_state.EmulationMode ? SnesCpu::LegacyIrqVector : SnesCpu::BreakVector, false);
}

void SnesCpu::COP()
{
	ProcessInterrupt(_state.EmulationMode ? SnesCpu::LegacyCoprocessorVector : SnesCpu::CoprocessorVector, false);
}

/******************
Bitwise operations
*******************/
void SnesCpu::AND()
{
	if(CheckFlag(ProcFlags::MemoryMode8)) {
		SetRegister(_state.A, _state.A & GetByteValue(), true);
	} else {
		SetRegister(_state.A, _state.A & GetWordValue(), false);
	}
}

void SnesCpu::EOR()
{
	if(CheckFlag(ProcFlags::MemoryMode8)) {
		SetRegister(_state.A, _state.A ^ GetByteValue(), true);
	} else {
		SetRegister(_state.A, _state.A ^ GetWordValue(), false);
	}
}

void SnesCpu::ORA()
{
	if(CheckFlag(ProcFlags::MemoryMode8)) {
		SetRegister(_state.A, _state.A | GetByteValue(), true);
	} else {
		SetRegister(_state.A, _state.A | GetWordValue(), false);
	}
}

/****************
Shift operations
*****************/
template<typename T> T SnesCpu::ShiftLeft(T value)
{
	T result = value << 1;
	if(value & (1 << (sizeof(T) * 8 - 1))) {
		SetFlags(ProcFlags::Carry);
	} else {
		ClearFlags(ProcFlags::Carry);
	}
	SetZeroNegativeFlags(result);
	return result;
}

template<typename T> T SnesCpu::RollLeft(T value)
{
	T result = value << 1 | (_state.PS & ProcFlags::Carry);
	if(value & (1 << (sizeof(T) * 8 - 1))) {
		SetFlags(ProcFlags::Carry);
	} else {
		ClearFlags(ProcFlags::Carry);
	}
	SetZeroNegativeFlags(result);
	return result;
}

template<typename T> T SnesCpu::ShiftRight(T value)
{
	T result = value >> 1;
	if(value & 0x01) {
		SetFlags(ProcFlags::Carry);
	} else {
		ClearFlags(ProcFlags::Carry);
	}
	SetZeroNegativeFlags(result);
	return result;
}

template<typename T> T SnesCpu::RollRight(T value)
{
	T result = value >> 1 | ((_state.PS & 0x01) << (sizeof(T) * 8 - 1));
	if(value & 0x01) {
		SetFlags(ProcFlags::Carry);
	} else {
		ClearFlags(ProcFlags::Carry);
	}
	SetZeroNegativeFlags(result);
	return result;
}

void SnesCpu::ASL_Acc()
{
	if(CheckFlag(ProcFlags::MemoryMode8)) {
		_state.A = (_state.A & 0xFF00) | (ShiftLeft<uint8_t>((uint8_t)_state.A));
	} else {
		_state.A = ShiftLeft<uint16_t>(_state.A);
	}
}

void SnesCpu::ASL()
{
	if(CheckFlag(ProcFlags::MemoryMode8)) {
		uint8_t value = GetByteValue();
		IdleOrDummyWrite(_operand, value);
		Write(_operand, ShiftLeft<uint8_t>(value));
	} else {
		uint16_t value = GetWordValue();
		Idle();
		WriteWordRmw(_operand, ShiftLeft<uint16_t>(value));
	}
}

void SnesCpu::LSR_Acc()
{
	if(CheckFlag(ProcFlags::MemoryMode8)) {
		_state.A = (_state.A & 0xFF00) | ShiftRight<uint8_t>((uint8_t)_state.A);
	} else {
		_state.A = ShiftRight<uint16_t>(_state.A);
	}
}

void SnesCpu::LSR()
{
	if(CheckFlag(ProcFlags::MemoryMode8)) {
		uint8_t value = GetByteValue();
		IdleOrDummyWrite(_operand, value);
		Write(_operand, ShiftRight<uint8_t>(value));
	} else {
		uint16_t value = GetWordValue();
		Idle();
		WriteWordRmw(_operand, ShiftRight<uint16_t>(value));
	}
}

void SnesCpu::ROL_Acc()
{
	if(CheckFlag(ProcFlags::MemoryMode8)) {
		_state.A = (_state.A & 0xFF00) | RollLeft<uint8_t>((uint8_t)_state.A);
	} else {
		_state.A = RollLeft<uint16_t>(_state.A);
	}
}

void SnesCpu::ROL()
{
	if(CheckFlag(ProcFlags::MemoryMode8)) {
		uint8_t value = GetByteValue();
		IdleOrDummyWrite(_operand, value);
		Write(_operand, RollLeft<uint8_t>(value));
	} else {
		uint16_t value = GetWordValue();
		Idle();
		WriteWordRmw(_operand, RollLeft<uint16_t>(value));
	}
}

void SnesCpu::ROR_Acc()
{
	if(CheckFlag(ProcFlags::MemoryMode8)) {
		_state.A = (_state.A & 0xFF00) | RollRight<uint8_t>((uint8_t)_state.A);
	} else {
		_state.A = RollRight<uint16_t>(_state.A);
	}
}

void SnesCpu::ROR()
{
	if(CheckFlag(ProcFlags::MemoryMode8)) {
		uint8_t value = GetByteValue();
		IdleOrDummyWrite(_operand, value);
		Write(_operand, RollRight<uint8_t>(value));
	} else {
		uint16_t value = GetWordValue();
		Idle();
		WriteWordRmw(_operand, RollRight<uint16_t>(value));
	}
}

/***************
Move operations
****************/
void SnesCpu::MVN()
{
	_state.DBR = _operand & 0xFF;
	uint32_t destBank = _state.DBR << 16;
	uint32_t srcBank = (_operand << 8) & 0xFF0000;

	uint8_t value = ReadData(srcBank | _state.X);
	Write(destBank | _state.Y, value);

	Idle();
	Idle();

	_state.X++;
	_state.Y++;
	if(CheckFlag(ProcFlags::IndexMode8)) {
		_state.X &= 0xFF;
		_state.Y &= 0xFF;
	}

	_state.A--;

	if(_state.A != 0xFFFF) {
		//Operation isn't done, set the PC back to the start of the instruction
		_state.PC -= 3;
	}
}

void SnesCpu::MVP()
{
	_state.DBR = _operand & 0xFF;
	uint32_t destBank = _state.DBR << 16;
	uint32_t srcBank = (_operand << 8) & 0xFF0000;

	uint8_t value = ReadData(srcBank | _state.X);
	Write(destBank | _state.Y, value);

	Idle();
	Idle();

	_state.X--;
	_state.Y--;
	if(CheckFlag(ProcFlags::IndexMode8)) {
		_state.X &= 0xFF;
		_state.Y &= 0xFF;
	}

	_state.A--;

	if(_state.A != 0xFFFF) {
		//Operation isn't done, set the PC back to the start of the instruction
		_state.PC -= 3;
	}
}

/********************
Push/pull operations
*********************/
void SnesCpu::PEA()
{
	//Push Effective Address
	PushWord((uint16_t)_operand, false);
	RestrictStackPointerValue();
}

void SnesCpu::PEI()
{
	//Push Effective Indirect address
	PushWord(ReadDataWord(_operand), false);
	RestrictStackPointerValue();
}

void SnesCpu::PER()
{
	//Push Effective Relative address
	PushWord((uint16_t)((int16_t)_operand + _state.PC), false);
	RestrictStackPointerValue();
}

void SnesCpu::PHB()
{
	Idle();
	PushByte(_state.DBR);
}

void SnesCpu::PHD()
{
	Idle();
	PushWord(_state.D, false);
	RestrictStackPointerValue();
}

void SnesCpu::PHK()
{
	//"PHP, PHK, PHP, PLB, and PLP push and pull one byte from the stack"
	Idle();
	PushByte(_state.K);
}

void SnesCpu::PHP()
{
	//"PHP, PHK, PHP, PLB, and PLP push and pull one byte from the stack"
	Idle();
	PushByte(_state.PS);
}

void SnesCpu::PLB()
{
	//"PHP, PHK, PHP, PLB, and PLP push and pull one byte from the stack"
	Idle();
	Idle();
	SetRegister(_state.DBR, PopByte(false));
	RestrictStackPointerValue();
}

void SnesCpu::PLD()
{
	//"PHD and PLD push and pull two bytes from the stack."
	Idle();
	Idle();
	SetRegister(_state.D, PopWord(false), false);
	RestrictStackPointerValue();
}

void SnesCpu::PLP()
{
	//"For PLP, (all of) the flags are pulled from the stack. Note that when the e flag is 1, the m and x flag are forced to 1, so after the PLP, both flags will still be 1 no matter what value is pulled from the stack."
	Idle();
	Idle();
	if(_state.EmulationMode) {
		SetPS(PopByte() | ProcFlags::MemoryMode8 | ProcFlags::IndexMode8);
	} else {
		SetPS(PopByte());
	}
}

void SnesCpu::PHA()
{
	//"When the m flag is 0, PHA and PLA push and pull a 16-bit value, and when the m flag is 1, PHA and PLA push and pull an 8-bit value. "
	Idle();
	PushRegister(_state.A, CheckFlag(ProcFlags::MemoryMode8));
}

void SnesCpu::PHX()
{
	Idle();
	PushRegister(_state.X, CheckFlag(ProcFlags::IndexMode8));
}

void SnesCpu::PHY()
{
	Idle();
	PushRegister(_state.Y, CheckFlag(ProcFlags::IndexMode8));
}

void SnesCpu::PLA()
{
	//"When the m flag is 0, PHA and PLA push and pull a 16-bit value, and when the m flag is 1, PHA and PLA push and pull an 8-bit value."
	Idle();
	Idle();
	PullRegister(_state.A, CheckFlag(ProcFlags::MemoryMode8));
}

void SnesCpu::PLX()
{
	Idle();
	Idle();
	PullRegister(_state.X, CheckFlag(ProcFlags::IndexMode8));
}

void SnesCpu::PLY()
{
	Idle();
	Idle();
	PullRegister(_state.Y, CheckFlag(ProcFlags::IndexMode8));
}

void SnesCpu::PushRegister(uint16_t reg, bool eightBitMode)
{
	//"When the x flag is 0, PHX, PHY, PLX, and PLY push and pull a 16-bit value, and when the x flag is 1, PHX, PHY, PLX, and PLY push and pull an 8-bit value."
	if(eightBitMode) {
		PushByte((uint8_t)reg);
	} else {
		PushWord(reg);
	}
}

void SnesCpu::PullRegister(uint16_t& reg, bool eightBitMode)
{
	//"When the x flag is 0, PHX, PHY, PLX, and PLY push and pull a 16-bit value, and when the x flag is 1, PHX, PHY, PLX, and PLY push and pull an 8-bit value."
	if(eightBitMode) {
		SetRegister(reg, PopByte(), true);
	} else {
		SetRegister(reg, PopWord(), false);
	}
}

/*********************
Store/load operations
**********************/
void SnesCpu::LoadRegister(uint16_t& reg, bool eightBitMode)
{
	if(eightBitMode) {
		SetRegister(reg, GetByteValue(), true);
	} else {
		SetRegister(reg, GetWordValue(), false);
	}
}

void SnesCpu::StoreRegister(uint16_t val, bool eightBitMode)
{
	if(eightBitMode) {
		Write(_operand, (uint8_t)val);
	} else {
		WriteWord(_operand, val);
	}
}

void SnesCpu::LDA()
{
	//"When the m flag is 0, LDA, STA, and STZ are 16-bit operations"
	LoadRegister(_state.A, CheckFlag(ProcFlags::MemoryMode8));
}

void SnesCpu::LDX()
{
	//"When the x flag is 0, LDX, LDY, STX, and STY are 16-bit operations"
	LoadRegister(_state.X, CheckFlag(ProcFlags::IndexMode8));
}

void SnesCpu::LDY()
{
	//"When the x flag is 0, LDX, LDY, STX, and STY are 16-bit operations"
	LoadRegister(_state.Y, CheckFlag(ProcFlags::IndexMode8));
}

void SnesCpu::STA()
{
	//"When the m flag is 0, LDA, STA, and STZ are 16-bit operations"
	StoreRegister(_state.A, CheckFlag(ProcFlags::MemoryMode8));
}

void SnesCpu::STX()
{
	//"When the x flag is 0, LDX, LDY, STX, and STY are 16-bit operations"
	StoreRegister(_state.X, CheckFlag(ProcFlags::IndexMode8));
}

void SnesCpu::STY()
{
	//"When the x flag is 0, LDX, LDY, STX, and STY are 16-bit operations"
	StoreRegister(_state.Y, CheckFlag(ProcFlags::IndexMode8));
}

void SnesCpu::STZ()
{
	//"When the m flag is 0, LDA, STA, and STZ are 16-bit operations"
	StoreRegister(0, CheckFlag(ProcFlags::MemoryMode8));
}

/*******************
Bit test operations
********************/
template<typename T> void SnesCpu::TestBits(T value, bool alterZeroFlagOnly)
{
	if(alterZeroFlagOnly) {
		//"Immediate addressing only affects the z flag (with the result of the bitwise And), but does not affect the n and v flags."
		if(((T)_state.A & value) == 0) {
			SetFlags(ProcFlags::Zero);
		} else {
			ClearFlags(ProcFlags::Zero);
		}
	} else {
		ClearFlags(ProcFlags::Zero | ProcFlags::Overflow | ProcFlags::Negative);

		if(((T)_state.A & value) == 0) {
			SetFlags(ProcFlags::Zero);
		}

		if(value & (1 << (sizeof(T) * 8 - 2))) {
			SetFlags(ProcFlags::Overflow);
		}
		if(value & (1 << (sizeof(T) * 8 - 1))) {
			SetFlags(ProcFlags::Negative);
		}
	}
}

void SnesCpu::BIT()
{
	if(CheckFlag(ProcFlags::MemoryMode8)) {
		TestBits<uint8_t>(GetByteValue(), _immediateMode);
	} else {
		TestBits<uint16_t>(GetWordValue(), _immediateMode);
	}
}

void SnesCpu::TRB()
{
	if(CheckFlag(ProcFlags::MemoryMode8)) {
		uint8_t value = GetByteValue();
		TestBits<uint8_t>(value, true);

		IdleOrDummyWrite(_operand, value);
		value &= ~_state.A;

		Write(_operand, value);
	} else {
		uint16_t value = GetWordValue();
		TestBits<uint16_t>(value, true);

		Idle();
		value &= ~_state.A;

		WriteWordRmw(_operand, value);
	}
}

void SnesCpu::TSB()
{
	if(CheckFlag(ProcFlags::MemoryMode8)) {
		uint8_t value = GetByteValue();
		TestBits<uint8_t>(value, true);

		IdleOrDummyWrite(_operand, value);
		value |= _state.A;

		Write(_operand, value);
	} else {
		uint16_t value = GetWordValue();
		TestBits<uint16_t>(value, true);

		Idle();
		value |= _state.A;

		WriteWordRmw(_operand, value);
	}
}

/******************
Transfer operations
*******************/
void SnesCpu::TAX()
{
	SetRegister(_state.X, _state.A, CheckFlag(ProcFlags::IndexMode8));
}

void SnesCpu::TAY()
{
	SetRegister(_state.Y, _state.A, CheckFlag(ProcFlags::IndexMode8));
}

void SnesCpu::TCD()
{
	SetRegister(_state.D, _state.A, false);
}

void SnesCpu::TCS()
{
	SetSP(_state.A);
}

void SnesCpu::TDC()
{
	SetRegister(_state.A, _state.D, false);
}

void SnesCpu::TSC()
{
	SetRegister(_state.A, _state.SP, false);
}

void SnesCpu::TSX()
{
	SetRegister(_state.X, _state.SP, CheckFlag(ProcFlags::IndexMode8));
}

void SnesCpu::TXA()
{
	SetRegister(_state.A, _state.X, CheckFlag(ProcFlags::MemoryMode8));
}

void SnesCpu::TXS()
{
	SetSP(_state.X);
}

void SnesCpu::TXY()
{
	SetRegister(_state.Y, _state.X, CheckFlag(ProcFlags::IndexMode8));
}

void SnesCpu::TYA()
{
	SetRegister(_state.A, _state.Y, CheckFlag(ProcFlags::MemoryMode8));
}

void SnesCpu::TYX()
{
	SetRegister(_state.X, _state.Y, CheckFlag(ProcFlags::IndexMode8));
}

void SnesCpu::XBA()
{
	Idle();
	_state.A = ((_state.A & 0xFF) << 8) | ((_state.A >> 8) & 0xFF);
	SetZeroNegativeFlags((uint8_t)_state.A);
}

void SnesCpu::XCE()
{
	bool carry = CheckFlag(ProcFlags::Carry);
	if(_state.EmulationMode) {
		SetFlags(ProcFlags::Carry);
	} else {
		ClearFlags(ProcFlags::Carry);
	}
	_state.EmulationMode = carry;

	if(_state.EmulationMode) {
		SetPS(_state.PS | ProcFlags::IndexMode8 | ProcFlags::MemoryMode8);
		_state.SP = 0x100 | (_state.SP & 0xFF);
	}
}

/*****************
No operation (NOP)
******************/
void SnesCpu::NOP()
{
	//1-byte NOP
}

void SnesCpu::WDM()
{
	//2-byte NOP
}

/****************
Misc. operations
*****************/
void SnesCpu::STP()
{
	//Stop the CPU
	_state.StopState = SnesCpuStopState::Stopped;
}

void SnesCpu::WAI()
{
	//Wait for interrupt
	_state.StopState = SnesCpuStopState::WaitingForIrq;
	_waiOver = false;
}

/****************
Addressing modes
*****************/
void SnesCpu::AddrMode_Abs()
{
	_operand = GetDataAddress(ReadOperandWord());
}

void SnesCpu::AddrMode_AbsIdxX(bool isWrite)
{
	uint32_t baseAddr = GetDataAddress(ReadOperandWord());
	_operand = (baseAddr + _state.X) & 0xFFFFFF;
	if(isWrite || !CheckFlag(ProcFlags::IndexMode8) || (_operand & 0xFF00) != (baseAddr & 0xFF00)) {
		Idle();
	}
}

void SnesCpu::AddrMode_AbsIdxY(bool isWrite)
{
	uint32_t baseAddr = GetDataAddress(ReadOperandWord());
	_operand = (baseAddr + _state.Y) & 0xFFFFFF;
	if(isWrite || !CheckFlag(ProcFlags::IndexMode8) || (_operand & 0xFF00) != (baseAddr & 0xFF00)) {
		Idle();
	}
}

void SnesCpu::AddrMode_AbsLng()
{
	_operand = ReadOperandLong();
}

void SnesCpu::AddrMode_AbsLngIdxX()
{
	_operand = (ReadOperandLong() + _state.X) & 0xFFFFFF;
}

void SnesCpu::AddrMode_AbsJmp()
{
	_operand = GetProgramAddress(ReadOperandWord());
}

void SnesCpu::AddrMode_AbsLngJmp()
{
	_operand = ReadOperandLong();
}

void SnesCpu::AddrMode_AbsInd()
{
	uint16_t addr = ReadOperandWord();
	uint8_t lsb = ReadData(addr);
	uint8_t msb = ReadData((uint16_t)(addr + 1));
	_operand = lsb | (msb << 8);
}

void SnesCpu::AddrMode_AbsIndLng()
{
	uint16_t addr = ReadOperandWord();

	uint8_t b1 = ReadData(addr);
	uint8_t b2 = ReadData((uint16_t)(addr + 1));
	uint8_t b3 = ReadData((uint16_t)(addr + 2));

	_operand = b1 | (b2 << 8) | (b3 << 16);
}

void SnesCpu::AddrMode_Acc()
{
	IdleOrRead();
}

void SnesCpu::AddrMode_BlkMov()
{
	_operand = ReadOperandWord();
}

uint8_t SnesCpu::ReadDirectOperandByte()
{
	uint8_t value = ReadOperandByte();
	if(_state.D & 0xFF) {
		//Add 1 cycle for direct register low (DL) not equal 0
		Idle();
	}
	return value;
}

void SnesCpu::AddrMode_Dir()
{
	_readWriteMask = 0xFFFF;
	_operand = GetDirectAddress(ReadDirectOperandByte());
}

void SnesCpu::AddrMode_DirIdxX()
{
	_readWriteMask = 0xFFFF;
	_operand = GetDirectAddress(ReadDirectOperandByte() + _state.X);
	Idle();
}

void SnesCpu::AddrMode_DirIdxY()
{
	_readWriteMask = 0xFFFF;
	_operand = GetDirectAddress(ReadDirectOperandByte() + _state.Y);
	Idle();
}

void SnesCpu::AddrMode_DirInd()
{
	_operand = GetDataAddress(GetDirectAddressIndirectWord(ReadDirectOperandByte()));
}

void SnesCpu::AddrMode_DirIdxIndX()
{
	uint8_t operandByte = ReadDirectOperandByte();
	Idle();
	_operand = GetDataAddress(GetDirectAddressIndirectWordWithPageWrap(operandByte + _state.X));
}

void SnesCpu::AddrMode_DirIndIdxY(bool isWrite)
{
	uint32_t baseAddr = GetDataAddress(GetDirectAddressIndirectWord(ReadDirectOperandByte()));
	_operand = (baseAddr + _state.Y) & 0xFFFFFF;

	if(isWrite || !CheckFlag(ProcFlags::IndexMode8) || (_operand & 0xFF00) != (baseAddr & 0xFF00)) {
		Idle();
	}
}

void SnesCpu::AddrMode_DirIndLng()
{
	_operand = GetDirectAddressIndirectLong(ReadDirectOperandByte());
}

void SnesCpu::AddrMode_DirIndLngIdxY()
{
	_operand = (GetDirectAddressIndirectLong(ReadDirectOperandByte()) + _state.Y) & 0xFFFFFF;
}

void SnesCpu::AddrMode_Imm8()
{
	_immediateMode = true;
	_operand = ReadOperandByte();
}

void SnesCpu::AddrMode_Imm16()
{
	_immediateMode = true;
	_operand = ReadOperandWord();
}

void SnesCpu::AddrMode_ImmX()
{
	_immediateMode = true;
	_operand = CheckFlag(ProcFlags::IndexMode8) ? ReadOperandByte() : ReadOperandWord();
}

void SnesCpu::AddrMode_ImmM()
{
	_immediateMode = true;
	_operand = CheckFlag(ProcFlags::MemoryMode8) ? ReadOperandByte() : ReadOperandWord();
}

void SnesCpu::AddrMode_Imp()
{
	IdleOrRead();
}

void SnesCpu::AddrMode_RelLng()
{
	_operand = ReadOperandWord();
	Idle();
}

void SnesCpu::AddrMode_Rel()
{
	_operand = ReadOperandByte();
}

void SnesCpu::AddrMode_StkRel()
{
	_operand = (uint16_t)(ReadOperandByte() + _state.SP);
	Idle();
}

void SnesCpu::AddrMode_StkRelIndIdxY()
{
	uint16_t addr = (uint16_t)(ReadOperandByte() + _state.SP);
	Idle();

	uint8_t lsb = ReadData(addr);
	uint8_t msb = ReadData((addr + 1) & 0xFFFF);

	_operand = (GetDataAddress(lsb | (msb << 8)) + _state.Y) & 0xFFFFFF;
	Idle();
}

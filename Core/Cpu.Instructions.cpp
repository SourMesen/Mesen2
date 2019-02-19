#include "stdafx.h"
#include "Cpu.h"

/************************
Add/substract operations
*************************/
void Cpu::Add8(uint8_t value)
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

void Cpu::Add16(uint16_t value)
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

void Cpu::ADC()
{
	if(CheckFlag(ProcFlags::MemoryMode8)) {
		Add8(GetByteValue());
	} else {
		Add16(GetWordValue());
	}
}

void Cpu::Sub8(uint8_t value)
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

void Cpu::Sub16(uint16_t value)
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

void Cpu::SBC()
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
void Cpu::BCC()
{
	BranchRelative(!CheckFlag(ProcFlags::Carry));
}

void Cpu::BCS()
{
	BranchRelative(CheckFlag(ProcFlags::Carry));
}

void Cpu::BEQ()
{
	BranchRelative(CheckFlag(ProcFlags::Zero));
}

void Cpu::BMI()
{
	BranchRelative(CheckFlag(ProcFlags::Negative));
}

void Cpu::BNE()
{
	BranchRelative(!CheckFlag(ProcFlags::Zero));
}

void Cpu::BPL()
{
	BranchRelative(!CheckFlag(ProcFlags::Negative));
}

void Cpu::BRA()
{
	BranchRelative(true);
}

void Cpu::BRL()
{
	_state.PC = (uint16_t)(_state.PC + (int16_t)_operand);
}

void Cpu::BVC()
{
	BranchRelative(!CheckFlag(ProcFlags::Overflow));
}

void Cpu::BVS()
{
	BranchRelative(CheckFlag(ProcFlags::Overflow));
}

void Cpu::BranchRelative(bool branch)
{
	int8_t offset = _operand;
	if(branch) {
		_state.PC = (uint16_t)(_state.PC + offset);
	}
}

/***************************
Set/clear flag instructions
****************************/
void Cpu::CLC()
{
	ClearFlags(ProcFlags::Carry);
}

void Cpu::CLD()
{
	ClearFlags(ProcFlags::Decimal);
}

void Cpu::CLI()
{
	ClearFlags(ProcFlags::IrqDisable);
}

void Cpu::CLV()
{
	ClearFlags(ProcFlags::Overflow);
}

void Cpu::SEC()
{
	SetFlags(ProcFlags::Carry);
}

void Cpu::SED()
{
	SetFlags(ProcFlags::Decimal);
}

void Cpu::SEI()
{
	SetFlags(ProcFlags::IrqDisable);
}

void Cpu::REP()
{
	ClearFlags((uint8_t)_operand);
}

void Cpu::SEP()
{
	SetFlags((uint8_t)_operand);
}

/******************************
Increment/decrement operations
*******************************/
void Cpu::DEX()
{
	IncDecReg(_state.X, -1);
}

void Cpu::DEY()
{
	IncDecReg(_state.Y, -1);
}

void Cpu::INX()
{
	IncDecReg(_state.X, 1);
}

void Cpu::INY()
{
	IncDecReg(_state.Y, 1);
}

void Cpu::DEC()
{
	IncDec(-1);
}

void Cpu::INC()
{
	IncDec(1);
}

void Cpu::IncDecReg(uint16_t &reg, int8_t offset)
{
	SetRegister(reg, reg + offset, CheckFlag(ProcFlags::IndexMode8));
}

void Cpu::IncDec(int8_t offset)
{
	if(_instAddrMode == AddrMode::Acc) {
		SetRegister(_state.A, _state.A + offset, CheckFlag(ProcFlags::MemoryMode8));
	} else {
		if(CheckFlag(ProcFlags::MemoryMode8)) {
			uint8_t value = GetByteValue() + offset;
			SetZeroNegativeFlags(value);
			Write(_operand, value);
		} else {
			uint16_t value = GetWordValue() + offset;
			SetZeroNegativeFlags(value);
			WriteWord(_operand, value);
		}
	}
}

/********************
Compare instructions
*********************/
void Cpu::Compare(uint16_t reg, bool eightBitMode)
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

void Cpu::CMP()
{
	Compare(_state.A, CheckFlag(ProcFlags::MemoryMode8));
}

void Cpu::CPX()
{
	Compare(_state.X, CheckFlag(ProcFlags::IndexMode8));
}

void Cpu::CPY()
{
	Compare(_state.Y, CheckFlag(ProcFlags::IndexMode8));
}

/*****************
Jump instructions
******************/
void Cpu::JML()
{
	_state.K = (_operand >> 16) & 0xFF;
	_state.PC = (uint16_t)_operand;
}

void Cpu::JMP()
{
	_state.PC = (uint16_t)_operand;
}

void Cpu::JSL()
{
	PushByte(_state.K);
	PushWord(_state.PC - 1);
	_state.K = (_operand >> 16) & 0xFF;
	_state.PC = (uint16_t)_operand;
}

void Cpu::JSR()
{
	PushWord(_state.PC - 1);
	_state.PC = (uint16_t)_operand;
}

void Cpu::RTI()
{
	if(_state.EmulationMode) {
		_state.PS = PopByte(); //TODO: incorrect
		_state.PC = PopWord();
	} else {
		_state.PS = PopByte(); //TODO: incorrect
		_state.PC = PopWord();
		_state.K = PopByte();
	}
}

void Cpu::RTL()
{
	_state.PC = PopWord();
	_state.PC++;
	_state.K = PopByte();
}

void Cpu::RTS()
{
	_state.PC = PopWord();
	_state.PC++;
}

/**********
Interrupts
***********/
void Cpu::ProcessInterrupt(uint16_t vector)
{
	if(_state.EmulationMode) {
		PushWord(_state.PC);
		PushByte(_state.PS | 0x20);

		SetFlags(ProcFlags::IrqDisable);
		ClearFlags(ProcFlags::Decimal);

		_state.PC = ReadDataWord(vector);
	} else {
		PushByte(_state.K);
		PushWord(_state.PC);
		PushByte(_state.PS);

		SetFlags(ProcFlags::IrqDisable);
		ClearFlags(ProcFlags::Decimal);

		_state.K = 0;
		_state.PC = ReadDataWord(vector);
	}
}

void Cpu::BRK()
{
	ProcessInterrupt(_state.EmulationMode ? Cpu::LegacyIrqVector : Cpu::BreakVector);
}

void Cpu::COP()
{
	ProcessInterrupt(_state.EmulationMode ? Cpu::LegacyCoprocessorVector : Cpu::CoprocessorVector);
}

/******************
Bitwise operations
*******************/
void Cpu::AND()
{
	if(CheckFlag(ProcFlags::MemoryMode8)) {
		SetRegister(_state.A, _state.A & GetByteValue(), true);
	} else {
		SetRegister(_state.A, _state.A & GetWordValue(), false);
	}
}

void Cpu::EOR()
{
	if(CheckFlag(ProcFlags::MemoryMode8)) {
		SetRegister(_state.A, _state.A ^ GetByteValue(), true);
	} else {
		SetRegister(_state.A, _state.A ^ GetWordValue(), false);
	}
}

void Cpu::ORA()
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
template<typename T> T Cpu::ShiftLeft(T value)
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

template<typename T> T Cpu::RollLeft(T value)
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

template<typename T> T Cpu::ShiftRight(T value)
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

template<typename T> T Cpu::RollRight(T value)
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

void Cpu::ASL()
{
	if(_instAddrMode == AddrMode::Acc) {
		if(CheckFlag(ProcFlags::MemoryMode8)) {
			_state.A = (_state.A & 0xFF00) | (ShiftLeft<uint8_t>((uint8_t)_state.A));
		} else {
			_state.A = ShiftLeft<uint16_t>(_state.A);
		}
	} else {
		if(CheckFlag(ProcFlags::MemoryMode8)) {
			Write(_operand, ShiftLeft<uint8_t>(GetByteValue()));
		} else {
			WriteWord(_operand, ShiftLeft<uint16_t>(GetWordValue()));
		}
	}
}

void Cpu::LSR()
{
	if(_instAddrMode == AddrMode::Acc) {
		if(CheckFlag(ProcFlags::MemoryMode8)) {
			_state.A = (_state.A & 0xFF00) | ShiftRight<uint8_t>((uint8_t)_state.A);
		} else {
			_state.A = ShiftRight<uint16_t>(_state.A);
		}
	} else {
		if(CheckFlag(ProcFlags::MemoryMode8)) {
			Write(_operand, ShiftRight<uint8_t>(GetByteValue()));
		} else {
			WriteWord(_operand, ShiftRight<uint16_t>(GetWordValue()));
		}
	}
}

void Cpu::ROL()
{
	if(_instAddrMode == AddrMode::Acc) {
		if(CheckFlag(ProcFlags::MemoryMode8)) {
			_state.A = (_state.A & 0xFF00) | RollLeft<uint8_t>((uint8_t)_state.A);
		} else {
			_state.A = RollLeft<uint16_t>(_state.A);
		}
	} else {
		if(CheckFlag(ProcFlags::MemoryMode8)) {
			Write(_operand, RollLeft<uint8_t>(GetByteValue()));
		} else {
			WriteWord(_operand, RollLeft<uint16_t>(GetWordValue()));
		}
	}
}

void Cpu::ROR()
{
	if(_instAddrMode == AddrMode::Acc) {
		if(CheckFlag(ProcFlags::MemoryMode8)) {
			_state.A = (_state.A & 0xFF00) | RollRight<uint8_t>((uint8_t)_state.A);
		} else {
			_state.A = RollRight<uint16_t>(_state.A);
		}
	} else {
		if(CheckFlag(ProcFlags::MemoryMode8)) {
			Write(_operand, RollRight<uint8_t>(GetByteValue()));
		} else {
			WriteWord(_operand, RollRight<uint16_t>(GetWordValue()));
		}
	}
}

/***************
Move operations
****************/
void Cpu::MVN()
{
	uint32_t srcBank = (_operand << 16) & 0xFF0000;
	uint32_t destBank = (_operand << 8) & 0xFF0000;
	while(_state.A != 0xFFFF) {
		uint8_t value = ReadData(srcBank | _state.X);
		Write(destBank | _state.Y, value);
		
		_state.X++;
		_state.Y++;
		_state.A--;
	}
}

void Cpu::MVP()
{
	uint32_t srcBank = (_operand << 16) & 0xFF0000;
	uint32_t destBank = (_operand << 8) & 0xFF0000;
	while(_state.A != 0xFFFF) {
		uint8_t value = ReadData(srcBank | _state.X);
		Write(destBank | _state.Y, value);

		_state.X--;
		_state.Y--;
		_state.A--;
	}
}

/********************
Push/pull operations
*********************/
void Cpu::PEA()
{
	//Push Effective Address
	PushWord((uint16_t)_operand);
}

void Cpu::PEI()
{
	//Push Effective Indirect address
	PushWord(ReadDataWord(_operand));
}

void Cpu::PER()
{
	//Push Effective Relative address
	PushWord((uint16_t)((int16_t)_operand + _state.PC));
}

void Cpu::PHB()
{
	PushByte(_state.DBR);
}

void Cpu::PHD()
{
	PushWord(_state.D);
}

void Cpu::PHK()
{
	//"PHP, PHK, PHP, PLB, and PLP push and pull one byte from the stack"
	PushByte(_state.K);
}

void Cpu::PHP()
{
	//"PHP, PHK, PHP, PLB, and PLP push and pull one byte from the stack"
	PushByte(_state.PS);
}

void Cpu::PLB()
{
	//"PHP, PHK, PHP, PLB, and PLP push and pull one byte from the stack"
	SetRegister(_state.DBR, PopByte());
}

void Cpu::PLD()
{
	//"PHD and PLD push and pull two bytes from the stack."
	SetRegister(_state.D, PopWord(), false);
}

void Cpu::PLP()
{
	//"For PLP, (all of) the flags are pulled from the stack. Note that when the e flag is 1, the m and x flag are forced to 1, so after the PLP, both flags will still be 1 no matter what value is pulled from the stack."
	if(_state.EmulationMode) {
		_state.PS = PopByte() | ProcFlags::MemoryMode8 | ProcFlags::IndexMode8;
	} else {
		_state.PS = PopByte();
	}
}

void Cpu::PHA()
{
	//"When the m flag is 0, PHA and PLA push and pull a 16-bit value, and when the m flag is 1, PHA and PLA push and pull an 8-bit value. "
	PushRegister(_state.A, CheckFlag(ProcFlags::MemoryMode8));
}

void Cpu::PHX()
{
	PushRegister(_state.X, CheckFlag(ProcFlags::IndexMode8));
}

void Cpu::PHY()
{
	PushRegister(_state.Y, CheckFlag(ProcFlags::IndexMode8));
}

void Cpu::PLA()
{
	//"When the m flag is 0, PHA and PLA push and pull a 16-bit value, and when the m flag is 1, PHA and PLA push and pull an 8-bit value."
	PullRegister(_state.A, CheckFlag(ProcFlags::MemoryMode8));
}

void Cpu::PLX()
{
	PullRegister(_state.X, CheckFlag(ProcFlags::IndexMode8));
}

void Cpu::PLY()
{
	PullRegister(_state.Y, CheckFlag(ProcFlags::IndexMode8));
}

void Cpu::PushRegister(uint16_t reg, bool eightBitMode)
{
	//"When the x flag is 0, PHX, PHY, PLX, and PLY push and pull a 16-bit value, and when the x flag is 1, PHX, PHY, PLX, and PLY push and pull an 8-bit value."
	if(eightBitMode) {
		PushByte((uint8_t)reg);
	} else {
		PushWord(reg);
	}
}

void Cpu::PullRegister(uint16_t &reg, bool eightBitMode)
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
void Cpu::LoadRegister(uint16_t &reg, bool eightBitMode)
{
	if(eightBitMode) {
		SetRegister(reg, GetByteValue(), true);
	} else {
		SetRegister(reg, GetWordValue(), false);
	}
}

void Cpu::StoreRegister(uint16_t val, bool eightBitMode)
{
	if(eightBitMode) {
		Write(_operand, (uint8_t)val);
	} else {
		WriteWord(_operand, val);
	}
}

void Cpu::LDA()
{
	//"When the m flag is 0, LDA, STA, and STZ are 16-bit operations"
	LoadRegister(_state.A, CheckFlag(ProcFlags::MemoryMode8));
}

void Cpu::LDX()
{
	//"When the x flag is 0, LDX, LDY, STX, and STY are 16-bit operations"
	LoadRegister(_state.X, CheckFlag(ProcFlags::IndexMode8));
}

void Cpu::LDY()
{
	//"When the x flag is 0, LDX, LDY, STX, and STY are 16-bit operations"
	LoadRegister(_state.Y, CheckFlag(ProcFlags::IndexMode8));
}

void Cpu::STA()
{
	//"When the m flag is 0, LDA, STA, and STZ are 16-bit operations"
	StoreRegister(_state.A, CheckFlag(ProcFlags::MemoryMode8));
}

void Cpu::STX()
{
	//"When the x flag is 0, LDX, LDY, STX, and STY are 16-bit operations"
	StoreRegister(_state.X, CheckFlag(ProcFlags::IndexMode8));
}

void Cpu::STY()
{
	//"When the x flag is 0, LDX, LDY, STX, and STY are 16-bit operations"
	StoreRegister(_state.Y, CheckFlag(ProcFlags::IndexMode8));
}

void Cpu::STZ()
{
	//"When the m flag is 0, LDA, STA, and STZ are 16-bit operations"
	StoreRegister(0, CheckFlag(ProcFlags::MemoryMode8));
}

/*******************
Bit test operations
********************/
template<typename T> void Cpu::TestBits(T value, bool alterZeroFlagOnly)
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

void Cpu::BIT()
{
	if(CheckFlag(ProcFlags::MemoryMode8)) {
		TestBits<uint8_t>(GetByteValue(), _instAddrMode <= AddrMode::ImmM);
	} else {
		TestBits<uint16_t>(GetWordValue(), _instAddrMode <= AddrMode::ImmM);
	}
}

void Cpu::TRB()
{
	if(CheckFlag(ProcFlags::MemoryMode8)) {
		TestBits<uint8_t>(GetByteValue(), true);

		uint8_t value = ReadData(_operand);
		value &= ~_state.A;
		Write(_operand, value);
	} else {
		TestBits<uint16_t>(GetWordValue(), true);
		
		uint16_t value = ReadDataWord(_operand);
		value &= ~_state.A;
		WriteWord(_operand, value);
	}	
}

void Cpu::TSB()
{
	if(CheckFlag(ProcFlags::MemoryMode8)) {
		TestBits<uint8_t>(GetByteValue(), true);

		uint8_t value = ReadData(_operand);
		value |= _state.A;
		Write(_operand, value);
	} else {
		TestBits<uint16_t>(GetWordValue(), true);

		uint16_t value = ReadDataWord(_operand);
		value |= _state.A;
		WriteWord(_operand, value);
	}
}

/******************
Transfer operations
*******************/
void Cpu::TAX()
{
	SetRegister(_state.X, _state.A, CheckFlag(ProcFlags::IndexMode8));
}

void Cpu::TAY()
{
	SetRegister(_state.Y, _state.A, CheckFlag(ProcFlags::IndexMode8));
}

void Cpu::TCD()
{
	SetRegister(_state.D, _state.A, false);
}

void Cpu::TCS()
{
	SetSP(_state.A);
}

void Cpu::TDC()
{
	SetRegister(_state.A, _state.D, CheckFlag(ProcFlags::MemoryMode8));
}

void Cpu::TSC()
{
	SetRegister(_state.A, _state.SP, CheckFlag(ProcFlags::MemoryMode8));
}

void Cpu::TSX()
{
	SetRegister(_state.X, _state.SP, CheckFlag(ProcFlags::IndexMode8));
}

void Cpu::TXA()
{
	SetRegister(_state.A, _state.X, CheckFlag(ProcFlags::MemoryMode8));
}

void Cpu::TXS()
{
	SetSP(_state.X);
}

void Cpu::TXY()
{
	SetRegister(_state.Y, _state.X, CheckFlag(ProcFlags::IndexMode8));
}

void Cpu::TYA()
{
	SetRegister(_state.A, _state.Y, CheckFlag(ProcFlags::MemoryMode8));
}

void Cpu::TYX()
{
	SetRegister(_state.X, _state.Y, CheckFlag(ProcFlags::IndexMode8));
}

void Cpu::XBA()
{
	_state.A = ((_state.A & 0xFF) << 8) | ((_state.A >> 8) & 0xFF);
	SetZeroNegativeFlags((uint8_t)_state.A);
}

void Cpu::XCE()
{
	bool carry = CheckFlag(ProcFlags::Carry);
	if(_state.EmulationMode) {
		SetFlags(ProcFlags::Carry);
	} else {
		ClearFlags(ProcFlags::Carry);
	}
	_state.EmulationMode = carry;

	if(_state.EmulationMode) {
		SetFlags(ProcFlags::IndexMode8 | ProcFlags::MemoryMode8);
		//_state.A &= 0xFF;
		_state.Y &= 0xFF;
		_state.X &= 0xFF;
		_state.SP = 0x100 | (_state.SP & 0xFF);
	}
}

/*****************
No operation (NOP)
******************/
void Cpu::NOP()
{
	//1-byte NOP
}

void Cpu::WDM()
{
	//2-byte NOP
}

/****************
Misc. operations
*****************/
void Cpu::STP()
{
	//Stop the CPU
}

void Cpu::WAI()
{
	//Wait for interrupt
}
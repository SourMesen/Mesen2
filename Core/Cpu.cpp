#include "stdafx.h"
#include "CpuTypes.h"
#include "Cpu.h"
#include "MemoryManager.h"

//TODO: PER doesn't load the right number of bytes?

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
		result = (uint32_t)_state.A + value + (_state.PS & ProcFlags::Carry);
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

	if(result & 0x100) {
		SetFlags(ProcFlags::Carry);
	}

	_state.A = (uint8_t)result;
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
		result = (uint32_t)_state.A + value + (_state.PS & ProcFlags::Carry);
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

	if(result & 0x10000) {
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
	uint32_t result;
	if(CheckFlag(ProcFlags::Decimal)) {
		result = (_state.A & 0x0F) + (value & 0x0F) + (_state.PS & ProcFlags::Carry);
		if(result <= 0x0F) result -= 0x06;
		result = (_state.A & 0xF0) + (value & 0xF0) + (result > 0x0F ? 0x10 : 0) + (result & 0x0F);
	} else {
		result = (uint32_t)_state.A + value + (_state.PS & ProcFlags::Carry);
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

	if(result & 0x100) {
		SetFlags(ProcFlags::Carry);
	}

	_state.A = (uint8_t)result;
}

void Cpu::Sub16(uint16_t value)
{
	uint32_t result;
	if(CheckFlag(ProcFlags::Decimal)) {
		result = (_state.A & 0x0F) + (value & 0x0F) + (_state.PS & ProcFlags::Carry);

		if(result <= 0x0F) result -= 0x06;
		result = (_state.A & 0xF0) + (value & 0xF0) + (result > 0x0F ? 0x10 : 0) + (result & 0x0F);

		if(result <= 0xFF) result -= 0x60;
		result = (_state.A & 0xF00) + (value & 0xF00) + (result > 0xFF ? 0x100 : 0) + (result & 0xFF);

		if(result <= 0xFFF) result -= 0x600;
		result = (_state.A & 0xF000) + (value & 0xF000) + (result > 0xFFF ? 0x1000 : 0) + (result & 0xFFF);
	} else {
		result = (uint32_t)_state.A + value + (_state.PS & ProcFlags::Carry);
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

	if(result & 0x10000) {
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
		PushWord(_state.PC + 1);
		PushByte(_state.PS | 0x20);

		SetFlags(ProcFlags::IrqDisable);
		ClearFlags(ProcFlags::Decimal);

		_state.PC = ReadDataWord(vector);
	} else {
		PushByte(_state.K);
		PushWord(_state.PC + 1);
		PushByte(_state.PS);

		SetFlags(ProcFlags::IrqDisable);
		ClearFlags(ProcFlags::Decimal);

		_state.PC = ReadDataWord(vector);
	}
}

void Cpu::BRK()
{
	ProcessInterrupt(_state.EmulationMode ? Cpu::LegacyBreakVector : Cpu::BreakVector);
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
	SetRegister(_state.A, _state.A & GetByteValue(), CheckFlag(ProcFlags::MemoryMode8));
}

void Cpu::EOR()
{
	SetRegister(_state.A, _state.A ^ GetByteValue(), CheckFlag(ProcFlags::MemoryMode8));
}

void Cpu::ORA()
{
	SetRegister(_state.A, _state.A | GetByteValue(), CheckFlag(ProcFlags::MemoryMode8));
}

/****************
Shift operations
*****************/
template<typename T> T Cpu::ShiftLeft(T value)
{
	T result = value << 1;
	if(value & (1 << (sizeof(T)*8-1))) {
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
			_state.A = ShiftLeft<uint8_t>((uint8_t)_state.A);
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
			_state.A = ShiftRight<uint8_t>((uint8_t)_state.A);
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
			_state.A = RollLeft<uint8_t>((uint8_t)_state.A);
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
			_state.A = RollRight<uint8_t>((uint8_t)_state.A);
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
	//TODO
}

void Cpu::MVP()
{
	//TODO
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
	PushWord((uint16_t)_operand);
}

void Cpu::PER()
{
	//Push Effective Relative address
	PushWord((uint16_t)_operand);
}

void Cpu::PHB()
{
	PushWord(_state.DBR);
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
template<typename T> void Cpu::TestBits(T value)
{
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

void Cpu::BIT()
{
	if(CheckFlag(ProcFlags::MemoryMode8)) {
		TestBits<uint8_t>(GetByteValue());
	} else {
		TestBits<uint16_t>(GetWordValue());
	}
}

void Cpu::TRB()
{
	//TODO
}

void Cpu::TSB()
{
	//TODO
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
	SetRegister(_state.DBR, (uint8_t)_state.A);
}

void Cpu::TCS()
{
	_state.SP = _state.A;
}

void Cpu::TDC()
{
	SetRegister(_state.A, _state.DBR, CheckFlag(ProcFlags::MemoryMode8));
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
	_state.SP = _state.X;
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
	_state.EmulationMode = (_state.PS & ProcFlags::Carry) ? true : false;
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

Cpu::Cpu(shared_ptr<MemoryManager> memoryManager)
{
	typedef Cpu C;
	Func opTable[256] = {
		//0		1			2			3			4			5			6			7			8			9			A			B			C			D			E			F
		&C::BRK, &C::ORA, &C::COP, &C::ORA, &C::TSB, &C::ORA, &C::ASL, &C::ORA, &C::PHP, &C::ORA, &C::ASL, &C::PHD, &C::TSB, &C::ORA, &C::ASL, &C::ORA, // 0
		&C::BPL, &C::ORA, &C::ORA, &C::ORA, &C::TRB, &C::ORA, &C::ASL, &C::ORA, &C::CLC, &C::ORA, &C::INC, &C::TCS, &C::TRB, &C::ORA, &C::ASL, &C::ORA, // 1
		&C::JSR, &C::AND, &C::JSL, &C::AND, &C::BIT, &C::AND, &C::ROL, &C::AND, &C::PLP, &C::AND, &C::ROL, &C::PLD, &C::BIT, &C::AND, &C::ROL, &C::AND, // 2
		&C::BMI, &C::AND, &C::AND, &C::AND, &C::BIT, &C::AND, &C::ROL, &C::AND, &C::SEC, &C::AND, &C::DEC, &C::TSC, &C::BIT, &C::AND, &C::ROL, &C::AND, // 3
		&C::RTI, &C::EOR, &C::WDM, &C::EOR, &C::MVP, &C::EOR, &C::LSR, &C::EOR, &C::PHA, &C::EOR, &C::LSR, &C::PHK, &C::JMP, &C::EOR, &C::LSR, &C::EOR, // 4
		&C::BVC, &C::EOR, &C::EOR, &C::EOR, &C::MVN, &C::EOR, &C::LSR, &C::EOR, &C::CLI, &C::EOR, &C::PHY, &C::TCD, &C::JMP, &C::EOR, &C::LSR, &C::EOR, // 5
		&C::RTS, &C::ADC, &C::PER, &C::ADC, &C::STZ, &C::ADC, &C::ROR, &C::ADC, &C::PLA, &C::ADC, &C::ROR, &C::RTL, &C::JMP, &C::ADC, &C::ROR, &C::ADC, // 6
		&C::BVS, &C::ADC, &C::ADC, &C::ADC, &C::STZ, &C::ADC, &C::ROR, &C::ADC, &C::SEI, &C::ADC, &C::PLY, &C::TDC, &C::JMP, &C::ADC, &C::ROR, &C::ADC, // 7
		&C::BRA, &C::STA, &C::BRL, &C::STA, &C::STY, &C::STA, &C::STX, &C::STA, &C::DEY, &C::BIT, &C::TXA, &C::PHB, &C::STY, &C::STA, &C::STX, &C::STA, // 8
		&C::BCC, &C::STA, &C::STA, &C::STA, &C::STY, &C::STA, &C::STX, &C::STA, &C::TYA, &C::STA, &C::TXS, &C::TXY, &C::STZ, &C::STA, &C::STZ, &C::STA, // 9
		&C::LDY, &C::LDA, &C::LDX, &C::LDA, &C::LDY, &C::LDA, &C::LDX, &C::LDA, &C::TAY, &C::LDA, &C::TAX, &C::PLB, &C::LDY, &C::LDA, &C::LDX, &C::LDA, // A
		&C::BCS, &C::LDA, &C::LDA, &C::LDA, &C::LDY, &C::LDA, &C::LDX, &C::LDA, &C::CLV, &C::LDA, &C::TSX, &C::TYX, &C::LDY, &C::LDA, &C::LDX, &C::LDA, // B
		&C::CPY, &C::CMP, &C::REP, &C::CMP, &C::CPY, &C::CMP, &C::DEC, &C::CMP, &C::INY, &C::CMP, &C::DEX, &C::WAI, &C::CPY, &C::CMP, &C::DEC, &C::CMP, // C
		&C::BNE, &C::CMP, &C::CMP, &C::CMP, &C::PEI, &C::CMP, &C::DEC, &C::CMP, &C::CLD, &C::CMP, &C::PHX, &C::STP, &C::JML, &C::CMP, &C::DEC, &C::CMP, // D
		&C::CPX, &C::SBC, &C::SEP, &C::SBC, &C::CPX, &C::SBC, &C::INC, &C::SBC, &C::INX, &C::SBC, &C::NOP, &C::XBA, &C::CPX, &C::SBC, &C::INC, &C::SBC, // E
		&C::BEQ, &C::SBC, &C::SBC, &C::SBC, &C::PEA, &C::SBC, &C::INC, &C::SBC, &C::SED, &C::SBC, &C::PLX, &C::XCE, &C::JSR, &C::SBC, &C::INC, &C::SBC  // F
	};

	typedef AddrMode M;
	AddrMode addrMode[256] = {
		//0       1              2            3                 4           5           6           7                 8       9           A       B       C              D           E           F           
		M::Stk,   M::DirIdxIndX, M::Stk,      M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Stk, M::ImmM,    M::Acc, M::Stk, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // 0
		M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::Dir,     M::DirIdxX, M::DirIdxX, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Acc, M::Imp, M::Abs,        M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX, // 1
		M::Abs,   M::DirIdxIndX, M::AbsLng,   M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Stk, M::ImmM,    M::Acc, M::Stk, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // 2
		M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::DirIdxX, M::DirIdxX, M::DirIdxX, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Acc, M::Imp, M::AbsIdxX,    M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX, // 3
		M::Stk,   M::DirIdxIndX, M::Imm8,     M::StkRel,        M::BlkMov,  M::Dir,     M::Dir,     M::DirIndLng,     M::Stk, M::ImmM,    M::Acc, M::Stk, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // 4
		M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::BlkMov,  M::DirIdxX, M::DirIdxX, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Stk, M::Imp, M::AbsLng,     M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX, // 5
		M::Stk,   M::DirIdxIndX, M::Stk,      M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Stk, M::ImmM,    M::Acc, M::Stk, M::AbsInd,     M::Abs,     M::Abs,     M::AbsLng,     // 6
		M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::DirIdxX, M::DirIdxX, M::DirIdxX, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Stk, M::Imp, M::AbsIdxXInd, M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX, // 7
		M::Rel,   M::DirIdxIndX, M::RelLng,   M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Imp, M::ImmM,    M::Imp, M::Stk, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // 8
		M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::DirIdxX, M::DirIdxX, M::DirIdxY, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Imp, M::Imp, M::Abs,        M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX, // 9
		M::ImmX,  M::DirIdxIndX, M::ImmX,     M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Imp, M::ImmM,    M::Imp, M::Stk, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // A
		M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::DirIdxX, M::DirIdxX, M::DirIdxY, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Imp, M::Imp, M::AbsIdxX,    M::AbsIdxX, M::AbsIdxY, M::AbsLngIdxX, // B
		M::ImmX,  M::DirIdxIndX, M::Imm8,     M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Imp, M::ImmM,    M::Imp, M::Imp, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // C
		M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::Stk,     M::DirIdxX, M::DirIdxX, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Stk, M::Imp, M::AbsIndLng,  M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX, // D
		M::ImmX,  M::DirIdxIndX, M::Imm8,     M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Imp, M::ImmM,    M::Imp, M::Imp, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // E
		M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::Stk,     M::DirIdxX, M::DirIdxX, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Stk, M::Imp, M::AbsIdxXInd, M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX  // F
	};

	memcpy(_opTable, opTable, sizeof(opTable));
	memcpy(_addrMode, addrMode, sizeof(addrMode));

	_memoryManager = memoryManager;
	_state = {};
	_state.PC = ReadDataWord(Cpu::ResetVector);
	_state.SP = 0x1FF;
	_state.EmulationMode = true;
	SetFlags(ProcFlags::MemoryMode8);
	SetFlags(ProcFlags::IndexMode8);
}

Cpu::~Cpu()
{
}

void Cpu::Reset()
{
}

void Cpu::Exec()
{
	uint8_t opCode = GetOpCode();
	_instAddrMode = _addrMode[opCode];
	_operand = FetchEffectiveAddress();
	(this->*_opTable[opCode])();

	opCount++;
}

uint32_t Cpu::GetBank()
{
	return _state.DBR << 16;
}

uint32_t Cpu::GetProgramAddress()
{
	return (_state.K << 16) | _state.PC;
}

uint32_t Cpu::GetDataAddress(uint16_t addr)
{
	return (_state.DBR << 16) | addr;
}

uint8_t Cpu::GetOpCode()
{
	uint8_t opCode = ReadCode(_state.PC, MemoryOperationType::ExecOpCode);
	_state.PC++;
	return opCode;
}

void Cpu::DummyRead()
{
	ReadCode(_state.PC, MemoryOperationType::DummyRead);
}

uint8_t Cpu::ReadOperandByte()
{
	return ReadCode(_state.PC++, MemoryOperationType::ExecOperand);
}

uint16_t Cpu::ReadOperandWord()
{
	uint8_t lsb = ReadOperandByte();
	uint8_t msb = ReadOperandByte();
	return (msb << 8) | lsb;
}

uint32_t Cpu::ReadOperandLong()
{
	uint8_t b1 = ReadOperandByte();
	uint8_t b2 = ReadOperandByte();
	uint8_t b3 = ReadOperandByte();
	return (b3 << 16) | (b2 << 8) | b1;
}

uint8_t Cpu::ReadCode(uint16_t addr, MemoryOperationType type)
{
	return _memoryManager->Read((_state.K << 16) | addr, type);
}

uint8_t Cpu::ReadData(uint32_t addr, MemoryOperationType type)
{
	return _memoryManager->Read(addr, type);
}

uint16_t Cpu::ReadDataWord(uint32_t addr, MemoryOperationType type)
{
	uint8_t lsb = ReadData(addr);
	uint8_t msb = ReadData(addr + 1);
	return (msb << 8) | lsb;
}

uint32_t Cpu::ReadDataLong(uint32_t addr, MemoryOperationType type)
{
	uint8_t b1 = ReadData(addr);
	uint8_t b2 = ReadData(addr + 1);
	uint8_t b3 = ReadData(addr + 2);
	return (b3 << 16) | (b2 << 8) | b1;
}

void Cpu::Write(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	_memoryManager->Write(addr, value, type);
}

void Cpu::WriteWord(uint32_t addr, uint16_t value, MemoryOperationType type)
{
	Write(addr, (uint8_t)value);
	Write(addr + 1, (uint8_t)(value >> 8));
}

uint8_t Cpu::GetByteValue()
{
	if(_instAddrMode <= AddrMode::ImmM) {
		return (uint8_t)_operand;
	} else {
		return ReadData(_operand);
	}
}

uint16_t Cpu::GetWordValue()
{
	if(_instAddrMode <= AddrMode::ImmM) {
		return (uint16_t)_operand;
	} else {
		return ReadDataWord(_operand);
	}
}

void Cpu::PushByte(uint8_t value)
{
	if(_state.EmulationMode) {
		_state.SP = 0x100 | (_state.SP & 0xFF);
		Write(_state.SP, value);
		_state.SP = 0x100 | ((_state.SP - 1) & 0xFF);
	} else {
		Write(_state.SP, value);
		_state.SP--;
	}
}

uint8_t Cpu::PopByte()
{
	if(_state.EmulationMode) {
		_state.SP = 0x100 | ((_state.SP + 1) & 0xFF);
	} else {
		_state.SP++;
	}
	return ReadData(_state.SP);
}

void Cpu::PushWord(uint16_t value)
{
	PushByte(value >> 8);
	PushByte((uint8_t)value);
}

uint16_t Cpu::PopWord()
{
	uint8_t lo = PopByte();
	uint8_t hi = PopByte();
	return lo | hi << 8;
}

uint16_t Cpu::GetDirectAddress(uint8_t baseAddress, uint16_t offset, bool allowEmulationMode)
{
	if(allowEmulationMode && _state.EmulationMode && (_state.D & 0xFF) == 0) {
		//TODO: Check if new instruction or not (PEI)
		return (uint16_t)((_state.D & 0xFF00) | ((baseAddress + offset) & 0xFF));
	} else {
		return (uint16_t)(_state.D + baseAddress + offset);
	}
}

uint32_t Cpu::FetchEffectiveAddress()
{
	switch(_instAddrMode) {
		case AddrMode::Abs: return GetBank() | ReadOperandWord();
		case AddrMode::AbsIdxXInd: return ReadDataWord((_state.K << 16) | ReadOperandWord()); //JMP/JSR
		case AddrMode::AbsIdxX: return (GetBank() | ReadOperandWord()) + _state.X;
		case AddrMode::AbsIdxY: return (GetBank() | ReadOperandWord()) + _state.Y;
		case AddrMode::AbsInd: return ReadDataWord(ReadOperandWord()); //JMP only
		case AddrMode::AbsIndLng: return ReadDataLong(ReadOperandLong()); //JML only
		
		case AddrMode::AbsLngIdxX: return ReadOperandLong() + _state.X;		
		case AddrMode::AbsLng: return ReadOperandLong();

		case AddrMode::Acc: DummyRead(); return 0;

		case AddrMode::BlkMov: return ReadOperandWord();

		case AddrMode::DirIdxIndX: {
			uint8_t operand = ReadOperandByte();
			uint8_t lsb = ReadData(GetDirectAddress(operand, _state.X));
			uint8_t msb = ReadData(GetDirectAddress(operand, _state.X + 1));
			return GetBank() | (msb << 8) | lsb;
		}

		case AddrMode::DirIdxX: return GetDirectAddress(ReadOperandByte(), _state.X);
		case AddrMode::DirIdxY: return GetDirectAddress(ReadOperandByte(), _state.Y);
		
		case AddrMode::DirIndIdxY:{
			uint8_t operand = ReadOperandByte();
			uint8_t lsb = ReadData(GetDirectAddress(operand));
			uint8_t msb = ReadData(GetDirectAddress(operand, 1));
			return (GetBank() | (msb << 8) | lsb) + _state.Y;
		}

		case AddrMode::DirIndLngIdxY: {
			uint8_t operand = ReadOperandByte();
			uint8_t b1 = ReadData(GetDirectAddress(operand));
			uint8_t b2 = ReadData(GetDirectAddress(operand, 1));
			uint8_t b3 = ReadData(GetDirectAddress(operand, 2));
			return ((b3 << 16) | (b2 << 8) | b1) + _state.Y;
		}		

		case AddrMode::DirIndLng: {
			uint8_t operand = ReadOperandByte();
			uint8_t b1 = ReadData(GetDirectAddress(operand));
			uint8_t b2 = ReadData(GetDirectAddress(operand, 1));
			uint8_t b3 = ReadData(GetDirectAddress(operand, 2));
			return (b3 << 16) | (b2 << 8) | b1;
		}

		case AddrMode::DirInd: {
			uint8_t operand = ReadOperandByte();
			uint8_t lsb = ReadData(GetDirectAddress(operand));
			uint8_t msb = ReadData(GetDirectAddress(operand, 1));
			return GetBank() | (msb << 8) | lsb;
		}		

		case AddrMode::Dir: return GetDirectAddress(ReadOperandByte());

		case AddrMode::Imm8: return ReadOperandByte();
		case AddrMode::ImmX: return CheckFlag(ProcFlags::IndexMode8) ? ReadOperandByte() : ReadOperandWord();
		case AddrMode::ImmM: return CheckFlag(ProcFlags::MemoryMode8) ? ReadOperandByte() : ReadOperandWord();
		
		case AddrMode::Imp: DummyRead(); return 0;

		case AddrMode::RelLng: return ReadOperandWord();
		case AddrMode::Rel: return ReadOperandByte();

		case AddrMode::Stk: return _state.SP;
		case AddrMode::StkRel: return (uint16_t)(ReadOperandByte() + _state.SP);

		case AddrMode::StkRelIndIdxY: {
			uint16_t addr = (uint16_t)(ReadOperandByte() + _state.SP);
			return (GetBank() | addr) + _state.Y;
		}
	}

	throw new std::runtime_error("Unreacheable code");
}

void Cpu::SetRegister(uint8_t &reg, uint8_t value)
{
	SetZeroNegativeFlags(value);
	reg = value;
}

void Cpu::SetRegister(uint16_t &reg, uint16_t value, bool eightBitMode)
{
	if(eightBitMode) {
		SetZeroNegativeFlags((uint8_t)value);
		reg = (reg & 0xFF00) | (uint8_t)value;
	} else {
		SetZeroNegativeFlags(value);
		reg = value;
	}
}

void Cpu::SetZeroNegativeFlags(uint16_t value)
{
	ClearFlags(ProcFlags::Zero | ProcFlags::Negative);
	if(value == 0) {
		SetFlags(ProcFlags::Zero);
	} else if(value & 0x8000) {
		SetFlags(ProcFlags::Negative);
	}
}

void Cpu::SetZeroNegativeFlags(uint8_t value)
{
	ClearFlags(ProcFlags::Zero | ProcFlags::Negative);
	if(value == 0) {
		SetFlags(ProcFlags::Zero);
	} else if(value & 0x80) {
		SetFlags(ProcFlags::Negative);
	}
}

void Cpu::ClearFlags(uint8_t flags)
{
	_state.PS &= ~flags;
}

void Cpu::SetFlags(uint8_t flags)
{
	_state.PS |= flags;
}

bool Cpu::CheckFlag(uint8_t flag)
{
	return (_state.PS & flag) == flag;
}

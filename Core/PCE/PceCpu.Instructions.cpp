#include "pch.h"
#include "PceCpu.h"
#include "PCE/PceMemoryManager.h"

void PceCpu::WriteMemoryModeValue(uint8_t value)
{
	ClearFlags(PceCpuFlags::Zero | PceCpuFlags::Negative);
	SetZeroNegativeFlags(value);
	MemoryWrite(PceCpu::ZeroPage + X(), value);
}

void PceCpu::AND()
{
	if(_memoryFlag) {
		DummyRead();
		uint8_t value = MemoryRead(PceCpu::ZeroPage + X());
		WriteMemoryModeValue(value & GetOperandValue());
	} else {
		SetA(A() & GetOperandValue());
	}
}

void PceCpu::EOR()
{
	if(_memoryFlag) {
		DummyRead();
		uint8_t value = MemoryRead(PceCpu::ZeroPage + X());
		WriteMemoryModeValue(value ^ GetOperandValue());
	} else {
		SetA(A() ^ GetOperandValue());
	}
}

void PceCpu::ORA()
{
	if(_memoryFlag) {
		DummyRead();
		uint8_t value = MemoryRead(PceCpu::ZeroPage + X());
		WriteMemoryModeValue(value | GetOperandValue());
	} else {
		SetA(A() | GetOperandValue());
	}
}

void PceCpu::ADD(uint8_t value)
{
	uint8_t source;
	if(_memoryFlag) {
		DummyRead();
		source = MemoryRead(PceCpu::ZeroPage + X());
	} else {
		source = A();
	}

	uint16_t result;
	if(CheckFlag(PceCpuFlags::Decimal)) {
		result = (uint16_t)(source & 0x0F) + (uint16_t)(value & 0x0F) + (_state.PS & PceCpuFlags::Carry);
		if(result > 0x09) result += 0x06;
		result = (uint16_t)(source & 0xF0) + (uint16_t)(value & 0xF0) + (result > 0x0F ? 0x10 : 0) + (result & 0x0F);
		if(result > 0x9F) result += 0x60;
	} else {
		result = (uint16_t)source + (uint16_t)value + (CheckFlag(PceCpuFlags::Carry) ? PceCpuFlags::Carry : 0x00);
		
		if(~(source ^ value) & (source ^ result) & 0x80) {
			SetFlags(PceCpuFlags::Overflow);
		} else {
			ClearFlags(PceCpuFlags::Overflow);
		}
	}

	ClearFlags(PceCpuFlags::Carry | PceCpuFlags::Negative | PceCpuFlags::Zero);
	SetZeroNegativeFlags((uint8_t)result);

	if(result > 0xFF) {
		SetFlags(PceCpuFlags::Carry);
	}

	if(_memoryFlag) {
		WriteMemoryModeValue((uint8_t)result);
	} else {
		SetA((uint8_t)result);
	}
}

void PceCpu::SUB(uint8_t value)
{
	int32_t result;
	if(CheckFlag(PceCpuFlags::Decimal)) {
		result = (A() & 0x0F) + (value & 0x0F) + (_state.PS & PceCpuFlags::Carry);
		if(result <= 0x0F) result -= 0x06;
		result = (A() & 0xF0) + (value & 0xF0) + (result > 0x0F ? 0x10 : 0) + (result & 0x0F);
		if(result <= 0xFF) result -= 0x60;
	} else {
		result = (uint16_t)A() + (uint16_t)value + (_state.PS & PceCpuFlags::Carry);

		if(~(A() ^ value) & (A() ^ result) & 0x80) {
			SetFlags(PceCpuFlags::Overflow);
		} else {
			ClearFlags(PceCpuFlags::Overflow);
		}
	}

	ClearFlags(PceCpuFlags::Carry | PceCpuFlags::Negative | PceCpuFlags::Zero);
	SetZeroNegativeFlags((uint8_t)result);

	if(result > 0xFF) {
		SetFlags(PceCpuFlags::Carry);
	}

	SetA((uint8_t)result);
}

void PceCpu::ADC()
{
	ADD(GetOperandValue());
}

void PceCpu::SBC()
{
	SUB(~GetOperandValue());
}

void PceCpu::CMP(uint8_t reg, uint8_t value)
{
	ClearFlags(PceCpuFlags::Carry | PceCpuFlags::Negative | PceCpuFlags::Zero);

	auto result = reg - value;

	if(reg >= value) {
		SetFlags(PceCpuFlags::Carry);
	}
	if(reg == value) {
		SetFlags(PceCpuFlags::Zero);
	}
	if((result & 0x80) == 0x80) {
		SetFlags(PceCpuFlags::Negative);
	}
}

void PceCpu::CPA() { CMP(A(), GetOperandValue()); }
void PceCpu::CPX() { CMP(X(), GetOperandValue()); }
void PceCpu::CPY() { CMP(Y(), GetOperandValue()); }

void PceCpu::INC()
{
	uint16_t addr = GetOperand();
	ClearFlags(PceCpuFlags::Negative | PceCpuFlags::Zero);
	uint8_t value = MemoryRead(addr);

	DummyRead();

	value++;
	SetZeroNegativeFlags(value);
	MemoryWrite(addr, value);
}

void PceCpu::DEC()
{
	uint16_t addr = GetOperand();
	ClearFlags(PceCpuFlags::Negative | PceCpuFlags::Zero);
	uint8_t value = MemoryRead(addr);

	DummyRead();

	value--;
	SetZeroNegativeFlags(value);
	MemoryWrite(addr, value);
}

uint8_t PceCpu::ASL(uint8_t value)
{
	ClearFlags(PceCpuFlags::Carry | PceCpuFlags::Negative | PceCpuFlags::Zero);
	if(value & 0x80) {
		SetFlags(PceCpuFlags::Carry);
	}

	uint8_t result = value << 1;
	SetZeroNegativeFlags(result);
	return result;
}

uint8_t PceCpu::LSR(uint8_t value)
{
	ClearFlags(PceCpuFlags::Carry | PceCpuFlags::Negative | PceCpuFlags::Zero);
	if(value & 0x01) {
		SetFlags(PceCpuFlags::Carry);
	}

	uint8_t result = value >> 1;
	SetZeroNegativeFlags(result);
	return result;
}

uint8_t PceCpu::ROL(uint8_t value)
{
	bool carryFlag = CheckFlag(PceCpuFlags::Carry);
	ClearFlags(PceCpuFlags::Carry | PceCpuFlags::Negative | PceCpuFlags::Zero);

	if(value & 0x80) {
		SetFlags(PceCpuFlags::Carry);
	}

	uint8_t result = (value << 1 | (carryFlag ? 0x01 : 0x00));
	SetZeroNegativeFlags(result);
	return result;
}

uint8_t PceCpu::ROR(uint8_t value)
{
	bool carryFlag = CheckFlag(PceCpuFlags::Carry);
	ClearFlags(PceCpuFlags::Carry | PceCpuFlags::Negative | PceCpuFlags::Zero);
	if(value & 0x01) {
		SetFlags(PceCpuFlags::Carry);
	}

	uint8_t result = (value >> 1 | (carryFlag ? 0x80 : 0x00));
	SetZeroNegativeFlags(result);
	return result;
}

void PceCpu::ASLAddr()
{
	uint16_t addr = GetOperand();
	uint8_t value = MemoryRead(addr);
	DummyRead();
	MemoryWrite(addr, ASL(value));
}

void PceCpu::LSRAddr()
{
	uint16_t addr = GetOperand();
	uint8_t value = MemoryRead(addr);
	DummyRead();
	MemoryWrite(addr, LSR(value));
}

void PceCpu::ROLAddr()
{
	uint16_t addr = GetOperand();
	uint8_t value = MemoryRead(addr);
	DummyRead();
	MemoryWrite(addr, ROL(value));
}

void PceCpu::RORAddr()
{
	uint16_t addr = GetOperand();
	uint8_t value = MemoryRead(addr);
	DummyRead();
	MemoryWrite(addr, ROR(value));
}

void PceCpu::BranchRelative(bool branch)
{
	int8_t offset = (int8_t)GetOperand();
	if(branch) {
		DummyRead();
		DummyRead();

		SetPC(PC() + offset);
	}
}

void PceCpu::BIT()
{
	uint8_t value = GetOperandValue();
	ClearFlags(PceCpuFlags::Zero | PceCpuFlags::Overflow | PceCpuFlags::Negative);
	if((A() & value) == 0) {
		SetFlags(PceCpuFlags::Zero);
	}
	if(value & 0x40) {
		SetFlags(PceCpuFlags::Overflow);
	}
	if(value & 0x80) {
		SetFlags(PceCpuFlags::Negative);
	}
}

void PceCpu::TSB()
{
	uint8_t value = MemoryRead(_operand);

	ClearFlags(PceCpuFlags::Zero | PceCpuFlags::Overflow | PceCpuFlags::Negative);
	if(value & 0x40) {
		SetFlags(PceCpuFlags::Overflow);
	}
	if(value & 0x80) {
		SetFlags(PceCpuFlags::Negative);
	}

	value |= A();
	if(value == 0) {
		SetFlags(PceCpuFlags::Zero);
	}

	DummyRead();
	MemoryWrite(_operand, value);
}

void PceCpu::TRB()
{
	uint8_t value = MemoryRead(_operand);

	ClearFlags(PceCpuFlags::Zero | PceCpuFlags::Overflow | PceCpuFlags::Negative);
	if(value & 0x40) {
		SetFlags(PceCpuFlags::Overflow);
	}
	if(value & 0x80) {
		SetFlags(PceCpuFlags::Negative);
	}

	value &= ~A();
	if(value == 0) {
		SetFlags(PceCpuFlags::Zero);
	}

	DummyRead();
	MemoryWrite(_operand, value);
}

void PceCpu::TST()
{
	DummyRead();
	DummyRead();

	uint8_t value = MemoryRead(_operand2);

	ClearFlags(PceCpuFlags::Zero | PceCpuFlags::Overflow | PceCpuFlags::Negative);
	if(value & 0x40) {
		SetFlags(PceCpuFlags::Overflow);
	}
	if(value & 0x80) {
		SetFlags(PceCpuFlags::Negative);
	}

	value &= _operand;

	if(value == 0) {
		SetFlags(PceCpuFlags::Zero);
	}
}

void PceCpu::CSL()
{
	//CSH/CSL take 3 cycles
	DummyRead();
#ifndef DUMMYCPU
	_memoryManager->SetSpeed(true);
#endif
}

void PceCpu::CSH()
{
	//CSH/CSL take 3 cycles
	DummyRead();
#ifndef DUMMYCPU
	_memoryManager->SetSpeed(false);
#endif
}

//OP Codes
void PceCpu::LDA() { SetA(GetOperandValue()); }
void PceCpu::LDX() { SetX(GetOperandValue()); }
void PceCpu::LDY() { SetY(GetOperandValue()); }

void PceCpu::STA() { MemoryWrite(GetOperand(), A()); }
void PceCpu::STX() { MemoryWrite(GetOperand(), X()); }
void PceCpu::STY() { MemoryWrite(GetOperand(), Y()); }
void PceCpu::STZ() { MemoryWrite(GetOperand(), 0); }

void PceCpu::TAX() { SetX(A()); }
void PceCpu::TAY() { SetY(A()); }
void PceCpu::TSX() { SetX(SP()); }
void PceCpu::TXA() { SetA(X()); }
void PceCpu::TXS() { SetSP(X()); }
void PceCpu::TYA() { SetA(Y()); }

void PceCpu::PHA() { Push(A()); }
void PceCpu::PHX() { Push(X()); }
void PceCpu::PHY() { Push(Y()); }

void PceCpu::PHP()
{
	uint8_t flags = PS() | PceCpuFlags::Break;
	Push((uint8_t)flags);
}

void PceCpu::PLA()
{
	DummyRead();
	SetA(Pop());
}

void PceCpu::PLP()
{
	DummyRead();
	SetPS(Pop());
}

void PceCpu::PLX()
{
	DummyRead();
	SetX(Pop());
}

void PceCpu::PLY()
{
	DummyRead();
	SetY(Pop());
}

void PceCpu::INC_Acc() { SetA(A() + 1); }
void PceCpu::INX() { SetX(X() + 1); }
void PceCpu::INY() { SetY(Y() + 1); }

void PceCpu::DEC_Acc() { SetA(A() - 1); }
void PceCpu::DEX() { SetX(X() - 1); }
void PceCpu::DEY() { SetY(Y() - 1); }

void PceCpu::ASL_Acc() { SetA(ASL(A())); }
void PceCpu::ASL_Memory() { ASLAddr(); }

void PceCpu::LSR_Acc() { SetA(LSR(A())); }
void PceCpu::LSR_Memory() { LSRAddr(); }

void PceCpu::ROL_Acc() { SetA(ROL(A())); }
void PceCpu::ROL_Memory() { ROLAddr(); }

void PceCpu::ROR_Acc() { SetA(ROR(A())); }
void PceCpu::ROR_Memory() { RORAddr(); }

void PceCpu::JMP_Abs()
{
	SetPC(GetOperand());
}

void PceCpu::JMP_Ind()
{
	//Unlike the 6502 CPU, this CPU works normally when crossing a page during indirect jumps
	DummyRead();
	DummyRead();
	SetPC(MemoryReadWord(_operand));
}

void PceCpu::JMP_AbsX()
{
	DummyRead();
	SetPC(MemoryReadWord(_operand));
}

void PceCpu::JSR()
{
	uint16_t addr = GetOperand();
	DummyRead();
	Push((uint16_t)(PC() - 1));
	SetPC(addr);
}

void PceCpu::RTS()
{
	DummyRead();
	uint16_t addr = PopWord();
	DummyRead();
	DummyRead();
	SetPC(addr + 1);
}

void PceCpu::BCC()
{
	BranchRelative(!CheckFlag(PceCpuFlags::Carry));
}

void PceCpu::BCS()
{
	BranchRelative(CheckFlag(PceCpuFlags::Carry));
}

void PceCpu::BEQ()
{
	BranchRelative(CheckFlag(PceCpuFlags::Zero));
}

void PceCpu::BMI()
{
	BranchRelative(CheckFlag(PceCpuFlags::Negative));
}

void PceCpu::BNE()
{
	BranchRelative(!CheckFlag(PceCpuFlags::Zero));
}

void PceCpu::BPL()
{
	BranchRelative(!CheckFlag(PceCpuFlags::Negative));
}

void PceCpu::BVC()
{
	BranchRelative(!CheckFlag(PceCpuFlags::Overflow));
}

void PceCpu::BVS()
{
	BranchRelative(CheckFlag(PceCpuFlags::Overflow));
}

void PceCpu::CLC() { ClearFlags(PceCpuFlags::Carry); }
void PceCpu::CLD() { ClearFlags(PceCpuFlags::Decimal); }
void PceCpu::CLI() { ClearFlags(PceCpuFlags::Interrupt); }
void PceCpu::CLV() { ClearFlags(PceCpuFlags::Overflow); }
void PceCpu::SEC() { SetFlags(PceCpuFlags::Carry); }
void PceCpu::SED() { SetFlags(PceCpuFlags::Decimal); }
void PceCpu::SEI() { SetFlags(PceCpuFlags::Interrupt); }
void PceCpu::SET() { SetFlags(PceCpuFlags::Memory); }

void PceCpu::BRK()
{
	ProcessIrq(true);
}

void PceCpu::RTI()
{
	DummyRead();
	SetPS(Pop());
	DummyRead();
	SetPC(PopWord());
}

void PceCpu::NOP()
{
	//NOP
}

void PceCpu::BSR()
{
	int8_t relAddr = (int8_t)GetOperand();
	DummyRead();
	DummyRead();
	Push((uint16_t)(PC() - 1));
	DummyRead();
	DummyRead();
	SetPC(PC() + relAddr);
}

void PceCpu::BRA()
{
	BranchRelative(true);
}

void PceCpu::SXY()
{
	DummyRead();
	std::swap(_state.X, _state.Y);
}

void PceCpu::SAX()
{
	DummyRead();
	std::swap(_state.A, _state.X);
}

void PceCpu::SAY()
{
	DummyRead();
	std::swap(_state.A, _state.Y);
}

void PceCpu::CLA()
{
	_state.A = 0;
}

void PceCpu::CLX()
{
	_state.X = 0;
}

void PceCpu::CLY()
{
	_state.Y = 0;
}

void PceCpu::ST0()
{
	DummyRead(); //1 dummy read
#ifndef DUMMYCPU
	ProcessCpuCycle(); //1 write cycle
	_memoryManager->WriteVdc(0, _operand);
#endif
}

void PceCpu::ST1()
{
	DummyRead(); //1 dummy read
#ifndef DUMMYCPU
	ProcessCpuCycle(); //1 write cycle
	_memoryManager->WriteVdc(2, _operand);
#endif
}

void PceCpu::ST2()
{
	DummyRead(); //1 dummy read
#ifndef DUMMYCPU
	ProcessCpuCycle(); //1 write cycle
	_memoryManager->WriteVdc(3, _operand);
#endif
}

void PceCpu::TMA()
{
	DummyRead();
	DummyRead();
#ifndef DUMMYCPU
	SetA(_memoryManager->GetMprValue(GetOperand()));
#endif
}

void PceCpu::TAM()
{
	DummyRead();
	DummyRead();
	DummyRead();
#ifndef DUMMYCPU
	_memoryManager->SetMprValue(GetOperand(), A());
#endif
}

void PceCpu::StartBlockTransfer()
{
	DummyRead();
	Push(Y());
	DummyRead();
	Push(X());
	DummyRead();
	Push(A());
	DummyRead();
}

void PceCpu::EndBlockTransfer()
{
	_state.A = Pop();
	_state.X = Pop();
	_state.Y = Pop();
}

void PceCpu::TAI()
{
	StartBlockTransfer();

	uint16_t src = _operand;
	uint16_t dst = _operand2;
	uint16_t length = _operand3;

	uint32_t count = 0;
	do {
		DummyRead();
		uint8_t value = MemoryRead(src);
		DummyRead();

		DummyRead();
		MemoryWrite(dst, value);
		DummyRead();

		src += (count & 0x01) ? -1 : 1;
		dst++;

		count++;
		length--;
	} while(length);

	EndBlockTransfer();
}

void PceCpu::TDD()
{
	StartBlockTransfer();

	uint16_t src = _operand;
	uint16_t dst = _operand2;
	uint16_t length = _operand3;

	do {
		DummyRead();
		uint8_t value = MemoryRead(src);
		DummyRead();

		DummyRead();
		MemoryWrite(dst, value);
		DummyRead();

		src--;
		dst--;

		length--;
	} while(length);

	EndBlockTransfer();
}

void PceCpu::TIA()
{
	StartBlockTransfer();

	uint16_t src = _operand;
	uint16_t dst = _operand2;
	uint16_t length = _operand3;

	uint32_t count = 0;
	do {
		DummyRead();
		uint8_t value = MemoryRead(src);
		DummyRead();

		DummyRead();
		MemoryWrite(dst, value);
		DummyRead();

		src++;
		dst += (count & 0x01) ? -1 : 1;

		count++;
		length--;
	} while(length);

	EndBlockTransfer();
}

void PceCpu::TII()
{
	StartBlockTransfer();

	uint16_t src = _operand;
	uint16_t dst = _operand2;
	uint16_t length = _operand3;

	do {
		DummyRead();
		uint8_t value = MemoryRead(src);
		DummyRead();

		DummyRead();
		MemoryWrite(dst, value);
		DummyRead();

		src++;
		dst++;

		length--;
	} while(length);

	EndBlockTransfer();
}

void PceCpu::TIN()
{
	StartBlockTransfer();

	uint16_t src = _operand;
	uint16_t dst = _operand2;
	uint16_t length = _operand3;

	do {
		DummyRead();
		uint8_t value = MemoryRead(src);
		DummyRead();

		DummyRead();
		MemoryWrite(dst, value);
		DummyRead();

		src++;

		length--;
	} while(length);

	EndBlockTransfer();
}

void PceCpu::BBR(uint8_t bit)
{
	DummyRead();
	DummyRead();

	uint8_t value = MemoryRead(PceCpu::ZeroPage + (_operand & 0xFF));
	if((value & (1 << bit)) == 0) {
		DummyRead();
		DummyRead();
		SetPC(PC() + (int8_t)(_operand >> 8));
	}
}

void PceCpu::BBS(uint8_t bit)
{
	DummyRead();
	DummyRead();

	uint8_t value = MemoryRead(PceCpu::ZeroPage + (_operand & 0xFF));
	if((value & (1 << bit)) != 0) {
		DummyRead();
		DummyRead();
		SetPC(PC() + (int8_t)(_operand >> 8));
	}
}

void PceCpu::RMB(uint8_t bit)
{
	uint8_t value = MemoryRead(_operand);
	value &= ~(1 << bit);
	DummyRead();
	DummyRead();
	MemoryWrite(_operand, value);
}

void PceCpu::SMB(uint8_t bit)
{
	uint8_t value = MemoryRead(_operand);
	value |= (1 << bit);
	DummyRead();
	DummyRead();
	MemoryWrite(_operand, value);
}

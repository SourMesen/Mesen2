#include "stdafx.h"
#include "PceCpu.h"
#include "Shared/Emulator.h"
#include "PCE/PceMemoryManager.h"
#include "PCE/PceConsole.h"
#include "Utilities/RandomHelper.h"

typedef PceCpu C;
PceCpu::Func const PceCpu::_opTable[] = {
	//	0		1			2			3			4			5			6						7				8			9			A					B			C					D			E						F
	&C::BRK,	&C::ORA,	&C::SXY,	&C::ST0,	&C::TSB,	&C::ORA,	&C::ASL_Memory,	&C::RMB0,	&C::PHP,	&C::ORA,	&C::ASL_Acc,	&C::NOP,	&C::TSB,			&C::ORA,	&C::ASL_Memory,	&C::BBR0, //0
	&C::BPL,	&C::ORA,	&C::ORA,	&C::ST1,	&C::TRB,	&C::ORA,	&C::ASL_Memory,	&C::RMB1,	&C::CLC,	&C::ORA,	&C::INC_Acc,	&C::NOP,	&C::TRB,			&C::ORA,	&C::ASL_Memory,	&C::BBR1, //1
	&C::JSR,	&C::AND,	&C::SAX,	&C::ST2,	&C::BIT,	&C::AND,	&C::ROL_Memory,	&C::RMB2,	&C::PLP,	&C::AND,	&C::ROL_Acc,	&C::NOP,	&C::BIT,			&C::AND,	&C::ROL_Memory,	&C::BBR2, //2
	&C::BMI,	&C::AND,	&C::AND,	&C::NOP,	&C::BIT,	&C::AND,	&C::ROL_Memory,	&C::RMB3,	&C::SEC,	&C::AND,	&C::DEC_Acc,	&C::NOP,	&C::BIT,			&C::AND,	&C::ROL_Memory,	&C::BBR3, //3
	&C::RTI,	&C::EOR,	&C::SAY,	&C::TMA,	&C::BSR,	&C::EOR,	&C::LSR_Memory,	&C::RMB4,	&C::PHA,	&C::EOR,	&C::LSR_Acc,	&C::NOP,	&C::JMP_Abs,	&C::EOR,	&C::LSR_Memory,	&C::BBR4, //4
	&C::BVC,	&C::EOR,	&C::EOR,	&C::TAM,	&C::CSL,	&C::EOR,	&C::LSR_Memory,	&C::RMB5,	&C::CLI,	&C::EOR,	&C::PHY,			&C::NOP,	&C::NOP,			&C::EOR,	&C::LSR_Memory,	&C::BBR5, //5
	&C::RTS,	&C::ADC,	&C::CLA,	&C::NOP,	&C::STZ,	&C::ADC,	&C::ROR_Memory,	&C::RMB6,	&C::PLA,	&C::ADC,	&C::ROR_Acc,	&C::NOP,	&C::JMP_Ind,	&C::ADC,	&C::ROR_Memory,	&C::BBR6, //6
	&C::BVS,	&C::ADC,	&C::ADC,	&C::TII,	&C::STZ,	&C::ADC,	&C::ROR_Memory,	&C::RMB7,	&C::SEI,	&C::ADC,	&C::PLY,			&C::NOP,	&C::JMP_AbsX,	&C::ADC,	&C::ROR_Memory,	&C::BBR7, //7
	&C::BRA,	&C::STA,	&C::CLX,	&C::TST,	&C::STY,	&C::STA,	&C::STX,				&C::SMB0,	&C::DEY,	&C::BIT,	&C::TXA,			&C::NOP,	&C::STY,			&C::STA,	&C::STX,				&C::BBS0, //8
	&C::BCC,	&C::STA,	&C::STA,	&C::TST,	&C::STY,	&C::STA,	&C::STX,				&C::SMB1,	&C::TYA,	&C::STA,	&C::TXS,			&C::NOP,	&C::STZ,			&C::STA,	&C::STZ,				&C::BBS1, //9
	&C::LDY,	&C::LDA,	&C::LDX,	&C::TST,	&C::LDY,	&C::LDA,	&C::LDX,				&C::SMB2,	&C::TAY,	&C::LDA,	&C::TAX,			&C::NOP,	&C::LDY,			&C::LDA,	&C::LDX,				&C::BBS2, //A
	&C::BCS,	&C::LDA,	&C::LDA,	&C::TST,	&C::LDY,	&C::LDA,	&C::LDX,				&C::SMB3,	&C::CLV,	&C::LDA,	&C::TSX,			&C::NOP,	&C::LDY,			&C::LDA,	&C::LDX,				&C::BBS3, //B
	&C::CPY,	&C::CPA,	&C::CLY,	&C::TDD,	&C::CPY,	&C::CPA,	&C::DEC,				&C::SMB4,	&C::INY,	&C::CPA,	&C::DEX,			&C::NOP,	&C::CPY,			&C::CPA,	&C::DEC,				&C::BBS4, //C
	&C::BNE,	&C::CPA,	&C::CPA,	&C::TIN,	&C::CSH,	&C::CPA,	&C::DEC,				&C::SMB5,	&C::CLD,	&C::CPA,	&C::PHX,			&C::NOP,	&C::NOP,			&C::CPA,	&C::DEC,				&C::BBS5, //D
	&C::CPX,	&C::SBC,	&C::NOP,	&C::TIA,	&C::CPX,	&C::SBC,	&C::INC,				&C::SMB6,	&C::INX,	&C::SBC,	&C::NOP,			&C::NOP,	&C::CPX,			&C::SBC,	&C::INC,				&C::BBS6, //E
	&C::BEQ,	&C::SBC,	&C::SBC,	&C::TAI,	&C::SET,	&C::SBC,	&C::INC,				&C::SMB7,	&C::SED,	&C::SBC,	&C::PLX,			&C::NOP,	&C::NOP,			&C::SBC,	&C::INC,				&C::BBS7  //F
};

typedef PceAddrMode M;
PceAddrMode const PceCpu::_addrMode[] = {
	//	0			1				2			3				4				5				6				7				8			9			A			B			C			D			E			F
	M::Imm,	M::IndX,		M::Imp,	M::Imm,		M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Acc,	M::Imp,	M::Abs,	M::Abs,	M::Abs,	M::ZeroRel,	//0
	M::Rel,	M::IndY,		M::ZInd,	M::Imm,		M::Zero,		M::ZeroX,	M::ZeroX,	M::Zero,		M::Imp,	M::AbsY,	M::Imp,	M::Imp,	M::Abs,	M::AbsX,	M::AbsX,	M::ZeroRel,//1
	M::Abs,	M::IndX,		M::Imp,	M::Imm,		M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Acc,	M::Imp,	M::Abs,	M::Abs,	M::Abs,	M::ZeroRel,	//2
	M::Rel,	M::IndY,		M::ZInd,	M::Imp,		M::ZeroX,	M::ZeroX,	M::ZeroX,	M::Zero,		M::Imp,	M::AbsY,	M::Imp,	M::Imp,	M::AbsX,	M::AbsX,	M::AbsX,	M::ZeroRel,//3
	M::Imp,	M::IndX,		M::Imp,	M::Imm,		M::Rel,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Acc,	M::Imp,	M::Abs,	M::Abs,	M::Abs,	M::ZeroRel,	//4
	M::Rel,	M::IndY,		M::ZInd,	M::Imm,		M::Imp,		M::ZeroX,	M::ZeroX,	M::Zero,		M::Imp,	M::AbsY,	M::Imp,	M::Imp,	M::Imp,	M::AbsX,	M::AbsX,	M::ZeroRel,//5
	M::Imp,	M::IndX,		M::Imp,	M::Imp,		M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Acc,	M::Imp,	M::Ind,	M::Abs,	M::Abs,	M::ZeroRel,	//6
	M::Rel,	M::IndY,		M::ZInd,	M::Block,	M::ZeroX,	M::ZeroX,	M::ZeroX,	M::Zero,		M::Imp,	M::AbsY,	M::Imp,	M::Imp,	M::AbsX,	M::AbsX,	M::AbsX,	M::ZeroRel,//7
	M::Rel,	M::IndX,		M::Imp,	M::ImZero,	M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Imp,	M::Imp,	M::Abs,	M::Abs,	M::Abs,	M::ZeroRel,	//8
	M::Rel,	M::IndY,		M::ZInd,	M::ImAbs,	M::ZeroX,	M::ZeroX,	M::ZeroY,	M::Zero,		M::Imp,	M::AbsY,	M::Imp,	M::Imp,	M::Abs,	M::AbsX,	M::AbsX,	M::ZeroRel,//9
	M::Imm,	M::IndX,		M::Imm,	M::ImZeroX,	M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Imp,	M::Imp,	M::Abs,	M::Abs,	M::Abs,	M::ZeroRel,	//A
	M::Rel,	M::IndY,		M::ZInd,	M::ImAbsX,	M::ZeroX,	M::ZeroX,	M::ZeroY,	M::Zero,		M::Imp,	M::AbsY,	M::Imp,	M::Imp,	M::AbsX,	M::AbsX,	M::AbsY,	M::ZeroRel,	//B
	M::Imm,	M::IndX,		M::Imp,	M::Block,	M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Imp,	M::Imp,	M::Abs,	M::Abs,	M::Abs,	M::ZeroRel,	//C
	M::Rel,	M::IndY,		M::ZInd,	M::Block,	M::Imp,		M::ZeroX,	M::ZeroX,	M::Zero,		M::Imp,	M::AbsY,	M::Imp,	M::Imp,	M::Imp,	M::AbsX,	M::AbsX,	M::ZeroRel,//D
	M::Imm,	M::IndX,		M::Imp,	M::Block,	M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Imp,	M::Imp,	M::Abs,	M::Abs,	M::Abs,	M::ZeroRel,	//E
	M::Rel,	M::IndY,		M::ZInd,	M::Block,	M::Imp,		M::ZeroX,	M::ZeroX,	M::Zero,		M::Imp,	M::AbsY,	M::Imp,	M::Imp,	M::Imp,	M::AbsX,	M::AbsX,	M::ZeroRel,//F
};

#ifndef DUMMYCPU
PceCpu::PceCpu(Emulator* emu, PceConsole* console, PceMemoryManager* memoryManager)
{
	_emu = emu;
	_console = console;
	_memoryManager = memoryManager;

	_instAddrMode = PceAddrMode::None;
	_state = {};
	_operand = 0;

	_state.PC = _memoryManager->Read(PceCpu::ResetVector) | _memoryManager->Read(PceCpu::ResetVector + 1) << 8;

	//A, X, Y, S are random on power on
	_state.A = RandomHelper::GetValue(0, 255);
	_state.X = RandomHelper::GetValue(0, 255);
	_state.Y = RandomHelper::GetValue(0, 255);
	_state.SP = RandomHelper::GetValue(0, 255);

	//I is set on power on
	SetFlags(PceCpuFlags::Interrupt);

	//T & M are always cleared on power on
	ClearFlags(PceCpuFlags::Decimal | PceCpuFlags::Memory);

	//Other flags are random
	if(RandomHelper::GetBool()) {
		SetFlags(PceCpuFlags::Zero);
	}
	if(RandomHelper::GetBool()) {
		SetFlags(PceCpuFlags::Negative);
	}
	if(RandomHelper::GetBool()) {
		SetFlags(PceCpuFlags::Overflow);
	}
	if(RandomHelper::GetBool()) {
		SetFlags(PceCpuFlags::Carry);
	}
}
#endif

void PceCpu::Exec()
{
	//TODO delete
	/*uint32_t cycleCounts[256] = {};
	for(int i = 0; i < 256; i++) {
	uint64_t count = _state.CycleCount;

	_instAddrMode = _addrMode[i];
	_operand = FetchOperand();
	(this->*_opTable[i])();

	cycleCounts[i] = _state.CycleCount - count + 1;
	}*/

#ifndef DUMMYCPU
	_emu->ProcessInstruction<CpuType::Pce>();
#endif

	//T flag is reset at the start of each instruction
	_memoryFlag = CheckFlag(PceCpuFlags::Memory);
	ClearFlags(PceCpuFlags::Memory);

	uint8_t opCode = GetOPCode();
	_instAddrMode = _addrMode[opCode];
	_operand = FetchOperand();
	(this->*_opTable[opCode])();

	if(_needIrq && _memoryManager->HasIrqSource((PceIrqSource)0xFF)) {
		ProcessIrq(false);
	}
}

uint16_t PceCpu::FetchOperand()
{
	switch(_instAddrMode) {
		case PceAddrMode::Acc:
		case PceAddrMode::Imp: DummyRead(); return 0;
		case PceAddrMode::Imm:
		case PceAddrMode::Rel: return GetImmediate();
		case PceAddrMode::Zero: _operand = PceCpu::ZeroPage + GetZeroAddr(); DummyRead(); return _operand;
		case PceAddrMode::ZeroX: _operand = PceCpu::ZeroPage + GetZeroXAddr(); DummyRead(); return _operand;
		case PceAddrMode::ZeroY: _operand = PceCpu::ZeroPage + GetZeroYAddr(); DummyRead(); return _operand;
		case PceAddrMode::Ind: return GetIndAddr();
		case PceAddrMode::IndX: return GetIndXAddr();
		case PceAddrMode::IndY: return GetIndYAddr();
		case PceAddrMode::Abs: _operand = GetAbsAddr(); DummyRead(); return _operand;
		case PceAddrMode::AbsX: _operand = GetAbsXAddr(); DummyRead(); return _operand;
		case PceAddrMode::AbsY: _operand = GetAbsYAddr(); DummyRead(); return _operand;

		case PceAddrMode::ZeroRel: return ReadWord();

		case PceAddrMode::Block:
			_operand = ReadWord();
			_operand2 = ReadWord();
			_operand3 = ReadWord();
			return _operand;

		case PceAddrMode::ZInd: return GetIndZeroAddr();

		case PceAddrMode::ImZero:
			_operand = ReadByte();
			_operand2 = PceCpu::ZeroPage + GetZeroAddr();
			DummyRead();
			return _operand;

		case PceAddrMode::ImZeroX:
			_operand = ReadByte();
			_operand2 = PceCpu::ZeroPage + GetZeroXAddr();
			DummyRead();
			return _operand;

		case PceAddrMode::ImAbs:
			_operand = ReadByte();
			_operand2 = GetAbsAddr();
			DummyRead();
			return _operand;

		case PceAddrMode::ImAbsX:
			_operand = ReadByte();
			_operand2 = GetAbsXAddr();
			DummyRead();
			return _operand;

		default:
			break;
	}

	LogDebug("invalid instruction");
	return 0;
}

void PceCpu::SetRegister(uint8_t& reg, uint8_t value)
{
	ClearFlags(PceCpuFlags::Zero | PceCpuFlags::Negative);
	SetZeroNegativeFlags(value);
	reg = value;
}

void PceCpu::Push(uint8_t value)
{
	MemoryWrite(PceCpu::StackPage + SP(), value);
	SetSP(SP() - 1);
}

void PceCpu::Push(uint16_t value)
{
	Push((uint8_t)(value >> 8));
	Push((uint8_t)value);
}

uint8_t PceCpu::Pop()
{
	SetSP(SP() + 1);
	return MemoryRead(PceCpu::StackPage + SP());
}

uint16_t PceCpu::PopWord()
{
	uint8_t lo = Pop();
	uint8_t hi = Pop();

	return lo | hi << 8;
}

uint8_t PceCpu::GetOPCode()
{
	uint8_t opCode = MemoryRead(_state.PC, MemoryOperationType::ExecOpCode);
	_state.PC++;
	return opCode;
}

uint16_t PceCpu::GetOperand()
{
	return _operand;
}

uint8_t PceCpu::GetOperandValue()
{
	if(_instAddrMode >= PceAddrMode::Zero) {
		return MemoryRead(GetOperand());
	} else {
		return (uint8_t)GetOperand();
	}
}

void PceCpu::DummyRead()
{
	//TODO - is this supposed to be an idle cycle, or a dummy read?
	MemoryRead(_state.PC, MemoryOperationType::DummyRead);
}

uint8_t PceCpu::ReadByte()
{
	uint8_t value = MemoryRead(_state.PC, MemoryOperationType::ExecOperand);
	_state.PC++;
	return value;
}

uint16_t PceCpu::ReadWord()
{
	uint8_t low = ReadByte();
	uint8_t high = ReadByte();
	return (high << 8) | low;
}

void PceCpu::ClearFlags(uint8_t flags)
{
	_state.PS &= ~flags;
}

void PceCpu::SetFlags(uint8_t flags)
{
	_state.PS |= flags;
}

bool PceCpu::CheckFlag(uint8_t flag)
{
	return (_state.PS & flag) == flag;
}

void PceCpu::SetZeroNegativeFlags(uint8_t value)
{
	if(value == 0) {
		SetFlags(PceCpuFlags::Zero);
	} else if(value & 0x80) {
		SetFlags(PceCpuFlags::Negative);
	}
}

void PceCpu::ProcessCpuCycle()
{
	_state.CycleCount++;
	_memoryManager->Exec();

	_needIrq = _memoryManager->HasIrqSource((PceIrqSource)0xFF) && !CheckFlag(PceCpuFlags::Interrupt);
}

#ifndef DUMMYCPU
void PceCpu::MemoryWrite(uint16_t addr, uint8_t value, MemoryOperationType operationType)
{
	ProcessCpuCycle();
	_memoryManager->Write(addr, value, operationType);
}

uint8_t PceCpu::MemoryRead(uint16_t addr, MemoryOperationType operationType)
{
	ProcessCpuCycle();
	uint8_t value = _memoryManager->Read(addr, operationType);
	return value;
}
#endif

uint16_t PceCpu::MemoryReadWord(uint16_t addr, MemoryOperationType operationType)
{
	uint8_t lo = MemoryRead(addr, operationType);
	uint8_t hi = MemoryRead(addr + 1, operationType);
	return lo | hi << 8;
}

uint16_t PceCpu::GetIndAddr()
{
	return ReadWord();
}

uint8_t PceCpu::GetImmediate()
{
	return ReadByte();
}

uint8_t PceCpu::GetZeroAddr()
{
	return ReadByte();
}

uint8_t PceCpu::GetZeroXAddr()
{
	uint8_t value = ReadByte();
	return value + X();
}

uint8_t PceCpu::GetZeroYAddr()
{
	uint8_t value = ReadByte();
	return value + Y();
}

uint16_t PceCpu::GetAbsAddr()
{
	return ReadWord();
}

uint16_t PceCpu::GetAbsXAddr()
{
	uint16_t baseAddr = ReadWord();
	return baseAddr + X();
}

uint16_t PceCpu::GetAbsYAddr()
{
	uint16_t baseAddr = ReadWord();
	return baseAddr + Y();
}

uint16_t PceCpu::ReadZeroPageWrap(uint8_t zero)
{
	if(zero == 0xFF) {
		uint8_t lo = MemoryRead(PceCpu::ZeroPage + 0xFF);
		uint8_t hi = (MemoryRead(PceCpu::ZeroPage + 0x00) << 8);
		return lo | (hi << 8);
	} else {
		return MemoryReadWord(PceCpu::ZeroPage + zero);
	}
}

uint16_t PceCpu::GetIndZeroAddr()
{
	uint8_t zero = ReadByte();
	DummyRead();
	uint16_t addr = ReadZeroPageWrap(zero);
	DummyRead();
	return addr;
}

uint16_t PceCpu::GetIndXAddr()
{
	uint8_t zero = ReadByte();
	DummyRead();
	zero += X();
	uint16_t addr = ReadZeroPageWrap(zero);
	DummyRead();
	return addr;
}

uint16_t PceCpu::GetIndYAddr()
{
	uint8_t zero = ReadByte();
	DummyRead();
	uint16_t addr = ReadZeroPageWrap(zero);
	DummyRead();
	return addr + Y();
}

void PceCpu::ProcessIrq(bool forBrk)
{
#ifndef DUMMYCPU
	uint16_t originalPc = PC();
#endif

	if(!forBrk) {
		DummyRead();  //fetch opcode (and discard it - $00 (BRK) is forced into the opcode register instead)
		DummyRead();  //read next instruction byte (actually the same as above, since PC increment is suppressed. Also discarded.)
	}

	DummyRead();
	Push((uint16_t)(PC()));
	DummyRead();
	DummyRead();

	if(forBrk) {
		//B flag is set on the stack for BRK
		Push((uint8_t)(PS() | PceCpuFlags::Break));
	} else {
		//"When an interrupt occurs P is pushed with the current state of D and T"
		//"when an interrupt occurs, [..] the value pushed to the stack has B cleared"
		Push((uint8_t)(PS() & ~PceCpuFlags::Break));
	}

	//"Within the interrupt subroutine, the CPU clears D, T and sets I,"
	ClearFlags(PceCpuFlags::Decimal | PceCpuFlags::Memory);
	SetFlags(PceCpuFlags::Interrupt);

	if(_memoryManager->HasIrqSource(PceIrqSource::TimerIrq)) {
		SetPC(MemoryReadWord(PceCpu::TimerIrqVector));
	} else if(_memoryManager->HasIrqSource(PceIrqSource::Irq1)) {
		SetPC(MemoryReadWord(PceCpu::Irq1Vector));
	} else if(_memoryManager->HasIrqSource(PceIrqSource::Irq2)) {
		SetPC(MemoryReadWord(PceCpu::Irq2Vector));
	}

#ifndef DUMMYCPU
	_emu->ProcessInterrupt<CpuType::Pce>(originalPc, _state.PC, false);
#endif
}
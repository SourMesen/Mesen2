#include "stdafx.h"
#include "CpuTypes.h"
#include "Cpu.h"
#include "MemoryManager.h"

//TODO: PER doesn't load the right number of bytes?

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
		M::AbsJmp,M::DirIdxIndX, M::AbsLngJmp,M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Stk, M::ImmM,    M::Acc, M::Stk, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // 2
		M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::DirIdxX, M::DirIdxX, M::DirIdxX, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Acc, M::Imp, M::AbsIdxX,    M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX, // 3
		M::Stk,   M::DirIdxIndX, M::Imm8,     M::StkRel,        M::BlkMov,  M::Dir,     M::Dir,     M::DirIndLng,     M::Stk, M::ImmM,    M::Acc, M::Stk, M::AbsJmp,     M::Abs,     M::Abs,     M::AbsLng,     // 4
		M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::BlkMov,  M::DirIdxX, M::DirIdxX, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Stk, M::Imp, M::AbsLngJmp,  M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX, // 5
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

uint32_t Cpu::GetProgramAddress(uint16_t addr)
{
	return (_state.K << 16) | addr;
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
	_state.CycleCount++;
	return _memoryManager->Read((_state.K << 16) | addr, type);
}

uint16_t Cpu::ReadCodeWord(uint16_t addr, MemoryOperationType type)
{
	uint8_t lsb = ReadCode(addr);
	uint8_t msb = ReadCode(addr + 1);
	return (msb << 8) | lsb;
}

uint8_t Cpu::ReadData(uint32_t addr, MemoryOperationType type)
{
	_state.CycleCount++;
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
	_state.CycleCount++;
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
		case AddrMode::Abs: return GetDataAddress(ReadOperandWord());
		case AddrMode::AbsIdxX: return GetDataAddress(ReadOperandWord()) + _state.X;
		case AddrMode::AbsIdxY: return GetDataAddress(ReadOperandWord()) + _state.Y;
		case AddrMode::AbsLng: return ReadOperandLong();
		case AddrMode::AbsLngIdxX: return ReadOperandLong() + _state.X;		
		
		case AddrMode::AbsJmp: return GetProgramAddress(ReadOperandWord());
		case AddrMode::AbsLngJmp: return ReadOperandLong();
		case AddrMode::AbsIdxXInd: return GetProgramAddress(ReadDataWord(GetProgramAddress(ReadOperandWord() + _state.X))); //JMP/JSR
		case AddrMode::AbsInd: return GetProgramAddress(ReadDataWord(ReadOperandWord())); //JMP only
		case AddrMode::AbsIndLng: return ReadDataLong(ReadOperandWord()); //JML only

		case AddrMode::Acc: DummyRead(); return 0;

		case AddrMode::BlkMov: return ReadOperandWord();

		case AddrMode::DirIdxIndX: {
			uint8_t operand = ReadOperandByte();
			uint8_t lsb = ReadData(GetDirectAddress(operand, _state.X));
			uint8_t msb = ReadData(GetDirectAddress(operand, _state.X + 1));
			return GetDataAddress((msb << 8) | lsb);
		}

		case AddrMode::DirIdxX: return GetDirectAddress(ReadOperandByte(), _state.X);
		case AddrMode::DirIdxY: return GetDirectAddress(ReadOperandByte(), _state.Y);
		
		case AddrMode::DirIndIdxY:{
			uint8_t operand = ReadOperandByte();
			uint8_t lsb = ReadData(GetDirectAddress(operand));
			uint8_t msb = ReadData(GetDirectAddress(operand, 1));
			return GetDataAddress((msb << 8) | lsb) + _state.Y;
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
			return GetDataAddress((msb << 8) | lsb);
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
			return GetDataAddress(addr) + _state.Y;
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

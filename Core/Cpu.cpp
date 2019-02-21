#include "stdafx.h"
#include "CpuTypes.h"
#include "Cpu.h"
#include "MemoryManager.h"

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
		&C::BVC, &C::EOR, &C::EOR, &C::EOR, &C::MVN, &C::EOR, &C::LSR, &C::EOR, &C::CLI, &C::EOR, &C::PHY, &C::TCD, &C::JML, &C::EOR, &C::LSR, &C::EOR, // 5
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
		M::Stk,   M::DirIdxIndX, M::RelLng,   M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Stk, M::ImmM,    M::Acc, M::Stk, M::AbsInd,     M::Abs,     M::Abs,     M::AbsLng,     // 6
		M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::DirIdxX, M::DirIdxX, M::DirIdxX, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Stk, M::Imp, M::AbsIdxXInd, M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX, // 7
		M::Rel,   M::DirIdxIndX, M::RelLng,   M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Imp, M::ImmM,    M::Imp, M::Stk, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // 8
		M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::DirIdxX, M::DirIdxX, M::DirIdxY, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Imp, M::Imp, M::Abs,        M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX, // 9
		M::ImmX,  M::DirIdxIndX, M::ImmX,     M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Imp, M::ImmM,    M::Imp, M::Stk, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // A
		M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::DirIdxX, M::DirIdxX, M::DirIdxY, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Imp, M::Imp, M::AbsIdxX,    M::AbsIdxX, M::AbsIdxY, M::AbsLngIdxX, // B
		M::ImmX,  M::DirIdxIndX, M::Imm8,     M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Imp, M::ImmM,    M::Imp, M::Imp, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // C
		M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::Dir,     M::DirIdxX, M::DirIdxX, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Stk, M::Imp, M::AbsIndLng,  M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX, // D
		M::ImmX,  M::DirIdxIndX, M::Imm8,     M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Imp, M::ImmM,    M::Imp, M::Imp, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // E
		M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::Imm16,   M::DirIdxX, M::DirIdxX, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Stk, M::Imp, M::AbsIdxXInd, M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX  // F
	};

	memcpy(_opTable, opTable, sizeof(opTable));
	memcpy(_addrMode, addrMode, sizeof(addrMode));

	_memoryManager = memoryManager;
	_state = {};
	_state.PC = ReadDataWord(Cpu::ResetVector);
	_state.SP = 0x1FF;
	_state.EmulationMode = true;
	_nmiFlag = false;
	_irqSource = (uint8_t)IrqSource::None;
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

	if(_nmiFlag) {
		ProcessInterrupt(_state.EmulationMode ? Cpu::LegacyNmiVector : Cpu::NmiVector);
		_nmiFlag = false;
	} else if(_irqSource && !CheckFlag(ProcFlags::IrqDisable)) {
		ProcessInterrupt(_state.EmulationMode ? Cpu::LegacyIrqVector : Cpu::IrqVector);
	}
}

void Cpu::SetNmiFlag()
{
	_nmiFlag = true;
}

void Cpu::SetIrqSource(IrqSource source)
{
	_irqSource |= (uint8_t)source;
}

bool Cpu::CheckIrqSource(IrqSource source)
{
	if(_irqSource & (uint8_t)source) {
		return true;
	} else {
		return false;
	}
}

void Cpu::ClearIrqSource(IrqSource source)
{
	_irqSource &= ~(uint8_t)source;
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
	Write(_state.SP, value);
	SetSP(_state.SP - 1);
}

uint8_t Cpu::PopByte()
{
	SetSP(_state.SP + 1);
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

uint16_t Cpu::GetDirectAddress(uint16_t offset, bool allowEmulationMode)
{
	if(allowEmulationMode && _state.EmulationMode && (_state.D & 0xFF) == 0) {
		//TODO: Check if new instruction or not (PEI)
		return (uint16_t)((_state.D & 0xFF00) | (offset & 0xFF));
	} else {
		return (uint16_t)(_state.D + offset);
	}
}

uint16_t Cpu::GetDirectAddressIndirectWord(uint16_t offset, bool allowEmulationMode)
{
	uint8_t lsb = ReadData(GetDirectAddress(offset + 0));
	uint8_t msb = ReadData(GetDirectAddress(offset + 1));
	return (msb << 8) | lsb;
}

uint32_t Cpu::GetDirectAddressIndirectLong(uint16_t offset, bool allowEmulationMode)
{
	uint8_t b1 = ReadData(GetDirectAddress(offset + 0));
	uint8_t b2 = ReadData(GetDirectAddress(offset + 1));
	uint8_t b3 = ReadData(GetDirectAddress(offset + 2));
	return (b3 << 16) | (b2 << 8) | b1;
}

uint32_t Cpu::FetchEffectiveAddress()
{
	switch(_instAddrMode) {
		case AddrMode::Abs: return GetDataAddress(ReadOperandWord());
		case AddrMode::AbsIdxX: return (GetDataAddress(ReadOperandWord()) + _state.X) & 0xFFFFFF;
		case AddrMode::AbsIdxY: return (GetDataAddress(ReadOperandWord()) + _state.Y) & 0xFFFFFF;
		case AddrMode::AbsLng: return ReadOperandLong();
		case AddrMode::AbsLngIdxX: return (ReadOperandLong() + _state.X) & 0xFFFFFF;
		
		case AddrMode::AbsJmp: return GetProgramAddress(ReadOperandWord());
		case AddrMode::AbsLngJmp: return ReadOperandLong();
		case AddrMode::AbsIdxXInd: return GetProgramAddress(ReadDataWord(GetProgramAddress(ReadOperandWord() + _state.X))); //JMP/JSR
		case AddrMode::AbsInd: return ReadDataWord(ReadOperandWord()); //JMP only
		case AddrMode::AbsIndLng: return ReadDataLong(ReadOperandWord()); //JML only

		case AddrMode::Acc: DummyRead(); return 0;

		case AddrMode::BlkMov: return ReadOperandWord();

		case AddrMode::Dir: return GetDirectAddress(ReadOperandByte());
		case AddrMode::DirIdxX: return GetDirectAddress(ReadOperandByte() + _state.X);
		case AddrMode::DirIdxY: return GetDirectAddress(ReadOperandByte() + _state.Y);

		case AddrMode::DirInd: return GetDataAddress(GetDirectAddressIndirectWord(ReadOperandByte()));
		case AddrMode::DirIdxIndX: return GetDataAddress(GetDirectAddressIndirectWord(ReadOperandByte() + _state.X));
		case AddrMode::DirIndIdxY: return (GetDataAddress(GetDirectAddressIndirectWord(ReadOperandByte())) + _state.Y) & 0xFFFFFF;
		case AddrMode::DirIndLng: return GetDirectAddressIndirectLong(ReadOperandByte());
		case AddrMode::DirIndLngIdxY: return (GetDirectAddressIndirectLong(ReadOperandByte()) + _state.Y) & 0xFFFFFF;

		case AddrMode::Imm8: return ReadOperandByte();
		case AddrMode::Imm16: return ReadOperandWord();
		case AddrMode::ImmX: return CheckFlag(ProcFlags::IndexMode8) ? ReadOperandByte() : ReadOperandWord();
		case AddrMode::ImmM: return CheckFlag(ProcFlags::MemoryMode8) ? ReadOperandByte() : ReadOperandWord();
		
		case AddrMode::Imp: DummyRead(); return 0;

		case AddrMode::RelLng: return ReadOperandWord();
		case AddrMode::Rel: return ReadOperandByte();

		case AddrMode::Stk: return 0;
		case AddrMode::StkRel: return (uint16_t)(ReadOperandByte() + _state.SP);

		case AddrMode::StkRelIndIdxY: {
			uint16_t addr = (uint16_t)(ReadOperandByte() + _state.SP);
			return (GetDataAddress(ReadDataWord(addr)) + _state.Y) & 0xFFFFFF;
		}
	}

	throw new std::runtime_error("Unreacheable code");
}

void Cpu::SetSP(uint16_t sp)
{
	if(_state.EmulationMode) {
		_state.SP = 0x100 | (sp & 0xFF);
	} else {
		_state.SP = sp;
	}
}

void Cpu::SetPS(uint8_t ps)
{
	_state.PS = ps;
	if(CheckFlag(ProcFlags::IndexMode8)) {
		//Truncate X/Y when 8-bit indexes are enabled
		_state.Y &= 0xFF;
		_state.X &= 0xFF;
	}
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

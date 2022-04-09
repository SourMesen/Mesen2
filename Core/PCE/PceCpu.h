#pragma once
#include "stdafx.h"
#include "PCE/PceTypes.h"
#include "PCE/PceMemoryManager.h"
#include "PCE/PceConsole.h"
#include "Shared/Emulator.h"
#include "Utilities/RandomHelper.h"
#include "MemoryOperationType.h"

class PceCpu
{
private:
	static constexpr uint16_t ResetVector = 0xFFFE;
	static constexpr uint16_t NmiVector = 0xFFFC;
	static constexpr uint16_t TimerIrqVector = 0xFFFA;
	static constexpr uint16_t Irq1Vector = 0xFFF8;
	static constexpr uint16_t Irq2Vector = 0xFFF6;
	
	static constexpr uint16_t ZeroPage = 0x2000;
	static constexpr uint16_t StackPage = 0x2100;

	typedef void(PceCpu::* Func)();

	Emulator* _emu;
	PceConsole* _console;
	PceMemoryManager* _memoryManager;

	PceCpuState _state;
	uint16_t _operand;
	uint16_t _operand2;
	uint16_t _operand3;
	bool _memoryFlag = false;

	bool _needIrq = false;

	Func _opTable[256];
	PceAddrMode _addrMode[256];
	PceAddrMode _instAddrMode;

private:
	void WriteMemoryModeValue(uint8_t value);
	void AND();
	void EOR();
	void ORA();

	void ADD(uint8_t value);
	void SUB(uint8_t value);

	void ADC();
	void SBC();

	void CMP(uint8_t reg, uint8_t value);

	void CPA();
	void CPX();
	void CPY();

	void INC();
	void DEC();

	uint8_t ASL(uint8_t value);
	uint8_t LSR(uint8_t value);
	uint8_t ROL(uint8_t value);
	uint8_t ROR(uint8_t value);

	void ASLAddr();
	void LSRAddr();
	void ROLAddr();
	void RORAddr();

	void BranchRelative(bool branch);

	void BIT();

	//OP Codes
	void LDA();
	void LDX();
	void LDY();

	void STA();
	void STX();
	void STY();
	void STZ();

	void TAX();
	void TAY();
	void TSX();
	void TXA();
	void TXS();
	void TYA();

	void PHA();
	void PHP();
	void PLA();
	void PLP();
	void PHY();
	void PLY();
	void PHX();
	void PLX();

	void INX();
	void INY();

	void DEX();
	void DEY();

	void ASL_Acc();
	void ASL_Memory();

	void LSR_Acc();
	void LSR_Memory();

	void ROL_Acc();
	void ROL_Memory();

	void ROR_Acc();
	void ROR_Memory();

	void JMP_Abs();
	void JMP_Ind();
	void JMP_AbsX();

	void JSR();
	void RTS();

	void BCC();
	void BCS();
	void BEQ();
	void BMI();
	void BNE();
	void BPL();
	void BVC();
	void BVS();

	void CLC();
	void CLD();
	void CLI();
	void CLV();
	void SEC();
	void SED();
	void SEI();

	void BRK();
	void RTI();
	void NOP();

	void BSR();
	void BRA();

	void SXY();
	void SAX();
	void SAY();
	
	void CLA();
	void CLX();
	void CLY();

	void ST0()
	{
		DummyRead();
		DummyRead();
		_memoryManager->WriteVdc(0, _operand);
	}
	
	void ST1()
	{
		DummyRead();
		DummyRead();
		_memoryManager->WriteVdc(2, _operand);
	}

	void ST2()
	{
		DummyRead();
		DummyRead();
		_memoryManager->WriteVdc(3, _operand);
	}

	void TMA();
	void TAM();
	
	void StartBlockTransfer()
	{
		DummyRead();
		Push(Y());
		DummyRead();
		Push(X());
		DummyRead();
		Push(A());
		DummyRead();

		_state.SH = (_operand & 0xFF00) >> 8;
		_state.DH = (_operand2 & 0xFF00) >> 8;
		_state.LH = (_operand3 & 0xFF00) >> 8;
	}

	void EndBlockTransfer()
	{
		SetA(Pop());
		SetX(Pop());
		SetY(Pop());
	}

	void TAI() 
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

	void TDD()
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

			src--;
			dst--;

			count++;
			length--;
		} while(length);

		EndBlockTransfer();
	}

	void TIA()
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

	void TII()
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
			dst++;

			count++;
			length--;
		} while(length);

		EndBlockTransfer();
	}

	void TIN()
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

			count++;
			length--;
		} while(length);

		EndBlockTransfer();
	}

	void TSB();
	void TRB();
	void TST();

	void CSL()
	{
		//CSH/CSL take 3 cycles
		DummyRead();
		_memoryManager->SetSpeed(true);
	}
	
	void CSH()
	{
		//CSH/CSL take 3 cycles
		DummyRead();
		_memoryManager->SetSpeed(false);
	}
	
	void SET()
	{
		SetFlags(PceCpuFlags::Memory);
	}
	
	void RMB0() { RMB<0>(); }
	void RMB1() { RMB<1>(); }
	void RMB2() { RMB<2>(); }
	void RMB3() { RMB<3>(); }
	void RMB4() { RMB<4>(); }
	void RMB5() { RMB<5>(); }
	void RMB6() { RMB<6>(); }
	void RMB7() { RMB<7>(); }

	void SMB0() { SMB<0>(); }
	void SMB1() { SMB<1>(); }
	void SMB2() { SMB<2>(); }
	void SMB3() { SMB<3>(); }
	void SMB4() { SMB<4>(); }
	void SMB5() { SMB<5>(); }
	void SMB6() { SMB<6>(); }
	void SMB7() { SMB<7>(); }

	void INC_Acc();
	void DEC_Acc();
	
	void BBR0() { BBR<0>(); }
	void BBR1() { BBR<1>(); }
	void BBR2() { BBR<2>(); }
	void BBR3() { BBR<3>(); }
	void BBR4() { BBR<4>(); }
	void BBR5() { BBR<5>(); }
	void BBR6() { BBR<6>(); }
	void BBR7() { BBR<7>(); }
	
	void BBS0() { BBS<0>(); }
	void BBS1() { BBS<1>(); }
	void BBS2() { BBS<2>(); }
	void BBS3() { BBS<3>(); }
	void BBS4() { BBS<4>(); }
	void BBS5() { BBS<5>(); }
	void BBS6() { BBS<6>(); }
	void BBS7() { BBS<7>(); }

	template<uint8_t bit>
	void BBR()
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

	template<uint8_t bit>
	void BBS()
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
	
	template<uint8_t bit>
	void RMB()
	{
		uint8_t value = MemoryRead(_operand);
		value &= ~(1 << bit);
		DummyRead();
		DummyRead();
		MemoryWrite(_operand, value);
	}

	template<uint8_t bit>
	void SMB()
	{
		uint8_t value = MemoryRead(_operand);
		value |= (1 << bit);
		DummyRead();
		DummyRead();
		MemoryWrite(_operand, value);
	}

public:
	PceCpu(Emulator* emu, PceConsole* console, PceMemoryManager* memoryManager)
	{
		_emu = emu;
		_console = console;
		_memoryManager = memoryManager;

		typedef PceCpu C;
		Func opTable[] = {
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
		PceAddrMode addrMode[] = {
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

		memcpy(_opTable, opTable, sizeof(opTable));
		memcpy(_addrMode, addrMode, sizeof(addrMode));

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

	PceCpuState& GetState()
	{
		return _state;
	}

	void Exec()
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

		_emu->ProcessInstruction<CpuType::Pce>();

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

	uint16_t FetchOperand()
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

	void SetRegister(uint8_t& reg, uint8_t value)
	{
		ClearFlags(PceCpuFlags::Zero | PceCpuFlags::Negative);
		SetZeroNegativeFlags(value);
		reg = value;
	}

	void Push(uint8_t value)
	{
		MemoryWrite(PceCpu::StackPage + SP(), value);
		SetSP(SP() - 1);
	}

	void Push(uint16_t value)
	{
		Push((uint8_t)(value >> 8));
		Push((uint8_t)value);
	}

	uint8_t Pop()
	{
		SetSP(SP() + 1);
		return MemoryRead(PceCpu::StackPage + SP());
	}

	uint16_t PopWord()
	{
		uint8_t lo = Pop();
		uint8_t hi = Pop();

		return lo | hi << 8;
	}

	uint8_t A() { return _state.A; }
	void SetA(uint8_t value) { SetRegister(_state.A, value); }
	uint8_t X() { return _state.X; }
	void SetX(uint8_t value) { SetRegister(_state.X, value); }
	uint8_t Y() { return _state.Y; }
	void SetY(uint8_t value) { SetRegister(_state.Y, value); }
	uint8_t SP() { return _state.SP; }
	void SetSP(uint8_t value) { _state.SP = value; }
	uint8_t PS() { return _state.PS; }
	void SetPS(uint8_t value) { _state.PS = value & 0xCF; }
	uint16_t PC() { return _state.PC; }
	void SetPC(uint16_t value) { _state.PC = value; }

	uint8_t GetOPCode()
	{
		uint8_t opCode = MemoryRead(_state.PC, MemoryOperationType::ExecOpCode);
		_state.PC++;
		return opCode;
	}

	uint16_t GetOperand()
	{
		return _operand;
	}

	uint8_t GetOperandValue()
	{
		if(_instAddrMode >= PceAddrMode::Zero) {
			return MemoryRead(GetOperand());
		} else {
			return (uint8_t)GetOperand();
		}
	}

	void DummyRead()
	{
		//TODO - is this supposed to be an idle cycle, or a dummy read?
		MemoryRead(_state.PC, MemoryOperationType::DummyRead);
	}

	uint8_t ReadByte()
	{
		uint8_t value = MemoryRead(_state.PC, MemoryOperationType::ExecOperand);
		_state.PC++;
		return value;
	}

	uint16_t ReadWord()
	{
		uint8_t low = ReadByte();
		uint8_t high = ReadByte();
		return (high << 8) | low;
	}

	void ClearFlags(uint8_t flags)
	{
		_state.PS &= ~flags;
	}

	void SetFlags(uint8_t flags)
	{
		_state.PS |= flags;
	}

	bool CheckFlag(uint8_t flag)
	{
		return (_state.PS & flag) == flag;
	}

	void SetZeroNegativeFlags(uint8_t value)
	{
		if(value == 0) {
			SetFlags(PceCpuFlags::Zero);
		} else if(value & 0x80) {
			SetFlags(PceCpuFlags::Negative);
		}
	}

	bool CheckPageCrossed(uint16_t valA, int8_t valB)
	{
		return ((valA + valB) & 0xFF00) != (valA & 0xFF00);
	}

	bool CheckPageCrossed(uint16_t valA, uint8_t valB)
	{
		return ((valA + valB) & 0xFF00) != (valA & 0xFF00);
	}

	void ProcessCpuCycle()
	{
		_state.CycleCount++;
		_memoryManager->Exec();

		_needIrq = _memoryManager->HasIrqSource((PceIrqSource)0xFF) && !CheckFlag(PceCpuFlags::Interrupt);
	}

	void MemoryWrite(uint16_t addr, uint8_t value, MemoryOperationType operationType = MemoryOperationType::Write)
	{
#ifdef DUMMYCPU
		LogMemoryOperation(addr, value, operationType);
#else
		ProcessCpuCycle();
		_memoryManager->Write(addr, value, operationType);
#endif
	}

	uint8_t MemoryRead(uint16_t addr, MemoryOperationType operationType = MemoryOperationType::Read)
	{
#ifdef DUMMYCPU
		uint8_t value = _memoryManager->DebugRead(addr);
		LogMemoryOperation(addr, value, operationType);
		return value;
#else 

		ProcessCpuCycle();
		uint8_t value = _memoryManager->Read(addr, operationType);
		return value;
#endif
	}

	uint16_t MemoryReadWord(uint16_t addr, MemoryOperationType operationType = MemoryOperationType::Read)
	{
		uint8_t lo = MemoryRead(addr, operationType);
		uint8_t hi = MemoryRead(addr + 1, operationType);
		return lo | hi << 8;
	}

	uint16_t GetIndAddr() { return ReadWord(); }
	uint8_t GetImmediate() { return ReadByte(); }
	uint8_t GetZeroAddr() { return ReadByte(); }
	
	uint8_t GetZeroXAddr()
	{
		uint8_t value = ReadByte();
		return value + X();
	}

	uint8_t GetZeroYAddr()
	{
		uint8_t value = ReadByte();
		return value + Y();
	}

	uint16_t GetAbsAddr() { return ReadWord(); }

	uint16_t GetAbsXAddr()
	{
		uint16_t baseAddr = ReadWord();
		return baseAddr + X();
	}

	uint16_t GetAbsYAddr()
	{
		uint16_t baseAddr = ReadWord();
		return baseAddr + Y();
	}

	uint16_t ReadZeroPageWrap(uint8_t zero)
	{
		if(zero == 0xFF) {
			uint8_t lo = MemoryRead(PceCpu::ZeroPage + 0xFF);
			uint8_t hi = (MemoryRead(PceCpu::ZeroPage + 0x00) << 8);
			return lo | (hi << 8);
		} else {
			return MemoryReadWord(PceCpu::ZeroPage + zero);
		}
	}

	uint16_t GetIndZeroAddr()
	{
		uint8_t zero = ReadByte();
		DummyRead();
		uint16_t addr = ReadZeroPageWrap(zero);
		DummyRead();
		return addr;
	}

	uint16_t GetIndXAddr()
	{
		uint8_t zero = ReadByte();
		DummyRead();
		zero += X();
		uint16_t addr = ReadZeroPageWrap(zero);
		DummyRead();
		return addr;
	}

	uint16_t GetIndYAddr()
	{
		uint8_t zero = ReadByte();
		DummyRead();
		uint16_t addr = ReadZeroPageWrap(zero);
		DummyRead();
		return addr + Y();
	}

	void ProcessIrq(bool forBrk)
	{
		uint16_t originalPc = PC();

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

		_emu->ProcessInterrupt<CpuType::Pce>(originalPc, _state.PC, false);
	}
};
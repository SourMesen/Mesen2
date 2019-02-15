#pragma once
#include "stdafx.h"
#include "CpuTypes.h"

class MemoryManager;

class Cpu
{
public:
	uint64_t opCount = 0;
	uint16_t GetPc() { return _state.PC; }
	CpuState GetState() { return _state; }
	int32_t GetLastOperand() { return (int32_t)_operand; }

private:
	static constexpr uint32_t NmiVector = 0x00FFEA;
	static constexpr uint32_t ResetVector = 0x00FFFC;
	static constexpr uint32_t IrqVector = 0x00FFEE;
	static constexpr uint32_t AbortVector = 0x00FFE8;
	static constexpr uint32_t BreakVector = 0x00FFE6;
	static constexpr uint32_t CoprocessorVector = 0x00FFE4;

	static constexpr uint16_t LegacyNmiVector = 0xFFFA;
	static constexpr uint32_t LegacyBreakVector = 0xFFFE;
	static constexpr uint32_t LegacyCoprocessorVector = 0x00FFF4;

	typedef void(Cpu::*Func)();
	
	shared_ptr<MemoryManager> _memoryManager;
	CpuState _state;
	AddrMode _instAddrMode;
	uint32_t _operand;
	bool _nmiFlag;

	Func _opTable[256];
	AddrMode _addrMode[256];
	
	uint32_t GetProgramAddress(uint16_t addr);
	uint32_t GetDataAddress(uint16_t addr);

	uint16_t GetDirectAddress(uint16_t offset, bool allowEmulationMode = true);

	uint16_t GetDirectAddressIndirectWord(uint16_t offset, bool allowEmulationMode = true);
	uint32_t GetDirectAddressIndirectLong(uint16_t offset, bool allowEmulationMode = true);
	
	uint8_t GetOpCode();

	void DummyRead();
	
	uint8_t ReadOperandByte();
	uint16_t ReadOperandWord();
	uint32_t ReadOperandLong();
	uint32_t FetchEffectiveAddress();

	void SetSP(uint16_t sp);

	void SetRegister(uint8_t &reg, uint8_t value);
	void SetRegister(uint16_t &reg, uint16_t value, bool eightBitMode);
	
	void SetZeroNegativeFlags(uint16_t value);
	void SetZeroNegativeFlags(uint8_t value);

	void ClearFlags(uint8_t flags);
	void SetFlags(uint8_t flags);
	bool CheckFlag(uint8_t flag);

	uint8_t ReadCode(uint16_t addr, MemoryOperationType type = MemoryOperationType::Read);
	uint16_t ReadCodeWord(uint16_t addr, MemoryOperationType type = MemoryOperationType::Read);

	uint8_t ReadData(uint32_t addr, MemoryOperationType type = MemoryOperationType::Read);
	uint16_t ReadDataWord(uint32_t addr, MemoryOperationType type = MemoryOperationType::Read);
	uint32_t ReadDataLong(uint32_t addr, MemoryOperationType type = MemoryOperationType::Read);

	void Write(uint32_t addr, uint8_t value, MemoryOperationType type = MemoryOperationType::Write);
	void WriteWord(uint32_t addr, uint16_t value, MemoryOperationType type = MemoryOperationType::Write);

	uint8_t GetByteValue();

	uint16_t GetWordValue();

	void PushByte(uint8_t value);
	uint8_t PopByte();

	void PushWord(uint16_t value);
	uint16_t PopWord();

	//Add/substract instructions
	void Add8(uint8_t value);
	void Add16(uint16_t value);
	void ADC();

	void Sub8(uint8_t value);
	void Sub16(uint16_t value);
	void SBC();
	
	//Branch instructions
	void BCC();
	void BCS();
	void BEQ();
	void BMI();
	void BNE();
	void BPL();
	void BRA();
	void BRL();
	void BVC();
	void BVS();
	void BranchRelative(bool branch);
	
	//Set/clear flag instructions
	void CLC();
	void CLD();
	void CLI();
	void CLV();
	void SEC();
	void SED();
	void SEI();

	void REP();
	void SEP();

	//Increment/decrement instructions
	void DEX();
	void DEY();
	void INX();
	void INY();
	void DEC();
	void INC();

	void IncDecReg(uint16_t & reg, int8_t offset);
	void IncDec(int8_t offset);

	//Compare instructions
	void Compare(uint16_t reg, bool eightBitMode);
	void CMP();
	void CPX();
	void CPY();

	//Jump instructions
	void JML();
	void JMP();
	void JSL();
	void JSR();
	void RTI();
	void RTL();
	void RTS();

	//Interrupts
	void ProcessInterrupt(uint16_t vector);
	void BRK();
	void COP();

	//Bitwise operations
	void AND();
	void EOR();
	void ORA();

	template<typename T> T ShiftLeft(T value);
	template<typename T> T RollLeft(T value);
	template<typename T> T ShiftRight(T value);
	template<typename T> T RollRight(T value);

	//Shift operations
	void ASL();
	void LSR();
	void ROL();
	void ROR();

	//Move operations
	void MVN();
	void MVP();

	//Push/pull instructions
	void PEA();
	void PEI();
	void PER();
	void PHB();
	void PHD();
	void PHK();
	void PHP();
	void PLB();
	void PLD();
	void PLP();

	void PHA();
	void PHX();
	void PHY();
	void PLA();
	void PLX();
	void PLY();

	void PushRegister(uint16_t reg, bool eightBitMode);
	void PullRegister(uint16_t &reg, bool eightBitMode);

	//Store/load instructions
	void LoadRegister(uint16_t &reg, bool eightBitMode);
	void StoreRegister(uint16_t val, bool eightBitMode);

	void LDA();
	void LDX();
	void LDY();

	void STA();
	void STX();
	void STY();
	void STZ();
		
	//Test bits
	template<typename T> void TestBits(T value, bool alterZeroFlagOnly);
	void BIT();

	void TRB();
	void TSB();

	//Transfer registers
	void TAX();
	void TAY();
	void TCD();
	void TCS();
	void TDC();
	void TSC();
	void TSX();
	void TXA();
	void TXS();
	void TXY();
	void TYA();
	void TYX();
	void XBA();
	void XCE();

	//No operation
	void NOP();
	void WDM();

	//Misc.
	void STP();
	void WAI();

public:
	Cpu(shared_ptr<MemoryManager> memoryManager);
	~Cpu();

	void Reset();
	void Exec();

	void SetNmiFlag();
};
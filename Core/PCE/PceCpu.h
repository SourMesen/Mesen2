#if (defined(DUMMYCPU) && !defined(__DUMMYCPU__H)) || (!defined(DUMMYCPU) && !defined(__CPU__H))
#ifdef DUMMYCPU
#define __DUMMYCPU__H
#else
#define __CPU__H
#endif

#include "pch.h"
#include "PCE/PceTypes.h"
#include "Utilities/ISerializable.h"
#include "Shared/MemoryOperationType.h"

class Emulator;
class PceMemoryManager;

class PceCpu final : public ISerializable
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

	static Func const _opTable[256];
	static PceAddrMode const _addrMode[256];

	Emulator* _emu;
	PceMemoryManager* _memoryManager;

	PceCpuState _state;
	uint16_t _operand;
	uint16_t _operand2;
	uint16_t _operand3;
	bool _memoryFlag = false;

	uint8_t _pendingIrqs = 0;
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

	void ST0();
	void ST1();
	void ST2();

	void TMA();
	void TAM();
	
	void StartBlockTransfer();
	void EndBlockTransfer();

	void TAI();
	void TDD();
	void TIA();
	void TII();
	void TIN();

	void TSB();
	void TRB();
	void TST();

	void CSL();
	void CSH();
	
	void SET();
	
	void RMB0() { RMB(0); }
	void RMB1() { RMB(1); }
	void RMB2() { RMB(2); }
	void RMB3() { RMB(3); }
	void RMB4() { RMB(4); }
	void RMB5() { RMB(5); }
	void RMB6() { RMB(6); }
	void RMB7() { RMB(7); }

	void SMB0() { SMB(0); }
	void SMB1() { SMB(1); }
	void SMB2() { SMB(2); }
	void SMB3() { SMB(3); }
	void SMB4() { SMB(4); }
	void SMB5() { SMB(5); }
	void SMB6() { SMB(6); }
	void SMB7() { SMB(7); }

	void INC_Acc();
	void DEC_Acc();
	
	void BBR0() { BBR(0); }
	void BBR1() { BBR(1); }
	void BBR2() { BBR(2); }
	void BBR3() { BBR(3); }
	void BBR4() { BBR(4); }
	void BBR5() { BBR(5); }
	void BBR6() { BBR(6); }
	void BBR7() { BBR(7); }
	
	void BBS0() { BBS(0); }
	void BBS1() { BBS(1); }
	void BBS2() { BBS(2); }
	void BBS3() { BBS(3); }
	void BBS4() { BBS(4); }
	void BBS5() { BBS(5); }
	void BBS6() { BBS(6); }
	void BBS7() { BBS(7); }

	void BBR(uint8_t bit);
	void BBS(uint8_t bit);
	void RMB(uint8_t bit);
	void SMB(uint8_t bit);
	
	__forceinline void FetchOperand();

	void SetRegister(uint8_t& reg, uint8_t value);

	void Push(uint8_t value);
	void Push(uint16_t value);

	uint8_t Pop();
	uint16_t PopWord();

	uint8_t A() { return _state.A; }
	void SetA(uint8_t value) { SetRegister(_state.A, value); }
	uint8_t X() { return _state.X; }
	void SetX(uint8_t value) { SetRegister(_state.X, value); }
	uint8_t Y() { return _state.Y; }
	void SetY(uint8_t value) { SetRegister(_state.Y, value); }
	uint8_t SP() { return _state.SP; }
	void SetSP(uint8_t value) { _state.SP = value; }
	uint8_t PS() { return _state.PS; }
	void SetPS(uint8_t value) { _state.PS = value & 0xEF; }
	uint16_t PC() { return _state.PC; }
	void SetPC(uint16_t value) { _state.PC = value; }

	__forceinline uint8_t GetOPCode();
	uint16_t GetOperand();
	uint8_t GetOperandValue();

	void DummyRead();

	uint8_t ReadByte();
	uint16_t ReadWord();

	void ClearFlags(uint8_t flags);
	void SetFlags(uint8_t flags);
	bool CheckFlag(uint8_t flag);
	void SetZeroNegativeFlags(uint8_t value);

	void ProcessCpuCycle();

	void MemoryWrite(uint16_t addr, uint8_t value, MemoryOperationType operationType = MemoryOperationType::Write);

	uint8_t MemoryRead(uint16_t addr, MemoryOperationType operationType = MemoryOperationType::Read);
	uint16_t MemoryReadWord(uint16_t addr, MemoryOperationType operationType = MemoryOperationType::Read);

	__forceinline uint16_t GetIndAddr();
	__forceinline uint8_t GetImmediate();
	__forceinline uint8_t GetZeroAddr();

	__forceinline uint8_t GetZeroXAddr();
	__forceinline uint8_t GetZeroYAddr();

	__forceinline uint16_t GetAbsAddr();
	__forceinline uint16_t GetAbsXAddr();
	__forceinline uint16_t GetAbsYAddr();

	__forceinline uint16_t ReadZeroPageWrap(uint8_t zero);

	__forceinline uint16_t GetIndZeroAddr();
	__forceinline uint16_t GetIndXAddr();
	__forceinline uint16_t GetIndYAddr();

	void ProcessIrq(bool forBrk);

public:
	PceCpu(Emulator* emu, PceMemoryManager* memoryManager);

	PceCpuState& GetState() { return _state; }
	
	void RunIdleCpuCycle();

	void Exec();

	void Serialize(Serializer& s) override;

#ifdef DUMMYCPU
private:
	uint32_t _memOpCounter = 0;
	MemoryOperationInfo _memOperations[10] = {};

	void LogMemoryOperation(uint32_t addr, uint8_t value, MemoryOperationType type);

public:
	void SetDummyState(PceCpuState& state);

	uint32_t GetOperationCount();
	MemoryOperationInfo GetOperationInfo(uint32_t index);
#endif
};

#endif

#if (defined(DUMMYSPC) && !defined(__DUMMYSPC__H)) || (!defined(DUMMYSPC) && !defined(__SPC__H))
#ifdef DUMMYSPC
#define __DUMMYSPC__H
#else
#define __SPC__H
#endif

#include "stdafx.h"
#include "SpcTypes.h"
#include "CpuTypes.h"
#include "SpcTimer.h"
#include "../Utilities/ISerializable.h"

class Console;
class MemoryManager;
class SPC_DSP;
struct AddressInfo;

class Spc : public ISerializable
{
public:
	static constexpr int SpcRamSize = 0x10000;
	static constexpr int SpcRomSize = 0x40;

private:
	static constexpr int SampleBufferSize = 0x100000;
	static constexpr uint16_t ResetVector = 0xFFFE;

	shared_ptr<Console> _console;
	shared_ptr<MemoryManager> _memoryManager;
	unique_ptr<SPC_DSP> _dsp;

	bool _immediateMode;
	uint16_t _operandA;
	uint16_t _operandB;
	double _clockRatio;

	SpcState _state;
	uint8_t* _ram;
	uint8_t* _spcBios;

	int16_t *_soundBuffer;
	//Store operations
	void STA();
	void STX();
	void STY();
	void STW();

	void STA_AutoIncX();
	void LDA_AutoIncX();

	//Load operations
	void LDA();
	void LDX();
	void LDY();
	void LDW();

	//Transfer
	void TXA();
	void TYA();
	void TAX();
	void TAY();
	void TSX();
	void TXS();

	void STC();
	void LDC();
	void MOV();

	//Arithmetic
	uint8_t Add(uint8_t a, uint8_t b);
	uint8_t Sub(uint8_t a, uint8_t b);

	void ADC();
	void ADC_Acc();
	void ADDW();
	void SBC();
	void SBC_Acc();
	void SUBW();

	void Compare(uint8_t a, uint8_t b);
	void CMP();
	void CMP_Acc();
	void CPX();
	void CPY();
	void CMPW();

	//Increment/decrement
	void INC();
	void INC_Acc();
	void INX();
	void INY();
	void INCW();

	void DEC();
	void DEC_Acc();
	void DEX();
	void DEY();
	void DECW();

	void MUL();
	void DIV();

	//Decimal
	void DAA();
	void DAS();

	//Logical
	void AND();
	void AND_Acc();
	void OR();
	void OR_Acc();
	void EOR();
	void EOR_Acc();

	void SetCarry(uint8_t carry);
	void OR1();
	void NOR1();
	void AND1();
	void NAND1();
	void EOR1();
	void NOT1();

	//Shift/rotate
	uint8_t ShiftLeft(uint8_t value);
	uint8_t RollLeft(uint8_t value);
	uint8_t ShiftRight(uint8_t value);
	uint8_t RollRight(uint8_t value);

	void ASL();
	void ASL_Acc();
	void LSR();
	void LSR_Acc();
	void ROL();
	void ROL_Acc();
	void ROR();
	void ROR_Acc();
	void XCN();

	//Branch operations
	void Branch();
	void BRA();
	void BEQ();
	void BNE();
	void BCS();
	void BCC();
	void BVS();
	void BVC();
	void BMI();
	void BPL();
	void BBS();
	void BBC();
	void CBNE();
	void DBNZ();
	void DBNZ_Y();
	void JMP();

	//Flag operations
	void CLRC();
	void SETC();
	void NOTC();
	void CLRV();
	void CLRP();
	void SETP();
	void EI();
	void DI();

	void TSET1();
	void TCLR1();

	template<uint8_t bit> void SET1();
	template<uint8_t bit> void CLR1();
	template<uint8_t bit> void BBS();
	template<uint8_t bit> void BBC();

	//Subroutine operations
	void PCALL();
	void JSR();
	void RTS();
	void BRK();
	void RTI();

	template<uint8_t offset> void TCALL();

	//Stack operations
	void PHA();
	void PHX();
	void PHY();
	void PHP();

	void PLA();
	void PLX();
	void PLY();
	void PLP();

	//Other operations
	void NOP();
	void SLEEP();
	void STOP();
	
	void Idle();
	void DummyRead();
	void DummyRead(uint16_t addr);
	uint8_t Read(uint16_t addr, MemoryOperationType type = MemoryOperationType::Read);
	uint16_t ReadWord(uint16_t addr, MemoryOperationType type = MemoryOperationType::Read);
	void Write(uint16_t addr, uint8_t value, MemoryOperationType type = MemoryOperationType::Write);

	void Addr_Dir();
	void Addr_DirIdxX();
	void Addr_DirIdxY();
	void Addr_DirToDir();
	void Addr_DirImm();
	void Addr_IndX();
	void Addr_IndXToIndY();
	void Addr_Abs();
	void Addr_AbsBit();
	void Addr_AbsIdxX();
	void Addr_AbsIdxY();
	void Addr_AbsIdxXInd();
	void Addr_DirIdxXInd();
	void Addr_DirIndIdxY();
	void Addr_Rel();
	void Addr_Imm();

	void ClearFlags(uint8_t flags);
	void SetFlags(uint8_t flags);
	bool CheckFlag(uint8_t flag);
	void SetZeroNegativeFlags(uint8_t value);
	void SetZeroNegativeFlags16(uint16_t value);

	uint8_t GetByteValue();
	uint16_t GetWordValue();

	void Push(uint8_t value);
	uint8_t Pop();
	void PushWord(uint16_t value);
	uint16_t PopWord();

	uint16_t GetDirectAddress(uint8_t offset);

	uint8_t GetOpCode();
	uint8_t ReadOperandByte();

	void IncCycleCount(int32_t addr);
	void Exec();

public:
	Spc(shared_ptr<Console> console, vector<uint8_t> &spcRomData);
	virtual ~Spc();

	void Run();
	void Reset();

	uint8_t DebugRead(uint16_t addr);
	void DebugWrite(uint16_t addr, uint8_t value);

	uint8_t CpuReadRegister(uint16_t addr);
	void CpuWriteRegister(uint32_t addr, uint8_t value);

	void ProcessEndFrame();

	SpcState GetState();
	AddressInfo GetAbsoluteAddress(uint16_t addr);

	uint8_t* GetSpcRam();
	uint8_t* GetSpcRom();

	void Serialize(Serializer &s) override;

#ifdef DUMMYSPC
private:
	uint32_t _writeCounter = 0;
	uint32_t _writeAddresses[10];
	uint8_t _writeValue[10];

	uint32_t _readCounter = 0;
	uint32_t _readAddresses[10];
	uint8_t _readValue[10];

	void LogRead(uint32_t addr, uint8_t value);
	void LogWrite(uint32_t addr, uint8_t value);

public:
	DummySpc(uint8_t *spcRam, uint8_t *spcRom, SpcState &state);

	void Step();

	void SetDummyState(SpcState &state);
	int32_t GetLastOperand();

	uint32_t GetWriteCount();
	uint32_t GetReadCount();
	void GetWriteInfo(uint32_t index, uint32_t &addr, uint8_t &value);
	void GetReadInfo(uint32_t index, uint32_t &addr, uint8_t &value);
#endif
};

#endif
#if (defined(DUMMYSPC) && !defined(__DUMMYSPC__H)) || (!defined(DUMMYSPC) && !defined(__SPC__H))
#ifdef DUMMYSPC
#define __DUMMYSPC__H
#else
#define __SPC__H
#endif

#include "pch.h"
#include "SNES/SpcTypes.h"
#include "SNES/DSP/DspTypes.h"
#include "SNES/SnesCpuTypes.h"
#include "SNES/SpcTimer.h"
#include "Shared/MemoryOperationType.h"
#include "Utilities/ISerializable.h"

class SnesConsole;
class Emulator;
class SnesMemoryManager;
class SpcFileData;
class Dsp;
struct AddressInfo;

class Spc : public ISerializable
{
public:
	static constexpr int SpcRamSize = 0x10000;
	static constexpr int SpcRomSize = 0x40;
	static constexpr int SpcSampleRate = 32000;

private:
	static constexpr int SampleBufferSize = 0x20000;
	static constexpr uint16_t ResetVector = 0xFFFE;

	Emulator* _emu = nullptr;
	SnesConsole* _console = nullptr;
	SnesMemoryManager* _memoryManager = nullptr;
	unique_ptr<Dsp> _dsp;

	double _clockRatio = 0.0;

	/* Temporary data used in the middle of operations */
	uint16_t _operandA = 0;
	uint16_t _operandB = 0;
	uint16_t _tmp1 = 0;
	uint16_t _tmp2 = 0;
	uint16_t _tmp3 = 0;
	uint8_t _opCode = 0;
	SpcOpStep _opStep = {};
	uint8_t _opSubStep = 0;

	bool _enabled = false;
	bool _pendingCpuRegUpdate = false;
	uint32_t _spcSampleRate = Spc::SpcSampleRate;

	SpcState _state;
	uint8_t* _ram;
	uint8_t _spcBios[64] {
		0xCD, 0xEF, 0xBD, 0xE8, 0x00, 0xC6, 0x1D, 0xD0,
		0xFC, 0x8F, 0xAA, 0xF4, 0x8F, 0xBB, 0xF5, 0x78,
		0xCC, 0xF4, 0xD0, 0xFB, 0x2F, 0x19, 0xEB, 0xF4,
		0xD0, 0xFC, 0x7E, 0xF4, 0xD0, 0x0B, 0xE4, 0xF5,
		0xCB, 0xF4, 0xD7, 0x00, 0xFC, 0xD0, 0xF3, 0xAB,
		0x01, 0x10, 0xEF, 0x7E, 0xF4, 0x10, 0xEB, 0xBA,
		0xF6, 0xDA, 0x00, 0xBA, 0xF4, 0xC4, 0xF4, 0xDD,
		0x5D, 0xD0, 0xDB, 0x1F, 0x00, 0x00, 0xC0, 0xFF
	};

	//Store operations
	void STA();
	void STX();
	void STY();
	void STW();

	void STA_AutoIncX();
	void LDA_AutoIncX();

	//Load operations
	void LDA();
	void LDA_Imm();
	void LDX();
	void LDX_Imm();
	void LDY();
	void LDY_Imm();
	void LDW();

	//Transfer
	void Transfer(uint8_t & dst, uint8_t src);
	void TXA();
	void TYA();
	void TAX();
	void TAY();
	void TSX();
	void TXS();

	void STC();
	void LDC();
	void MOV();
	void MOV_Imm();

	//Arithmetic
	uint8_t Add(uint8_t a, uint8_t b);
	uint8_t Sub(uint8_t a, uint8_t b);

	void ADC();
	void ADC_Acc();
	void ADC_Imm();
	void ADDW();
	void SBC();
	void SBC_Acc();
	void SBC_Imm();
	void SUBW();

	void Compare(uint8_t a, uint8_t b);
	void CMP();
	void CMP_Acc();
	void CMP_Imm();
	void CPX();
	void CPX_Imm();
	void CPY();
	void CPY_Imm();
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
	void AND_Imm();
	void OR();
	void OR_Acc();
	void OR_Imm();
	void EOR();
	void EOR_Acc();
	void EOR_Imm();

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
	void PushOperation(uint8_t value);
	void PullOperation(uint8_t & dst);

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

	void Push(uint8_t value);
	uint8_t Pop();

	uint16_t GetDirectAddress(uint8_t offset);

	uint8_t GetOpCode();
	uint8_t ReadOperandByte();

	__forceinline void IncCycleCount(int32_t addr);
	void EndOp();
	void EndAddr();
	__forceinline void ProcessCycle();
	__forceinline void Exec();
	
	void UpdateClockRatio();
	void ExitExecLoop();

public:
	Spc(SnesConsole* console);
	virtual ~Spc();

	void SetSpcState(bool enabled);

	void Run();
	void Reset();

	uint8_t DebugRead(uint16_t addr);
	void DebugWrite(uint16_t addr, uint8_t value);

	void DebugWriteDspReg(uint8_t addr, uint8_t value);

	uint8_t CpuReadRegister(uint16_t addr);
	void CpuWriteRegister(uint32_t addr, uint8_t value);

	uint8_t DspReadRam(uint16_t addr);
	void DspWriteRam(uint16_t addr, uint8_t value);

	void ProcessEndFrame();

	SpcState& GetState();
	DspState& GetDspState();

	bool IsMuted();
	AddressInfo GetAbsoluteAddress(uint16_t addr);
	int GetRelativeAddress(AddressInfo & absAddress);

	uint8_t* GetSpcRam();
	uint8_t* GetSpcRom();

	void LoadSpcFile(SpcFileData* spcData);

	void Serialize(Serializer &s) override;

#ifdef DUMMYSPC
private:
	uint32_t _memOpCounter = 0;
	MemoryOperationInfo _memOperations[10] = {};

	void LogMemoryOperation(uint32_t addr, uint8_t value, MemoryOperationType type);

public:
	DummySpc(uint8_t *spcRam);

	void Step();

	void SetDummyState(SpcState &state);

	uint32_t GetOperationCount();
	MemoryOperationInfo GetOperationInfo(uint32_t index);
#endif
};

#endif
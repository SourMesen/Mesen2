#if (defined(DUMMYCPU) && !defined(__DUMMYCPU__H)) || (!defined(DUMMYCPU) && !defined(__CPU__H))
#ifdef DUMMYCPU
#define __DUMMYCPU__H
#else
#define __CPU__H
#endif

#include "pch.h"
#include "SNES/SnesCpuTypes.h"
#include "Utilities/ISerializable.h"
#include "Shared/MemoryOperationType.h"

class MemoryMappings;
class SnesMemoryManager;
class SnesDmaController;
class SnesConsole;
class Emulator;

class SnesCpu : public ISerializable
{
private:
	static constexpr uint32_t NmiVector = 0x00FFEA;
	static constexpr uint32_t ResetVector = 0x00FFFC;
	static constexpr uint32_t IrqVector = 0x00FFEE;
	static constexpr uint32_t AbortVector = 0x00FFE8;
	static constexpr uint32_t BreakVector = 0x00FFE6;
	static constexpr uint32_t CoprocessorVector = 0x00FFE4;

	static constexpr uint16_t LegacyNmiVector = 0xFFFA;
	static constexpr uint32_t LegacyIrqVector = 0xFFFE;
	static constexpr uint32_t LegacyCoprocessorVector = 0x00FFF4;

	typedef void(SnesCpu::*Func)();
	
	SnesMemoryManager *_memoryManager = nullptr;
	SnesDmaController *_dmaController = nullptr;
	Emulator *_emu = nullptr;
	SnesConsole *_console = nullptr;

	bool _immediateMode = false;
	uint32_t _readWriteMask = 0xFFFFFF;

	SnesCpuState _state = {};
	uint32_t _operand = 0;
	bool _waiOver = true;

	uint32_t GetProgramAddress(uint16_t addr);
	uint32_t GetDataAddress(uint16_t addr);

	uint16_t GetDirectAddress(uint16_t offset, bool allowEmulationMode = true);

	uint16_t GetDirectAddressIndirectWord(uint16_t offset);
	uint16_t GetDirectAddressIndirectWordWithPageWrap(uint16_t offset);
	uint32_t GetDirectAddressIndirectLong(uint16_t offset);
	
	uint8_t GetOpCode();
	
	uint16_t GetResetVector();

	void ProcessCpuCycle();

	void Idle();
	void IdleOrDummyWrite(uint32_t addr, uint8_t value);
	void IdleOrRead();
	void IdleEndJump();
	void IdleTakeBranch();
	
	uint8_t ReadOperandByte();
	uint16_t ReadOperandWord();
	uint32_t ReadOperandLong();

	uint16_t ReadVector(uint16_t vector);

	uint8_t Read(uint32_t addr, MemoryOperationType type);

	void SetSP(uint16_t sp, bool allowEmulationMode = true);
	__forceinline void RestrictStackPointerValue();
	void SetPS(uint8_t ps);

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
	void WriteWordRmw(uint32_t addr, uint16_t value, MemoryOperationType type = MemoryOperationType::Write);

	uint8_t GetByteValue();

	uint16_t GetWordValue();

	void PushByte(uint8_t value, bool allowEmulationMode = true);
	uint8_t PopByte(bool allowEmulationMode = true);

	void PushWord(uint16_t value, bool allowEmulationMode = true);
	uint16_t PopWord(bool allowEmulationMode = true);

	//Add/subtract instructions
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

	void DEC_Acc();
	void INC_Acc();

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

	void JMP_AbsIdxXInd();
	void JSR_AbsIdxXInd();

	//Interrupts
	void ProcessInterrupt(uint16_t vector, bool forHardwareInterrupt);
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
	void ASL_Acc();
	void ASL();
	void LSR_Acc();
	void LSR();
	void ROL_Acc();
	void ROL();
	void ROR_Acc();
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

	//Addressing modes
	//Absolute: a
	void AddrMode_Abs();
	//Absolute Indexed: a,x
	void AddrMode_AbsIdxX(bool isWrite);
	//Absolute Indexed: a,y
	void AddrMode_AbsIdxY(bool isWrite);
	//Absolute Long: al
	void AddrMode_AbsLng();
	//Absolute Long Indexed: al,x
	void AddrMode_AbsLngIdxX();

	void AddrMode_AbsJmp();
	void AddrMode_AbsLngJmp();
	void AddrMode_AbsInd(); //JMP only
	void AddrMode_AbsIndLng(); //JML only

	void AddrMode_Acc();

	void AddrMode_BlkMov();

	uint8_t ReadDirectOperandByte();
	
	//Direct: d
	void AddrMode_Dir();
	//Direct Indexed: d,x
	void AddrMode_DirIdxX();
	//Direct Indexed: d,y
	void AddrMode_DirIdxY();
	//Direct Indirect: (d)
	void AddrMode_DirInd();
	
	//Direct Indexed Indirect: (d,x)
	void AddrMode_DirIdxIndX();
	//Direct Indirect Indexed: (d),y
	void AddrMode_DirIndIdxY(bool isWrite);
	//Direct Indirect Long: [d]
	void AddrMode_DirIndLng();
	//Direct Indirect Indexed Long: [d],y
	void AddrMode_DirIndLngIdxY();

	void AddrMode_Imm8();
	void AddrMode_Imm16();
	void AddrMode_ImmX();
	void AddrMode_ImmM();

	void AddrMode_Imp();

	void AddrMode_RelLng();
	void AddrMode_Rel();

	void AddrMode_StkRel();
	void AddrMode_StkRelIndIdxY();
	
	__forceinline void RunOp();
	__noinline void ProcessHaltedState();
	__forceinline void CheckForInterrupts();

public:
#ifndef DUMMYCPU
	SnesCpu(SnesConsole *console);
#else
	DummySnesCpu(SnesConsole* console, CpuType type);
#endif

	virtual ~SnesCpu();

	void PowerOn();

	void Reset();
	void Exec();

	SnesCpuState& GetState();
	uint64_t GetCycleCount();

	template<uint64_t value>
	void IncreaseCycleCount();

	void SetNmiFlag(uint8_t delay);
	void DetectNmiSignalEdge();

	void SetIrqSource(SnesIrqSource source);
	bool CheckIrqSource(SnesIrqSource source);
	void ClearIrqSource(SnesIrqSource source);

	// Inherited via ISerializable
	void Serialize(Serializer &s) override;

#ifdef DUMMYCPU
private:
	MemoryMappings* _memoryMappings = nullptr;

	uint32_t _memOpCounter = 0;
	MemoryOperationInfo _memOperations[10] = {};

	void LogMemoryOperation(uint32_t addr, uint8_t value, MemoryOperationType type);

public:
	void SetDummyState(SnesCpuState &state);
	int32_t GetLastOperand();

	uint32_t GetOperationCount();
	MemoryOperationInfo GetOperationInfo(uint32_t index);
#endif
};

template<uint64_t count>
void SnesCpu::IncreaseCycleCount()
{
	_state.CycleCount += count;
}

#endif
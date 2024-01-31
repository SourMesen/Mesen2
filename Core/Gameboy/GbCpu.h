#if (defined(DUMMYCPU) && !defined(__DUMMYGBCPU__H)) || (!defined(DUMMYCPU) && !defined(__GBCPU__H))
#ifdef DUMMYCPU
#define __DUMMYGBCPU__H
#else
#define __GBCPU__H
#endif

#include "pch.h"
#include "Gameboy/GbTypes.h"
#include "Debugger/DebugTypes.h"
#include "Utilities/ISerializable.h"

class GbMemoryManager;
class Gameboy;
class GbPpu;
class Emulator;

class GbCpu : public ISerializable
{
private:
	GbCpuState _state = {};
	Register16 _regAF = Register16(&_state.A, &_state.Flags);
	Register16 _regBC = Register16(&_state.B, &_state.C);
	Register16 _regDE = Register16(&_state.D, &_state.E);
	Register16 _regHL = Register16(&_state.H, &_state.L);

	GbMemoryManager* _memoryManager = nullptr;
	Emulator* _emu = nullptr;
	Gameboy* _gameboy = nullptr;
	GbPpu* _ppu = nullptr;

	uint8_t _prevIrqVector = 0;

	void ExecOpCode(uint8_t opCode);

	void ProcessCgbSpeedSwitch();
	__noinline void ProcessHaltBug();

	__forceinline void ExecCpuCycle();
	__forceinline void ExecMasterCycle();
	__forceinline uint8_t ReadOpCode();
	__forceinline uint8_t ReadCode();
	__forceinline uint16_t ReadCodeWord();

	template<GbOamCorruptionType oamCorruptionType = GbOamCorruptionType::Read>
	__forceinline uint8_t Read(uint16_t addr);

	__forceinline void Write(uint16_t addr, uint8_t value);

	bool CheckFlag(uint8_t flag);
	void SetFlag(uint8_t flag);
	void ClearFlag(uint8_t flag);
	void SetFlagState(uint8_t flag, bool state);

	void PushByte(uint8_t value);
	void PushWord(uint16_t value);
	uint16_t PopWord();

	void LD(uint8_t& dst, uint8_t value);
	void LD(uint16_t& dst, uint16_t value);
	void LD(Register16& dst, uint16_t value);
	void LD_Indirect(uint16_t dst, uint8_t value);
	void LD_Indirect16(uint16_t dst, uint16_t value);
	void LD_HL(int8_t value);

	void INC(uint8_t& dst);
	void INC(Register16& dst);
	void INC_SP();
	void INC_Indirect(uint16_t addr);
	void DEC(uint8_t& dst);
	void DEC(Register16& dst);
	void DEC_Indirect(uint16_t addr);
	void DEC_SP();

	void ADD(uint8_t value);
	void ADD_SP(int8_t value);
	void ADD(Register16& reg, uint16_t value);
	void ADC(uint8_t value);
	void SUB(uint8_t value);
	void SBC(uint8_t value);

	void AND(uint8_t value);
	void OR(uint8_t value);
	void XOR(uint8_t value);
	
	void CP(uint8_t value);

	void NOP();
	void InvalidOp();
	void STOP();
	void HALT();

	void CPL();

	void RL(uint8_t& dst);
	void RL_Indirect(uint16_t addr);
	void RLC(uint8_t& dst);
	void RLC_Indirect(uint16_t addr);
	void RR(uint8_t& dst);
	void RR_Indirect(uint16_t addr);
	void RRC(uint8_t& dst);
	void RRC_Indirect(uint16_t addr);
	void RRA();
	void RRCA();
	void RLCA();
	void RLA();
	void SRL(uint8_t& dst);
	void SRL_Indirect(uint16_t addr);
	void SRA(uint8_t& dst);
	void SRA_Indirect(uint16_t addr);
	void SLA(uint8_t& dst);
	void SLA_Indirect(uint16_t addr);

	void SWAP(uint8_t& dst);
	void SWAP_Indirect(uint16_t addr);

	template<MemoryOperationType type, GbOamCorruptionType oamCorruptionType>
	uint8_t ReadMemory(uint16_t addr);

	template<uint8_t bit>
	void BIT(uint8_t src);

	template<uint8_t bit>
	void RES(uint8_t& dst);

	template<uint8_t bit>
	void RES_Indirect(uint16_t addr);

	template<uint8_t bit>
	void SET(uint8_t& dst);

	template<uint8_t bit>
	void SET_Indirect(uint16_t addr);

	void DAA();

	void JP(uint16_t dstAddr);
	void JP_HL();
	void JP(bool condition, uint16_t dstAddr);
	void JR(int8_t offset);
	void JR(bool condition, int8_t offset);

	void CALL(uint16_t dstAddr);
	void CALL(bool condition, uint16_t dstAddr);
	void RET();
	void RET(bool condition);
	void RETI();
	void RST(uint8_t value);
	
	void POP(Register16& reg);
	void PUSH(Register16& reg);
	void POP_AF();

	void SCF();
	void CCF();

	void EI();
	void DI();
	void PREFIX();
	
	__forceinline void ProcessNextCycleStart();
	__noinline bool HandleStoppedState();

public:
	virtual ~GbCpu();

	void Init(Emulator* emu, Gameboy* gameboy, GbMemoryManager* memoryManager);

	GbCpuState& GetState();
	bool IsHalted();

	uint64_t GetCycleCount() { return _state.CycleCount; }

	void Exec();
	void PowerOn();

	void Serialize(Serializer& s) override;

#ifdef DUMMYCPU
private:
	uint32_t _memOpCounter = 0;
	MemoryOperationInfo _memOperations[10] = {};

public:
	void SetDummyState(GbCpuState& state);
	uint32_t GetOperationCount();
	void LogMemoryOperation(uint32_t addr, uint8_t value, MemoryOperationType type);
	MemoryOperationInfo GetOperationInfo(uint32_t index);
#endif
};
#endif

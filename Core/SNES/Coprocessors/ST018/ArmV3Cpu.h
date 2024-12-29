#if (defined(DUMMYCPU) && !defined(__DUMMYARMV3CPU__H)) || (!defined(DUMMYCPU) && !defined(__ARMV3CPU__H))
#ifdef DUMMYCPU
#define __DUMMYARMV3CPU__H
#else
#define __ARMV3CPU__H
#endif

#include "pch.h"
#include "SNES/Coprocessors/ST018/ArmV3Types.h"
#include "Shared/Emulator.h"
#include "Shared/ArmEnums.h"
#include "Debugger/DebugTypes.h"
#include "Utilities/ISerializable.h"

class Emulator;
class St018;

class ArmV3Cpu : public ISerializable
{
private:
	uint32_t _opCode = 0;
	ArmV3CpuState _state = {};

	Emulator* _emu = nullptr;
	St018* _st018 = nullptr;

	typedef void(ArmV3Cpu::* Func)();
	static Func _armTable[0x1000];
	static ArmOpCategory _armCategory[0x1000];

	uint32_t Add(uint32_t op1, uint32_t op2, bool carry, bool updateFlags);
	uint32_t Sub(uint32_t op1, uint32_t op2, bool carry, bool updateFlags);
	uint32_t LogicalOp(uint32_t result, bool carry, bool updateFlags);

	uint32_t RotateRight(uint32_t value, uint32_t shift);
	uint32_t RotateRight(uint32_t value, uint32_t shift, bool& carry);

	uint32_t ShiftLsl(uint32_t value, uint8_t shift, bool& carry);
	uint32_t ShiftLsr(uint32_t value, uint8_t shift, bool& carry);
	uint32_t ShiftAsr(uint32_t value, uint8_t shift, bool& carry);
	uint32_t ShiftRor(uint32_t value, uint8_t shift, bool& carry);
	uint32_t ShiftRrx(uint32_t value, bool& carry);

	uint32_t R(uint8_t reg);
	void SetR(uint8_t reg, uint32_t value)
	{
		_state.R[reg] = value;
		if(reg == 15) {
			_state.Pipeline.ReloadRequested = true;
		}
	}

	ArmV3CpuFlags& GetSpsr();

	static void InitArmOpTable();
	void ArmBranch();
	void ArmMsr();
	void ArmMrs();
	void ArmDataProcessing();
	void ArmMultiply();
	void ArmMultiplyLong();
	void ArmSingleDataTransfer();
	void ArmBlockDataTransfer();
	void ArmSingleDataSwap();
	void ArmSoftwareInterrupt();
	void ArmInvalidOp();

	bool CheckConditions(uint32_t condCode);

	void SwitchMode(ArmV3CpuMode mode);

	void ReloadPipeline();

	__forceinline void ProcessPipeline()
	{
		ArmV3CpuPipeline& pipe = _state.Pipeline;

		if(pipe.ReloadRequested) {
			ReloadPipeline();
		}

		pipe.Execute = pipe.Decode;
		pipe.Decode = pipe.Fetch;

		pipe.Fetch.Address = _state.R[15] = (_state.R[15] + 4);
		pipe.Fetch.OpCode = ReadCode(pipe.Mode, pipe.Fetch.Address);
	}

	uint32_t ReadCode(ArmV3AccessModeVal mode, uint32_t addr);
	__forceinline uint32_t Read(ArmV3AccessModeVal mode, uint32_t addr);
	__forceinline void Write(ArmV3AccessModeVal mode, uint32_t addr, uint32_t value);
	void Idle();
	void Idle(uint8_t cycleCount);

	void ProcessException(ArmV3CpuMode mode, ArmV3CpuVector vector);

public:
	virtual ~ArmV3Cpu();

	static void StaticInit();

	void Init(Emulator* emu, St018* st018);

	static ArmOpCategory GetArmOpCategory(uint32_t opCode);

	ArmV3CpuState& GetState();
	uint32_t GetProgramCounter() { return _state.R[15]; }
	void SetProgramCounter(uint32_t addr);

	__forceinline void Exec()
	{
#ifndef DUMMYCPU
		_emu->ProcessInstruction<CpuType::St018>();
#endif

		_opCode = _state.Pipeline.Execute.OpCode;
#ifndef DUMMYCPU
		if(CheckConditions(_opCode >> 28)) {
#else 
		{
#endif
			uint16_t opType = ((_opCode & 0x0FF00000) >> 16) | ((_opCode & 0xF0) >> 4);
			(this->*_armTable[opType])();
		}

#ifndef DUMMYCPU
		ProcessPipeline();
#endif
	}

	void PowerOn(bool forReset);

	void Serialize(Serializer & s) override;

#ifdef DUMMYCPU
private:
	uint32_t _memOpCounter = 0;
	MemoryOperationInfo _memOperations[32] = {};
	ArmV3AccessModeVal _memAccessMode[32] = {};

public:
	void SetDummyState(ArmV3CpuState & state);
	uint32_t GetOperationCount();
	void LogMemoryOperation(uint32_t addr, uint32_t value, ArmV3AccessModeVal mode, MemoryOperationType type);
	MemoryOperationInfo GetOperationInfo(uint32_t index);
	ArmV3AccessModeVal GetOperationMode(uint32_t index);
#endif
	};
#endif

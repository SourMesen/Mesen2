#if (defined(DUMMYCPU) && !defined(__DUMMYGBACPU__H)) || (!defined(DUMMYCPU) && !defined(__GBACPU__H))
#ifdef DUMMYCPU
#define __DUMMYGBACPU__H
#else
#define __GBACPU__H
#endif

#include "pch.h"
#include "GBA/GbaTypes.h"
#include "GBA/GbaMemoryManager.h"
#include "Shared/Emulator.h"
#include "Debugger/DebugTypes.h"
#include "Utilities/ISerializable.h"

class GbaMemoryManager;
class GbaRomPrefetch;
class Emulator;

class GbaCpu : public ISerializable
{
private:
	uint32_t _opCode = 0;
	GbaCpuState _state = {};

	GbaMemoryManager* _memoryManager = nullptr;
	GbaRomPrefetch* _prefetch = nullptr;
	Emulator* _emu = nullptr;

	typedef void(GbaCpu::* Func)();
	static Func _armTable[0x1000];
	static Func _thumbTable[0x100];
	static ArmOpCategory _armCategory[0x1000];
	static GbaThumbOpCategory _thumbCategory[0x100];

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

	GbaCpuFlags& GetSpsr();

	static void InitArmOpTable();
	void ArmBranchExchangeRegister();
	void ArmBranch();
	void ArmMsr();
	void ArmMrs();
	void ArmDataProcessing();
	void ArmMultiply();
	void ArmMultiplyLong();
	void ArmSingleDataTransfer();
	void ArmSignedHalfDataTransfer();
	void ArmBlockDataTransfer();
	void ArmSingleDataSwap();
	void ArmSoftwareInterrupt();
	void ArmInvalidOp();

	bool CheckConditions(uint32_t condCode);

	static void InitThumbOpTable();
	void ThumbMoveShiftedRegister();
	void ThumbAddSubtract();
	void ThumbMoveCmpAddSub();
	void ThumbAluOperation();
	void ThumbHiRegBranchExch();
	void ThumbPcRelLoad();
	void ThumbLoadStoreRegOffset();
	void ThumbLoadStoreSignExtended();
	void ThumbLoadStoreImmOffset();
	void ThumbLoadStoreHalfWord();
	void ThumbSpRelLoadStore();
	void ThumbLoadAddress();
	void ThumbAddOffsetToSp();
	void ThumbPushPopReg();
	void ThumbMultipleLoadStore();
	void ThumbConditionalBranch();
	void ThumbSoftwareInterrupt();
	void ThumbUnconditionalBranch();
	void ThumbLongBranchLink();

	void SwitchMode(GbaCpuMode mode);

	void ReloadPipeline();

	__forceinline void ProcessPipeline()
	{
		GbaCpuPipeline& pipe = _state.Pipeline;

		if(pipe.ReloadRequested) {
			ReloadPipeline();
		}

		pipe.Execute = pipe.Decode;
		pipe.Decode = pipe.Fetch;

		pipe.Fetch.Address = _state.R[15] = _state.CPSR.Thumb ? (_state.R[15] + 2) : (_state.R[15] + 4);
		pipe.Fetch.OpCode = ReadCode(pipe.Mode, pipe.Fetch.Address);
	}

	uint32_t ReadCode(GbaAccessModeVal mode, uint32_t addr);
	uint32_t Read(GbaAccessModeVal mode, uint32_t addr);
	void Write(GbaAccessModeVal mode, uint32_t addr, uint32_t value);
	void Idle();
	void Idle(uint8_t cycleCount);

	void ProcessException(GbaCpuMode mode, GbaCpuVector vector);
	void CheckForIrqs();

public:
	virtual ~GbaCpu();

	static void StaticInit();

	void Init(Emulator* emu, GbaMemoryManager* memoryManager, GbaRomPrefetch* prefetch);

	static ArmOpCategory GetArmOpCategory(uint32_t opCode);
	static GbaThumbOpCategory GetThumbOpCategory(uint16_t opCode);

	GbaCpuState& GetState();
	uint32_t GetProgramCounter() { return _state.R[15]; }
	void SetProgramCounter(uint32_t addr, bool thumb);

	template<bool debuggerEnabled>
	__noinline bool ProcessHaltMode()
	{
		return InlineProcessHaltMode<debuggerEnabled>();
	}

	template<bool debuggerEnabled>
	__forceinline bool InlineProcessHaltMode()
	{
		if constexpr(debuggerEnabled) {
			_emu->ProcessHaltedCpu<CpuType::Gba>();
		}
		
		bool isHaltOver = _memoryManager->IsHaltOver();
		bool processIrq = !_state.CPSR.IrqDisable && _memoryManager->ProcessIrq();

		if(_memoryManager->IsSystemStopped()) {
			_memoryManager->ProcessStoppedCycle();
		} else {
			_memoryManager->ProcessInternalCycle<true>();
		}

		if(isHaltOver) {
			_memoryManager->ProcessInternalCycle<true>();
			_state.Stopped = false;
			if(processIrq) {
				CheckForIrqs();
			}
			return false;
		} else {
			return true;
		}
	}

	template<bool inlineHalt, bool debuggerEnabled>
	__forceinline void Exec()
	{
#ifndef DUMMYCPU
		if constexpr(inlineHalt) {
			if(_state.Stopped && InlineProcessHaltMode<debuggerEnabled>()) {
				return;
			}
		} else {
			if(_state.Stopped && ProcessHaltMode<debuggerEnabled>()) {
				return;
			}
		}

		if constexpr(debuggerEnabled) {
			_emu->ProcessInstruction<CpuType::Gba>();
		}
#endif

		_opCode = _state.Pipeline.Execute.OpCode;
		if(_state.CPSR.Thumb) {
			(this->*_thumbTable[(_opCode >> 8) & 0xFF])();
		} else {
#ifndef DUMMYCPU
			if(CheckConditions(_opCode >> 28)) {
#else 
			{
#endif
				uint16_t opType = ((_opCode & 0x0FF00000) >> 16) | ((_opCode & 0xF0) >> 4);
				(this->*_armTable[opType])();
			}
		}

#ifndef DUMMYCPU
		bool checkIrq = !_state.CPSR.IrqDisable && _memoryManager->ProcessIrq();
		ProcessPipeline();
		if(checkIrq) {
			CheckForIrqs();
		}
#endif
	}

	void SetStopFlag() { _state.Stopped = true; }
	void ClearSequentialFlag() { _state.Pipeline.Mode &= ~GbaAccessMode::Sequential; }
	void SetSequentialFlag() { _state.Pipeline.Mode |= GbaAccessMode::Sequential; }

	void PowerOn();

	void Serialize(Serializer& s) override;

#ifdef DUMMYCPU
private:
	uint32_t _memOpCounter = 0;
	MemoryOperationInfo _memOperations[32] = {};
	GbaAccessModeVal _memAccessMode[32] = {};

public:
	void SetDummyState(GbaCpuState& state);
	uint32_t GetOperationCount();
	void LogMemoryOperation(uint32_t addr, uint32_t value, GbaAccessModeVal mode, MemoryOperationType type);
	MemoryOperationInfo GetOperationInfo(uint32_t index);
	GbaAccessModeVal GetOperationMode(uint32_t index);
#endif
};
#endif

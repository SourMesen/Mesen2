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
	uint8_t _ldmGlitch = 0;

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

	__forceinline uint32_t R(uint8_t reg)
	{
		//Used for ARM mode, which can trigger the LDM^ glitch
		if(_ldmGlitch == 0 || reg == 15 || reg < 8) {
			return _state.R[reg];
		}

		return _state.R[reg] | _state.UserRegs[reg - 8];
	}

	__forceinline uint32_t RT(uint8_t reg)
	{
		//Thumb mode can't trigger the LDM glitch
		return _state.R[reg];
	}

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

	__forceinline bool CheckConditions(uint32_t condCode)
	{
		/*Code Suffix Flags Meaning
		0000 EQ Z set equal
		0001 NE Z clear not equal
		0010 CS C set unsigned higher or same
		0011 CC C clear unsigned lower
		0100 MI N set negative
		0101 PL N clear positive or zero
		0110 VS V set overflow
		0111 VC V clear no overflow
		1000 HI C set and Z clear unsigned higher
		1001 LS C clear or Z set unsigned lower or same
		1010 GE N equals V greater or equal
		1011 LT N not equal to V less than
		1100 GT Z clear AND(N equals V) greater than
		1101 LE Z set OR(N not equal to V) less than or equal
		1110 AL(ignored) always
		*/
		switch(condCode) {
			case 0: return _state.CPSR.Zero;
			case 1: return !_state.CPSR.Zero;
			case 2: return _state.CPSR.Carry;
			case 3: return !_state.CPSR.Carry;
			case 4: return _state.CPSR.Negative;
			case 5: return !_state.CPSR.Negative;
			case 6: return _state.CPSR.Overflow;
			case 7: return !_state.CPSR.Overflow;
			case 8: return _state.CPSR.Carry && !_state.CPSR.Zero;
			case 9: return !_state.CPSR.Carry || _state.CPSR.Zero;
			case 10: return _state.CPSR.Negative == _state.CPSR.Overflow;
			case 11: return _state.CPSR.Negative != _state.CPSR.Overflow;
			case 12: return !_state.CPSR.Zero && (_state.CPSR.Negative == _state.CPSR.Overflow);
			case 13: return _state.CPSR.Zero || (_state.CPSR.Negative != _state.CPSR.Overflow);
			case 14: return true;
			case 15: return false;
		}

		return true;
	}

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

		if(_memoryManager->IsSystemStopped()) {
			_memoryManager->ProcessStoppedCycle();
		} else {
			_memoryManager->ProcessInternalCycle<true>();
		}

		if(isHaltOver) {
			_memoryManager->ProcessInternalCycle<true>();
			_state.Stopped = false;
			return false;
		} else {
			return true;
		}
	}

	template<bool inlineHalt, bool debuggerEnabled>
	__forceinline void Exec()
	{
#ifndef DUMMYCPU
		//Check if DMA needs to be executed before running the next	instruction.
		//If a DMA is pending, it needs to start before the CPU tries to run the
		//next instruction instruction. This can impact the timing at which the IRQ is checked
		//(before the DMA vs after the DMA), which affects test results.
		//Additionally, it's possible for the DMA to turn on halt mode, so DMA needs to be
		//processed here, to ensure the CPU immediately enters halt mode after DMA is over,
		//instead of running the next instruction before halting.
		//This fixes the haltcnt test rom and the DMA Prefetch test in the mGBA Suite test rom
		_memoryManager->ProcessDma();

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

		uint64_t startClock = _memoryManager->GetMasterClock();
		bool irqDisable = _state.CPSR.IrqDisable;
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
		if(_state.Pipeline.ReloadRequested) {
			ReloadPipeline();
		}

		bool checkIrq = _memoryManager->ProcessIrq();
		if(checkIrq && startClock != _memoryManager->GetMasterClock()) {
			//TST, TEQ, CMP, CMN can modify the I flag without any fetch/idle cycles.
			//In that case, the IRQ handling is done using the original I flag before the instruction was executed. (Passes "psr" test)
			//Otherwise, use the current I flag.
			irqDisable = _state.CPSR.IrqDisable;
		}

		ProcessPipeline();
		if(!irqDisable && checkIrq) {
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

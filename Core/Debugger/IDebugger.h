#pragma once
#include "pch.h"
#include "Debugger/DebuggerFeatures.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/StepBackManager.h"
#include "Debugger/FrozenAddressManager.h"

enum class StepType;
class BreakpointManager;
class CallstackManager;
class IAssembler;
class BaseEventManager;
class CodeDataLogger;
class ITraceLogger;
class PpuTools;
class Emulator;
struct BaseState;
enum class EventType;
enum class MemoryOperationType;

//TODOv2 rename/refactor to BaseDebugger
class IDebugger
{
protected:
	unique_ptr<StepRequest> _step;
	unique_ptr<StepBackManager> _stepBackManager;

	FrozenAddressManager _frozenAddressManager;

public:
	bool IgnoreBreakpoints = false;
	bool AllowChangeProgramCounter = false;
	CpuInstructionProgress InstructionProgress = {};

	IDebugger(Emulator* emu) : _stepBackManager(new StepBackManager(emu, this)) {}
	virtual ~IDebugger() = default;

	StepRequest* GetStepRequest() { return _step.get(); }
	bool CheckStepBack() { return _stepBackManager->CheckStepBack(); }
	bool IsStepBack() { return _stepBackManager->IsRewinding(); }
	void ResetStepBackCache() { return _stepBackManager->ResetCache(); }
	void StepBack(int32_t stepCount) { return _stepBackManager->StepBack((StepBackType)stepCount); }
	virtual StepBackConfig GetStepBackConfig() { return { GetCpuCycleCount(), 0, 0 }; }

	FrozenAddressManager& GetFrozenAddressManager() { return _frozenAddressManager; }

	virtual void ResetPrevOpCode() {}

	virtual void Step(int32_t stepCount, StepType type) = 0;
	virtual void Reset() = 0;
	virtual void Run() = 0;
	
	virtual void Init() {}
	virtual void ProcessConfigChange() {}

	virtual void ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi) {}
	virtual void ProcessInputOverrides(DebugControllerState inputOverrides[8]) {}

	virtual void DrawPartialFrame() { }

	virtual DebuggerFeatures GetSupportedFeatures() { return {}; }
	virtual uint64_t GetCpuCycleCount(bool forProfiler = false) { return 0; }
	virtual uint32_t GetProgramCounter(bool getInstPc) = 0;
	virtual void SetProgramCounter(uint32_t addr, bool updateDebuggerOnly = false) = 0;

	virtual uint8_t GetCpuFlags() { return 0; }

	virtual BreakpointManager* GetBreakpointManager() = 0;
	virtual CallstackManager* GetCallstackManager() = 0;
	virtual IAssembler* GetAssembler() = 0;
	virtual BaseEventManager* GetEventManager() = 0;
	virtual ITraceLogger* GetTraceLogger() = 0;
	virtual PpuTools* GetPpuTools() { return nullptr; }

	virtual void GetRomHeader(uint8_t* headerData, uint32_t& size) {}

	virtual BaseState& GetState() = 0;
	virtual void GetPpuState(BaseState& state) {};
	virtual void SetPpuState(BaseState& state) {};
};

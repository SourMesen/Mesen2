#pragma once
#include "pch.h"
#include "Debugger/DebuggerFeatures.h"
#include "Debugger/DebugTypes.h"

enum class StepType;
class BreakpointManager;
class CallstackManager;
class IAssembler;
class BaseEventManager;
class CodeDataLogger;
class ITraceLogger;
class PpuTools;
struct BaseState;
enum class EventType;
enum class MemoryOperationType;

class IDebugger
{
protected:
	unique_ptr<StepRequest> _step;

public:
	bool IgnoreBreakpoints = false;
	bool AllowChangeProgramCounter = false;
	CpuInstructionProgress InstructionProgress = {};

	virtual ~IDebugger() = default;

	StepRequest* GetStepRequest() { return _step.get(); }

	virtual void Step(int32_t stepCount, StepType type) = 0;
	virtual void Reset() = 0;
	virtual void Run() = 0;
	
	virtual void Init() {}
	virtual void ProcessConfigChange() {}

	virtual void ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi) {}
	virtual void ProcessInputOverrides(DebugControllerState inputOverrides[8]) {}

	virtual void DrawPartialFrame() { }

	virtual DebuggerFeatures GetSupportedFeatures() { return {}; }
	virtual uint64_t GetCpuCycleCount() { return 0; }
	virtual uint32_t GetProgramCounter(bool getInstPc) = 0;
	virtual void SetProgramCounter(uint32_t addr) = 0;

	virtual BreakpointManager* GetBreakpointManager() = 0;
	virtual CallstackManager* GetCallstackManager() = 0;
	virtual IAssembler* GetAssembler() = 0;
	virtual BaseEventManager* GetEventManager() = 0;
	virtual ITraceLogger* GetTraceLogger() = 0;
	virtual PpuTools* GetPpuTools() { return nullptr; }

	virtual BaseState& GetState() = 0;
	virtual void GetPpuState(BaseState& state) {};
	virtual void SetPpuState(BaseState& state) {};
};

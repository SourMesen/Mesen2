#pragma once
#include "stdafx.h"

enum class StepType;
class BreakpointManager;
class CallstackManager;
class IAssembler;
class IEventManager;
class CodeDataLogger;
class ITraceLogger;
class PpuTools;
struct BaseState;
enum class EventType;
enum class MemoryOperationType;

class IDebugger
{
public:
	virtual ~IDebugger() = default;

	virtual void Step(int32_t stepCount, StepType type) = 0;
	virtual void Reset() = 0;
	virtual void Run() = 0;
	
	virtual void Init() {}

	virtual void ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi) {}

	virtual BreakpointManager* GetBreakpointManager() = 0;
	virtual shared_ptr<CallstackManager> GetCallstackManager() = 0;
	virtual shared_ptr<IAssembler> GetAssembler() = 0;
	virtual shared_ptr<IEventManager> GetEventManager() = 0;
	virtual shared_ptr<CodeDataLogger> GetCodeDataLogger() = 0;
	virtual ITraceLogger* GetTraceLogger() = 0;
	virtual PpuTools* GetPpuTools() { return nullptr; }

	virtual BaseState& GetState() = 0;
	virtual void GetPpuState(BaseState& state) {};
};
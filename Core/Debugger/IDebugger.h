#pragma once
#include "stdafx.h"

enum class StepType;
class BreakpointManager;
class CallstackManager;
class IAssembler;
class IEventManager;
class CodeDataLogger;
struct BaseState;
enum class MemoryOperationType;

class IDebugger
{
public:
	virtual void Step(int32_t stepCount, StepType type) = 0;
	virtual void Reset() = 0;
	virtual void Run() = 0;

	virtual void ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType opType) {}
	virtual void ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType opType) {}
	virtual void ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi) {}
	virtual void ProcessPpuCycle(uint16_t& cycle, uint16_t& scanline) {}

	virtual BreakpointManager* GetBreakpointManager() = 0;
	virtual shared_ptr<CallstackManager> GetCallstackManager() = 0;
	virtual shared_ptr<IAssembler> GetAssembler() = 0;
	virtual shared_ptr<IEventManager> GetEventManager() = 0;
	virtual shared_ptr<CodeDataLogger> GetCodeDataLogger() = 0;

	virtual BaseState& GetState() = 0;
};
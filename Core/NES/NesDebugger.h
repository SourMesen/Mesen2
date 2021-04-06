#pragma once
#include "stdafx.h"
#include "IDebugger.h"

class Debugger;
class BreakpointManager;

class NesDebugger : public IDebugger
{
private:
	unique_ptr<BreakpointManager> _breakpointManager;

public:
	NesDebugger(Debugger* debugger);
	virtual ~NesDebugger();

	// Inherited via IDebugger
	virtual void Step(int32_t stepCount, StepType type) override;
	virtual void Reset() override;
	virtual void Run() override;
	virtual BreakpointManager* GetBreakpointManager() override;
	virtual shared_ptr<CallstackManager> GetCallstackManager() override;
	virtual shared_ptr<IAssembler> GetAssembler() override;
	virtual shared_ptr<IEventManager> GetEventManager() override;
	virtual shared_ptr<CodeDataLogger> GetCodeDataLogger() override;
};
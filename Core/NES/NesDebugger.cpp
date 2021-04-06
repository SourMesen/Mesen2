#include "stdafx.h"
#include "NesDebugger.h"
#include "BreakpointManager.h"
#include "ExpressionEvaluator.h"

NesDebugger::NesDebugger(Debugger* debugger)
{
	_breakpointManager.reset(new BreakpointManager(debugger, CpuType::Nes));
}

NesDebugger::~NesDebugger()
{
}

void NesDebugger::Step(int32_t stepCount, StepType type)
{
}

void NesDebugger::Reset()
{
}

void NesDebugger::Run()
{
}

BreakpointManager* NesDebugger::GetBreakpointManager()
{
    return _breakpointManager.get();
}

shared_ptr<CallstackManager> NesDebugger::GetCallstackManager()
{
    return shared_ptr<CallstackManager>();
}

shared_ptr<IAssembler> NesDebugger::GetAssembler()
{
    return shared_ptr<IAssembler>();
}

shared_ptr<IEventManager> NesDebugger::GetEventManager()
{
    return shared_ptr<IEventManager>();
}

shared_ptr<CodeDataLogger> NesDebugger::GetCodeDataLogger()
{
    return shared_ptr<CodeDataLogger>();
}

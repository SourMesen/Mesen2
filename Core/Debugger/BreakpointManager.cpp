#include "stdafx.h"
#include "BreakpointManager.h"
#include "DebugTypes.h"
#include "Debugger.h"
#include "Breakpoint.h"
#include "DebugUtilities.h"
#include "ExpressionEvaluator.h"
#include "IEventManager.h"
#include "MemoryOperationType.h"

BreakpointManager::BreakpointManager(Debugger *debugger, CpuType cpuType, IEventManager* eventManager)
{
	_debugger = debugger;
	_cpuType = cpuType;
	_hasBreakpoint = false;
	
	//TODO
	//_eventManager = eventManager ? eventManager : debugger->GetEventManager(CpuType::Cpu).get();
}

void BreakpointManager::SetBreakpoints(Breakpoint breakpoints[], uint32_t count)
{
	_hasBreakpoint = false;
	for(int i = 0; i < BreakpointManager::BreakpointTypeCount; i++) {
		_breakpoints[i].clear();
		_rpnList[i].clear();
		_hasBreakpointType[i] = false;
	}

	_bpExpEval.reset(new ExpressionEvaluator(_debugger, _cpuType));

	for(uint32_t j = 0; j < count; j++) {
		Breakpoint &bp = breakpoints[j];
		for(int i = 0; i < BreakpointManager::BreakpointTypeCount; i++) {
			BreakpointType bpType = (BreakpointType)i;
			if((bp.IsMarked() || bp.IsEnabled()) && bp.HasBreakpointType(bpType)) {
				CpuType cpuType = bp.GetCpuType();
				if(_cpuType != cpuType) {
					continue;
				}

				_breakpoints[i].push_back(bp);

				if(bp.HasCondition()) {
					bool success = true;
					ExpressionData data = _bpExpEval->GetRpnList(bp.GetCondition(), success);
					_rpnList[i].push_back(success ? data : ExpressionData());
				} else {
					_rpnList[i].push_back(ExpressionData());
				}
				
				_hasBreakpoint = true;
				_hasBreakpointType[i] = true;
			}
		}
	}
}

BreakpointType BreakpointManager::GetBreakpointType(MemoryOperationType type)
{
	switch(type) {
		default:
		case MemoryOperationType::ExecOperand:
		case MemoryOperationType::ExecOpCode:
			return BreakpointType::Execute;

		case MemoryOperationType::DmaRead:
		case MemoryOperationType::Read:
			return BreakpointType::Read;

		case MemoryOperationType::DmaWrite:
		case MemoryOperationType::Write:
			return BreakpointType::Write;
	}
}

int BreakpointManager::InternalCheckBreakpoint(MemoryOperationInfo operationInfo, AddressInfo &address)
{
	BreakpointType type = GetBreakpointType(operationInfo.Type);

	if(!_hasBreakpointType[(int)type]) {
		return -1;
	}

	BaseState& state =_debugger->GetStateRef(_cpuType);
	EvalResultType resultType;
	vector<Breakpoint> &breakpoints = _breakpoints[(int)type];
	for(size_t i = 0; i < breakpoints.size(); i++) {
		if(breakpoints[i].Matches(operationInfo.Address, address)) {
			if(!breakpoints[i].HasCondition() || _bpExpEval->Evaluate(_rpnList[(int)type][i], state, resultType, operationInfo)) {
				if(breakpoints[i].IsMarked()) {
					//TODO
					//_eventManager->AddEvent(DebugEventType::Breakpoint, operationInfo, breakpoints[i].GetId());
				}
				if(breakpoints[i].IsEnabled()) {
					return breakpoints[i].GetId();
				}
			}
		}
	}

	return -1;
}

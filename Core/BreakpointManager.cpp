#include "stdafx.h"
#include "BreakpointManager.h"
#include "DebugTypes.h"
#include "Debugger.h"
#include "Breakpoint.h"
#include "ExpressionEvaluator.h"

BreakpointManager::BreakpointManager(Debugger *debugger)
{
	_debugger = debugger;
}

void BreakpointManager::SetBreakpoints(Breakpoint breakpoints[], uint32_t count)
{
	for(int j = 0; j < (int)CpuType::Sa1 + 1; j++) {
		for(int i = 0; i < BreakpointManager::BreakpointTypeCount; i++) {
			_breakpoints[j][i].clear();
			_rpnList[j][i].clear();
			_hasBreakpoint[j][i] = false;
		}
	}

	_bpExpEval[(int)CpuType::Cpu].reset(new ExpressionEvaluator(_debugger, CpuType::Cpu));
	_bpExpEval[(int)CpuType::Spc].reset(new ExpressionEvaluator(_debugger, CpuType::Spc));

	for(uint32_t j = 0; j < count; j++) {
		Breakpoint &bp = breakpoints[j];
		for(int i = 0; i < BreakpointManager::BreakpointTypeCount; i++) {
			BreakpointType bpType = (BreakpointType)i;
			if((bp.IsMarked() || bp.IsEnabled()) && bp.HasBreakpointType(bpType)) {
				CpuType cpuType = bp.GetCpuType();
				_breakpoints[(int)cpuType][i].push_back(bp);

				if(bp.HasCondition()) {
					bool success = true;
					ExpressionData data = _bpExpEval[(int)cpuType]->GetRpnList(bp.GetCondition(), success);
					_rpnList[(int)cpuType][i].push_back(success ? data : ExpressionData());
				} else {
					_rpnList[(int)cpuType][i].push_back(ExpressionData());
				}
				
				_hasBreakpoint[(int)cpuType][i] = true;
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

int BreakpointManager::CheckBreakpoint(CpuType cpuType, MemoryOperationInfo operationInfo, AddressInfo &address)
{
	BreakpointType type = GetBreakpointType(operationInfo.Type);

	if(!_hasBreakpoint[(int)cpuType][(int)type]) {
		return -1;
	}

	DebugState state;
	_debugger->GetState(state, false);
	EvalResultType resultType;
	vector<Breakpoint> &breakpoints = _breakpoints[(int)cpuType][(int)type];
	for(size_t i = 0; i < breakpoints.size(); i++) {
		if(breakpoints[i].Matches(operationInfo.Address, address)) {
			if(!breakpoints[i].HasCondition() || _bpExpEval[(int)cpuType]->Evaluate(_rpnList[(int)cpuType][(int)type][i], state, resultType, operationInfo)) {
				return breakpoints[i].GetId();
			}
		}
	}

	return -1;
}

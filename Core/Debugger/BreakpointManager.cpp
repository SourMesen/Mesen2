#include "pch.h"
#include "Debugger/BreakpointManager.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/Debugger.h"
#include "Debugger/Breakpoint.h"
#include "Debugger/DebugUtilities.h"
#include "Debugger/ExpressionEvaluator.h"
#include "Debugger/BaseEventManager.h"
#include "Shared/MemoryOperationType.h"

BreakpointManager::BreakpointManager(Debugger *debugger, IDebugger* cpuDebugger, CpuType cpuType, BaseEventManager* eventManager)
{
	_debugger = debugger;
	_cpuDebugger = cpuDebugger;
	_cpuType = cpuType;
	_hasBreakpoint = false;
	
	_eventManager = eventManager;
}

void BreakpointManager::SetBreakpoints(Breakpoint breakpoints[], uint32_t count)
{
	_hasBreakpoint = false;
	for(int i = 0; i < BreakpointManager::BreakpointTypeCount; i++) {
		_breakpoints[i].clear();
		_rpnList[i].clear();
		_hasBreakpointType[i] = false;
	}

	_forbidBreakpoints.clear();
	_forbidRpn.clear();

	_bpExpEval.reset(new ExpressionEvaluator(_debugger, _cpuDebugger, _cpuType));

	for(uint32_t j = 0; j < count; j++) {
		Breakpoint &bp = breakpoints[j];
		if(bp.HasBreakpointType(BreakpointType::Forbid)) {
			if(_cpuType == bp.GetCpuType() && bp.IsEnabled()) {
				if(bp.HasCondition()) {
					bool success = true;
					ExpressionData data = _bpExpEval->GetRpnList(bp.GetCondition(), success);
					_forbidRpn.push_back(success ? data : ExpressionData());
				} else {
					_forbidRpn.push_back(ExpressionData());
				}
				_forbidBreakpoints.push_back(bp);
			}
			continue;
		}

		for(int i = 0; i < BreakpointManager::BreakpointTypeCount; i++) {
			MemoryOperationType opType = (MemoryOperationType)i;
			if((bp.IsMarked() || bp.IsEnabled()) && bp.HasBreakpointType(GetBreakpointType(opType))) {
				CpuType cpuType = bp.GetCpuType();
				if(_cpuType != cpuType) {
					continue;
				}

				if(bp.IsAllowedForOpType(opType)) {
					_breakpoints[i].push_back(bp);
				}

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

bool BreakpointManager::IsForbidden(MemoryOperationInfo* memoryOpPtr, AddressInfo& relAddr, AddressInfo& absAddr)
{
	MemoryOperationInfo memoryOp = memoryOpPtr != nullptr ? *memoryOpPtr : MemoryOperationInfo {};

	MemoryOperationInfo op;
	op.Address = relAddr.Address;
	op.MemType = relAddr.Type;
	for(size_t i = 0, len = _forbidBreakpoints.size(); i < len; i++) {
		if(_forbidBreakpoints[i].Matches(op, absAddr)) {
			EvalResultType resultType;
			if(_forbidBreakpoints[i].HasCondition() && !_bpExpEval->Evaluate(_forbidRpn[i], resultType, memoryOp, absAddr)) {
				continue;
			}

			return true;
		}
	}

	return false;
}

BreakpointType BreakpointManager::GetBreakpointType(MemoryOperationType type)
{
	switch(type) {
		case MemoryOperationType::ExecOperand:
		case MemoryOperationType::ExecOpCode:
			return BreakpointType::Execute;

		case MemoryOperationType::DmaRead:
		case MemoryOperationType::Read:
		case MemoryOperationType::DummyRead:
		case MemoryOperationType::PpuRenderingRead:
			return BreakpointType::Read;

		case MemoryOperationType::DmaWrite:
		case MemoryOperationType::Write:
		case MemoryOperationType::DummyWrite:
			return BreakpointType::Write;

		default:
			throw std::runtime_error("Unsupported memory operation type");
	}
}

template<uint8_t accessWidth>
int BreakpointManager::InternalCheckBreakpoint(MemoryOperationInfo operationInfo, AddressInfo &address, bool processMarkedBreakpoints)
{
	EvalResultType resultType;
	vector<Breakpoint> &breakpoints = _breakpoints[(int)operationInfo.Type];
	for(size_t i = 0, len = breakpoints.size(); i < len; i++) {
		if(breakpoints[i].Matches<accessWidth>(operationInfo, address)) {
			if(breakpoints[i].HasCondition() && !_bpExpEval->Evaluate(_rpnList[(int)operationInfo.Type][i], resultType, operationInfo, address)) {
				continue;
			}

			if(breakpoints[i].IsMarked() && processMarkedBreakpoints) {
				_eventManager->AddEvent(DebugEventType::Breakpoint, operationInfo, breakpoints[i].GetId());
			}
			if(breakpoints[i].IsEnabled()) {
				return breakpoints[i].GetId();
			}
		}
	}

	return -1;
}

template int BreakpointManager::InternalCheckBreakpoint<1>(MemoryOperationInfo operationInfo, AddressInfo& address, bool processMarkedBreakpoints);
template int BreakpointManager::InternalCheckBreakpoint<2>(MemoryOperationInfo operationInfo, AddressInfo& address, bool processMarkedBreakpoints);
template int BreakpointManager::InternalCheckBreakpoint<4>(MemoryOperationInfo operationInfo, AddressInfo& address, bool processMarkedBreakpoints);

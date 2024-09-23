#include "pch.h"
#include "Debugger/Breakpoint.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/DebugUtilities.h"

template<uint8_t accessWidth>
bool Breakpoint::Matches(MemoryOperationInfo& operation, AddressInfo &info)
{
	if(operation.MemType == _memoryType && DebugUtilities::IsRelativeMemory(_memoryType)) {
		for(int i = 0; i < accessWidth; i++) {
			if((int32_t)operation.Address + i >= _startAddr && (int32_t)operation.Address + i <= _endAddr) {
				return true;
			}
		}
		return false;
	} else if(_memoryType == info.Type) {
		for(int i = 0; i < accessWidth; i++) {
			if(info.Address + i >= _startAddr && info.Address + i <= _endAddr) {
				return true;
			}
		}
		return false;
	}

	return false;
}

bool Breakpoint::HasBreakpointType(BreakpointType type)
{
	switch(type) {
		default:
		case BreakpointType::Execute: return ((uint8_t)_type & (uint8_t)BreakpointTypeFlags::Execute) != 0;
		case BreakpointType::Read: return ((uint8_t)_type & (uint8_t)BreakpointTypeFlags::Read) != 0;
		case BreakpointType::Write: return ((uint8_t)_type & (uint8_t)BreakpointTypeFlags::Write) != 0;
		case BreakpointType::Forbid: return ((uint8_t)_type & (uint8_t)BreakpointTypeFlags::Forbid) != 0;
	}
}

string Breakpoint::GetCondition()
{
	return _condition;
}

bool Breakpoint::HasCondition()
{
	return _condition[0] != 0;
}

uint32_t Breakpoint::GetId()
{
	return _id;
}

CpuType Breakpoint::GetCpuType()
{
	return _cpuType;
}

bool Breakpoint::IsEnabled()
{
	return _enabled;
}

bool Breakpoint::IsMarked()
{
	return _markEvent;
}

bool Breakpoint::IsAllowedForOpType(MemoryOperationType opType)
{
	if(_ignoreDummyOperations) {
		return opType != MemoryOperationType::DummyRead && opType != MemoryOperationType::DummyWrite;
	}
	return true;
}

template bool Breakpoint::Matches<1>(MemoryOperationInfo& operation, AddressInfo& info);
template bool Breakpoint::Matches<2>(MemoryOperationInfo& operation, AddressInfo& info);
template bool Breakpoint::Matches<4>(MemoryOperationInfo& operation, AddressInfo& info);

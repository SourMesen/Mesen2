#include "pch.h"
#include "Debugger/Breakpoint.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/DebugUtilities.h"

bool Breakpoint::Matches(MemoryOperationInfo& operation, AddressInfo &info)
{
	if(operation.MemType == _memoryType && DebugUtilities::IsRelativeMemory(_memoryType)) {
		return (int32_t)operation.Address >= _startAddr && (int32_t)operation.Address <= _endAddr;
	} else if(_memoryType == info.Type) {
		return info.Address >= _startAddr && info.Address <= _endAddr;
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

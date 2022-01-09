#include "stdafx.h"
#include "Breakpoint.h"
#include "DebugTypes.h"
#include "DebugUtilities.h"

bool Breakpoint::Matches(MemoryOperationInfo& operation, AddressInfo &info)
{
	if(DebugUtilities::IsPpuMemory(_memoryType) != DebugUtilities::IsPpuMemory(info.Type)) {
		//Don't break on a PPU breakpoint if the operation is for a CPU, or vice versa
		return false;
	}

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

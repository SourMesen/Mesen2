#include "stdafx.h"
#include "Breakpoint.h"
#include "DebugTypes.h"

bool Breakpoint::Matches(uint32_t memoryAddr, AddressInfo &info)
{
	if(_memoryType == SnesMemoryType::CpuMemory) {
		if(_startAddr == -1) {
			return true;
		} else if(_endAddr == -1) {
			return (int32_t)memoryAddr == _startAddr;
		} else {
			return (int32_t)memoryAddr >= _startAddr && (int32_t)memoryAddr <= _endAddr;
		}
	} else if(_memoryType == info.Type) {
		if(_startAddr == -1) {
			return true;
		} else if(_endAddr == -1) {
			return info.Address == _startAddr;
		} else {
			return info.Address >= _startAddr && info.Address <= _endAddr;
		}
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

BreakpointCategory Breakpoint::GetBreakpointCategory()
{
	return GetBreakpointCategory(_memoryType);
}

BreakpointCategory Breakpoint::GetBreakpointCategory(SnesMemoryType memoryType)
{
	switch(memoryType) {
		case SnesMemoryType::CpuMemory:
		case SnesMemoryType::PrgRom:
		case SnesMemoryType::WorkRam:
		case SnesMemoryType::SaveRam:
			return BreakpointCategory::Cpu;

		case SnesMemoryType::SpcRam:
		case SnesMemoryType::SpcRom:
		case SnesMemoryType::SpcMemory:
			return BreakpointCategory::Spc;

		case SnesMemoryType::VideoRam:
			return BreakpointCategory::VideoRam;

		case SnesMemoryType::SpriteRam:
			return BreakpointCategory::Oam;

		case SnesMemoryType::CGRam:
			return BreakpointCategory::CgRam;

		default:	throw std::runtime_error("invalid memory type");
	}
}

bool Breakpoint::IsEnabled()
{
	return _enabled;
}

bool Breakpoint::IsMarked()
{
	return _markEvent;
}

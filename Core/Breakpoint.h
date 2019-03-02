#pragma once
#include "stdafx.h"

enum class SnesMemoryType;
struct AddressInfo;
enum class BreakpointType;
enum class BreakpointTypeFlags;
enum class BreakpointCategory;

class Breakpoint
{
public:
	bool Matches(uint32_t memoryAddr, AddressInfo &info);
	bool HasBreakpointType(BreakpointType type);
	string GetCondition();
	bool HasCondition();

	uint32_t GetId();
	bool IsEnabled();
	bool IsMarked();
	
	BreakpointCategory GetBreakpointCategory();
	static BreakpointCategory GetBreakpointCategory(SnesMemoryType memoryType);

private:
	uint32_t _id;
	SnesMemoryType _memoryType;
	BreakpointTypeFlags _type;
	int32_t _startAddr;
	int32_t _endAddr;
	bool _enabled;
	bool _markEvent;
	char _condition[1000];
};
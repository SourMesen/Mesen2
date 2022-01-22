#pragma once
#include "stdafx.h"

enum class CpuType : uint8_t; 
enum class MemoryType;
struct AddressInfo;
enum class BreakpointType;
enum class BreakpointTypeFlags;
enum class BreakpointCategory;
struct MemoryOperationInfo;

class Breakpoint
{
public:
	bool Matches(MemoryOperationInfo &opInfo, AddressInfo &info);
	bool HasBreakpointType(BreakpointType type);
	string GetCondition();
	bool HasCondition();

	uint32_t GetId();
	CpuType GetCpuType();
	bool IsEnabled();
	bool IsMarked();
	
private:
	uint32_t _id;
	CpuType _cpuType;
	MemoryType _memoryType;
	BreakpointTypeFlags _type;
	int32_t _startAddr;
	int32_t _endAddr;
	bool _enabled;
	bool _markEvent;
	char _condition[1000];
};
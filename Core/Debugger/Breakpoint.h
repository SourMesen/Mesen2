#pragma once
#include "pch.h"

enum class CpuType : uint8_t; 
enum class MemoryType;
struct AddressInfo;
enum class BreakpointType;
enum class BreakpointTypeFlags;
enum class MemoryOperationType;
struct MemoryOperationInfo;

class Breakpoint
{
public:
	template<uint8_t accessWidth = 1> bool Matches(MemoryOperationInfo &opInfo, AddressInfo &info);
	bool HasBreakpointType(BreakpointType type);
	string GetCondition();
	bool HasCondition();

	uint32_t GetId();
	CpuType GetCpuType();
	bool IsEnabled();
	bool IsMarked();
	bool IsAllowedForOpType(MemoryOperationType opType);

private:
	uint32_t _id;
	CpuType _cpuType;
	MemoryType _memoryType;
	BreakpointTypeFlags _type;
	int32_t _startAddr;
	int32_t _endAddr;
	bool _enabled;
	bool _markEvent;
	bool _ignoreDummyOperations;
	char _condition[1000];
};
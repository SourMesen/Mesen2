#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/DebugUtilities.h"
#include "Shared/MemoryType.h"

class Debugger;
class SnesMemoryManager;
class Spc;
class SnesConsole;
class Sa1;
class Gsu;
class Cx4;
class Gameboy;

struct AddressCounters
{
	uint64_t ReadStamp;
	uint64_t WriteStamp;
	uint64_t ExecStamp;
	uint32_t ReadCounter;
	uint32_t WriteCounter;
	uint32_t ExecCounter;
};

enum class ReadResult
{
	Normal,
	FirstUninitRead,
	UninitRead
};

class MemoryAccessCounter
{
private:
	vector<AddressCounters> _counters[DebugUtilities::GetMemoryTypeCount()];

	Debugger* _debugger = nullptr;
	bool _enableBreakOnUninitRead = false;

public:
	MemoryAccessCounter(Debugger *debugger);

	template<uint8_t accessWidth = 1> ReadResult ProcessMemoryRead(AddressInfo& addressInfo, uint64_t masterClock);
	template<uint8_t accessWidth = 1> void ProcessMemoryWrite(AddressInfo& addressInfo, uint64_t masterClock);
	template<uint8_t accessWidth = 1> void ProcessMemoryExec(AddressInfo& addressInfo, uint64_t masterClock);

	void ResetCounts();

	void GetAccessCounts(uint32_t offset, uint32_t length, MemoryType memoryType, AddressCounters counts[]);
};
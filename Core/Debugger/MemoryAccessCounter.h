#pragma once
#include "stdafx.h"
#include "DebugTypes.h"
#include "MemoryType.h"

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
	vector<AddressCounters> _counters[(int)MemoryType::Register];

	Debugger* _debugger;

	bool IsAddressUninitialized(AddressInfo &addressInfo);

public:
	MemoryAccessCounter(Debugger *debugger);

	ReadResult ProcessMemoryRead(AddressInfo& addressInfo, uint64_t masterClock);
	void ProcessMemoryWrite(AddressInfo& addressInfo, uint64_t masterClock);
	void ProcessMemoryExec(AddressInfo& addressInfo, uint64_t masterClock);

	void ResetCounts();

	void GetAccessCounts(uint32_t offset, uint32_t length, MemoryType memoryType, AddressCounters counts[]);
};
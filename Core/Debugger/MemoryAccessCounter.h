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
	uint32_t Address;
	uint32_t ReadCount;
	uint64_t ReadStamp;

	bool UninitRead;
	uint32_t WriteCount;
	uint64_t WriteStamp;

	uint32_t ExecCount;
	uint64_t ExecStamp;
};

class MemoryAccessCounter
{
private:
	vector<AddressCounters> _counters[(int)MemoryType::Register];

	Debugger* _debugger;

	bool IsAddressUninitialized(AddressInfo &addressInfo);

public:
	MemoryAccessCounter(Debugger *debugger);

	uint64_t GetReadCount(AddressInfo& addressInfo);

	bool ProcessMemoryRead(AddressInfo& addressInfo, uint64_t masterClock);
	void ProcessMemoryWrite(AddressInfo& addressInfo, uint64_t masterClock);
	void ProcessMemoryExec(AddressInfo& addressInfo, uint64_t masterClock);

	void ResetCounts();

	void GetAccessCounts(uint32_t offset, uint32_t length, MemoryType memoryType, AddressCounters counts[]);
};
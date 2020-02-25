#pragma once
#include "stdafx.h"
#include "DebugTypes.h"

class Debugger;
class MemoryManager;
class Spc;
class Console;
class Sa1;
class Gsu;
class Cx4;

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
	vector<AddressCounters> _counters[(int)SnesMemoryType::Register];

	Debugger* _debugger;
	MemoryManager* _memoryManager;
	Spc* _spc;
	Sa1* _sa1;
	Gsu* _gsu;
	Cx4* _cx4;

	bool IsAddressUninitialized(AddressInfo &addressInfo);

public:
	MemoryAccessCounter(Debugger *debugger, Console *console);

	bool ProcessMemoryRead(AddressInfo& addressInfo, uint64_t masterClock);
	void ProcessMemoryWrite(AddressInfo& addressInfo, uint64_t masterClock);
	void ProcessMemoryExec(AddressInfo& addressInfo, uint64_t masterClock);

	void ResetCounts();

	void GetAccessCounts(uint32_t offset, uint32_t length, SnesMemoryType memoryType, AddressCounters counts[]);
};
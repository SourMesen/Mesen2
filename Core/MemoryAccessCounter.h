#pragma once
#include "stdafx.h"
#include "DebugTypes.h"

class Debugger;
class MemoryManager;
class Spc;
class Console;
class Sa1;
class Gsu;

class MemoryAccessCounter
{
private:
	vector<uint32_t> _readCounts[(int)SnesMemoryType::Register];
	vector<uint32_t> _writeCounts[(int)SnesMemoryType::Register];
	vector<uint32_t> _execCounts[(int)SnesMemoryType::Register];

	vector<uint64_t> _readStamps[(int)SnesMemoryType::Register];
	vector<uint64_t> _writeStamps[(int)SnesMemoryType::Register];
	vector<uint64_t> _execStamps[(int)SnesMemoryType::Register];

	vector<bool> _uninitReads[(int)SnesMemoryType::Register];

	Debugger* _debugger;
	MemoryManager* _memoryManager;
	Spc* _spc;
	Sa1* _sa1;
	Gsu* _gsu;

	vector<uint32_t>& GetCountArray(MemoryOperationType operationType, SnesMemoryType memType);
	vector<uint64_t>& GetStampArray(MemoryOperationType operationType, SnesMemoryType memType);
	bool IsAddressUninitialized(AddressInfo &addressInfo);

public:
	MemoryAccessCounter(Debugger *debugger, Console *console);

	bool ProcessMemoryAccess(AddressInfo &addressInfo, MemoryOperationType operation, uint64_t masterClock);
	void ResetCounts();

	void GetAccessStamps(uint32_t offset, uint32_t length, SnesMemoryType memoryType, MemoryOperationType operationType, uint64_t stamps[]);
	void GetAccessCounts(uint32_t offset, uint32_t length, SnesMemoryType memoryType, MemoryOperationType operationType, uint32_t counts[]);
	void GetUninitMemoryReads(SnesMemoryType memoryType, bool uninitReads[]);
};
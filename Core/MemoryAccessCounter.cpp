#include "stdafx.h"
#include "MemoryAccessCounter.h"
#include "DebugBreakHelper.h"
#include "Debugger.h"
#include "MemoryDumper.h"
#include "SNES/MemoryManager.h"
#include "SNES/Console.h"
#include "SNES/Spc.h"
#include "SNES/Sa1.h"
#include "SNES/Gsu.h"
#include "SNES/Cx4.h"
#include "SNES/BaseCartridge.h"
#include "Gameboy/Gameboy.h"

MemoryAccessCounter::MemoryAccessCounter(Debugger* debugger)
{
	_debugger = debugger;

	for(int i = (int)SnesMemoryType::PrgRom; i < (int)SnesMemoryType::Register; i++) {
		uint32_t memSize = _debugger->GetMemoryDumper()->GetMemorySize((SnesMemoryType)i);
		_counters[i].reserve(memSize);
		for(uint32_t j = 0; j < memSize; j++) {
			_counters[i].push_back({ j });
		}
	}
}

bool MemoryAccessCounter::IsAddressUninitialized(AddressInfo& addressInfo)
{
	if(!DebugUtilities::IsRomMemory(addressInfo.Type)) {
		return _counters[(int)addressInfo.Type][addressInfo.Address].WriteCount == 0;
	}
	return false;
}

uint64_t MemoryAccessCounter::GetReadCount(AddressInfo& addressInfo)
{
	return _counters[(int)addressInfo.Type][addressInfo.Address].ReadCount;
}

bool MemoryAccessCounter::ProcessMemoryRead(AddressInfo &addressInfo, uint64_t masterClock)
{
	if(addressInfo.Address < 0) {
		return false;
	}

	AddressCounters& counts = _counters[(int)addressInfo.Type][addressInfo.Address];
	counts.ReadCount++;
	counts.ReadStamp = masterClock;
	if(counts.WriteCount == 0 && IsAddressUninitialized(addressInfo)) {
		//Mark address as read before being written to (if trying to read/execute)
		counts.UninitRead = true;
		return true;
	}
	return false;
}

void MemoryAccessCounter::ProcessMemoryWrite(AddressInfo& addressInfo, uint64_t masterClock)
{
	if(addressInfo.Address < 0) {
		return;
	}

	AddressCounters& counts = _counters[(int)addressInfo.Type][addressInfo.Address];
	counts.WriteCount++;
	counts.WriteStamp = masterClock;
}

void MemoryAccessCounter::ProcessMemoryExec(AddressInfo& addressInfo, uint64_t masterClock)
{
	if(addressInfo.Address < 0) {
		return;
	}

	AddressCounters& counts = _counters[(int)addressInfo.Type][addressInfo.Address];
	counts.ExecCount++;
	counts.ExecStamp = masterClock;
}

void MemoryAccessCounter::ResetCounts()
{
	DebugBreakHelper helper(_debugger);
	for(int i = 0; i < (int)SnesMemoryType::Register; i++) {
		for(uint32_t j = 0; j < _counters[i].size(); j++) {
			_counters[i][j] = { j };
		}
	}
}

void MemoryAccessCounter::GetAccessCounts(uint32_t offset, uint32_t length, SnesMemoryType memoryType, AddressCounters counts[])
{
	if(memoryType <= DebugUtilities::GetLastCpuMemoryType()) {
		AddressInfo addr = {};
		addr.Type = memoryType;
		for(uint32_t i = 0; i < length; i++) {
			addr.Address = offset + i;
			AddressInfo info = _debugger->GetAbsoluteAddress(addr);
			if(info.Address >= 0) {
				counts[i] = _counters[(int)info.Type][info.Address];
			}
		}
	} else {
		memcpy(counts, _counters[(int)memoryType].data() + offset, length * sizeof(AddressCounters));
	}
}

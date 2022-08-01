#include "stdafx.h"
#include "MemoryAccessCounter.h"
#include "DebugBreakHelper.h"
#include "Debugger.h"
#include "MemoryDumper.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/SnesConsole.h"
#include "SNES/Spc.h"
#include "SNES/Coprocessors/SA1/Sa1.h"
#include "SNES/Coprocessors/GSU/Gsu.h"
#include "SNES/Coprocessors/CX4/Cx4.h"
#include "SNES/BaseCartridge.h"
#include "Gameboy/Gameboy.h"

MemoryAccessCounter::MemoryAccessCounter(Debugger* debugger)
{
	_debugger = debugger;

	for(int i = (int)MemoryType::SnesPrgRom; i < (int)MemoryType::Register; i++) {
		uint32_t memSize = _debugger->GetMemoryDumper()->GetMemorySize((MemoryType)i);
		_counters[i].reserve(memSize);
		for(uint32_t j = 0; j < memSize; j++) {
			_counters[i].push_back({});
		}
	}
}

bool MemoryAccessCounter::IsAddressUninitialized(AddressInfo& addressInfo)
{
	if(DebugUtilities::IsVolatileRam(addressInfo.Type)) {
		return _counters[(int)addressInfo.Type][addressInfo.Address].WriteStamp == 0;
	}
	return false;
}

ReadResult MemoryAccessCounter::ProcessMemoryRead(AddressInfo &addressInfo, uint64_t masterClock)
{
	if(addressInfo.Address < 0) {
		return ReadResult::Normal;
	}

	AddressCounters& counts = _counters[(int)addressInfo.Type][addressInfo.Address];
	if(counts.WriteStamp == 0 && IsAddressUninitialized(addressInfo)) {
		ReadResult result = counts.ReadStamp == 0 ? ReadResult::FirstUninitRead : ReadResult::UninitRead;
		counts.ReadStamp = masterClock;
		return result;
	}

	counts.ReadStamp = masterClock;
	counts.ReadCounter++;
	return ReadResult::Normal;
}

void MemoryAccessCounter::ProcessMemoryWrite(AddressInfo& addressInfo, uint64_t masterClock)
{
	if(addressInfo.Address < 0) {
		return;
	}

	AddressCounters& counts = _counters[(int)addressInfo.Type][addressInfo.Address];
	counts.WriteStamp = masterClock;
	counts.WriteCounter++;
}

void MemoryAccessCounter::ProcessMemoryExec(AddressInfo& addressInfo, uint64_t masterClock)
{
	if(addressInfo.Address < 0) {
		return;
	}

	AddressCounters& counts = _counters[(int)addressInfo.Type][addressInfo.Address];
	counts.ExecStamp = masterClock;
	counts.ExecCounter++;
}

void MemoryAccessCounter::ResetCounts()
{
	DebugBreakHelper helper(_debugger);
	for(int i = 0; i < (int)MemoryType::Register; i++) {
		for(uint32_t j = 0; j < _counters[i].size(); j++) {
			_counters[i][j] = { j };
		}
	}
}

void MemoryAccessCounter::GetAccessCounts(uint32_t offset, uint32_t length, MemoryType memoryType, AddressCounters counts[])
{
	if(memoryType <= DebugUtilities::GetLastCpuMemoryType()) {
		AddressInfo addr = {};
		addr.Type = memoryType;
		for(uint32_t i = 0; i < length; i++) {
			addr.Address = offset + i;
			AddressInfo info = _debugger->GetAbsoluteAddress(addr);
			if(info.Address >= 0 && info.Type != MemoryType::Register) {
				counts[i] = _counters[(int)info.Type][info.Address];
			}
		}
	} else {
		memcpy(counts, _counters[(int)memoryType].data() + offset, length * sizeof(AddressCounters));
	}
}

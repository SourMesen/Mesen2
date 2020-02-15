#include "stdafx.h"
#include "MemoryAccessCounter.h"
#include "MemoryManager.h"
#include "DebugBreakHelper.h"
#include "Debugger.h"
#include "MemoryDumper.h"
#include "Spc.h"
#include "Sa1.h"
#include "Gsu.h"
#include "BaseCartridge.h"

MemoryAccessCounter::MemoryAccessCounter(Debugger* debugger, Console *console)
{
	_debugger = debugger;
	_memoryManager = console->GetMemoryManager().get();
	_spc = console->GetSpc().get();
	_sa1 = console->GetCartridge()->GetSa1();
	_gsu = console->GetCartridge()->GetGsu();

	for(int i = (int)SnesMemoryType::PrgRom; i < (int)SnesMemoryType::Register; i++) {
		uint32_t memSize = _debugger->GetMemoryDumper()->GetMemorySize((SnesMemoryType)i);
		_counters[i].reserve(memSize);
		for(uint32_t j = 0; j < memSize; j++) {
			_counters[i].push_back({ j });
		}
	}
}

bool MemoryAccessCounter::IsAddressUninitialized(AddressInfo &addressInfo)
{
	if(addressInfo.Type != SnesMemoryType::PrgRom && addressInfo.Type != SnesMemoryType::SaveRam) {
		return _counters[(int)addressInfo.Type][addressInfo.Address].WriteCount == 0;
	}
	return false;
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
	switch(memoryType) {
		case SnesMemoryType::CpuMemory:
			for(uint32_t i = 0; i < length; i++) {
				AddressInfo info = _memoryManager->GetMemoryMappings()->GetAbsoluteAddress(offset + i);
				if(info.Address >= 0) {
					counts[i] = _counters[(int)info.Type][info.Address];
				}
			}
			break;

		case SnesMemoryType::SpcMemory:
			for(uint32_t i = 0; i < length; i++) {
				AddressInfo info = _spc->GetAbsoluteAddress(offset + i);
				if(info.Address >= 0) {
					counts[i] = _counters[(int)info.Type][info.Address];
				}
			}
			break;

		case SnesMemoryType::Sa1Memory:
			for(uint32_t i = 0; i < length; i++) {
				AddressInfo info = _sa1->GetMemoryMappings()->GetAbsoluteAddress(offset + i);
				if(info.Address >= 0) {
					counts[i] = _counters[(int)info.Type][info.Address];
				}
			}
			break;

		case SnesMemoryType::GsuMemory:
			for(uint32_t i = 0; i < length; i++) {
				AddressInfo info = _gsu->GetMemoryMappings()->GetAbsoluteAddress(offset + i);
				if(info.Address >= 0) {
					counts[i] = _counters[(int)info.Type][info.Address];
				}
			}
			break;

		default:
			memcpy(counts, _counters[(int)memoryType].data() + offset, length * sizeof(AddressCounters));
			break;
	}
}

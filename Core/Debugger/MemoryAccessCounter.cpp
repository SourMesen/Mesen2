#include "pch.h"
#include "Debugger/MemoryAccessCounter.h"
#include "Debugger/DebugBreakHelper.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugUtilities.h"
#include "Debugger/MemoryDumper.h"
#include "Shared/Interfaces/IConsole.h"

MemoryAccessCounter::MemoryAccessCounter(Debugger* debugger)
{
	_debugger = debugger;

	//Enable breaking on uninit reads when debugger is opened at power on
	_enableBreakOnUninitRead = _debugger->GetConsole()->GetMasterClock() < 1000;

	for(int i = (int)DebugUtilities::GetLastCpuMemoryType() + 1; i < DebugUtilities::GetMemoryTypeCount(); i++) {
		uint32_t memSize = _debugger->GetMemoryDumper()->GetMemorySize((MemoryType)i);
		_counters[i].reserve(memSize);
		for(uint32_t j = 0; j < memSize; j++) {
			_counters[i].push_back({});
		}
	}
}

template<uint8_t accessWidth>
ReadResult MemoryAccessCounter::ProcessMemoryRead(AddressInfo &addressInfo, uint64_t masterClock)
{
	if(addressInfo.Address < 0) {
		return ReadResult::Normal;
	}

	ReadResult result = ReadResult::Normal;
	for(int i = 0; i < accessWidth; i++) {
		AddressCounters& counts = _counters[(int)addressInfo.Type][addressInfo.Address+i];
		if(_enableBreakOnUninitRead && counts.WriteStamp == 0 && DebugUtilities::IsVolatileRam(addressInfo.Type)) {
			result = (ReadResult)((int)result | (int)(counts.ReadStamp == 0 ? ReadResult::FirstUninitRead : ReadResult::UninitRead));
		}
		counts.ReadStamp = masterClock;
		counts.ReadCounter++;
	}
	return result;
}

template<uint8_t accessWidth>
void MemoryAccessCounter::ProcessMemoryWrite(AddressInfo& addressInfo, uint64_t masterClock)
{
	if(addressInfo.Address < 0) {
		return;
	}

	for(int i = 0; i < accessWidth; i++) {
		AddressCounters& counts = _counters[(int)addressInfo.Type][addressInfo.Address+i];
		counts.WriteStamp = masterClock;
		counts.WriteCounter++;
	}
}

template<uint8_t accessWidth>
void MemoryAccessCounter::ProcessMemoryExec(AddressInfo& addressInfo, uint64_t masterClock)
{
	if(addressInfo.Address < 0) {
		return;
	}

	for(int i = 0; i < accessWidth; i++) {
		AddressCounters& counts = _counters[(int)addressInfo.Type][addressInfo.Address+i];
		counts.ExecStamp = masterClock;
		counts.ExecCounter++;
	}
}

void MemoryAccessCounter::ResetCounts()
{
	DebugBreakHelper helper(_debugger);
	for(int i = 0; i < DebugUtilities::GetMemoryTypeCount(); i++) {
		memset(_counters[i].data(), 0, _counters[i].size() * sizeof(AddressCounters));
	}
	_enableBreakOnUninitRead = _debugger->GetConsole()->GetMasterClock() < 1000;
}

void MemoryAccessCounter::GetAccessCounts(uint32_t offset, uint32_t length, MemoryType memoryType, AddressCounters counts[])
{
	if(DebugUtilities::IsRelativeMemory(memoryType)) {
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
		if(offset + length <= _counters[(int)memoryType].size()) {
			memcpy(counts, _counters[(int)memoryType].data() + offset, length * sizeof(AddressCounters));
		}
	}
}

template ReadResult MemoryAccessCounter::ProcessMemoryRead<1>(AddressInfo& addressInfo, uint64_t masterClock);
template ReadResult MemoryAccessCounter::ProcessMemoryRead<2>(AddressInfo& addressInfo, uint64_t masterClock);
template ReadResult MemoryAccessCounter::ProcessMemoryRead<4>(AddressInfo& addressInfo, uint64_t masterClock);

template void MemoryAccessCounter::ProcessMemoryWrite<1>(AddressInfo& addressInfo, uint64_t masterClock);
template void MemoryAccessCounter::ProcessMemoryWrite<2>(AddressInfo& addressInfo, uint64_t masterClock);
template void MemoryAccessCounter::ProcessMemoryWrite<4>(AddressInfo& addressInfo, uint64_t masterClock);

template void MemoryAccessCounter::ProcessMemoryExec<1>(AddressInfo& addressInfo, uint64_t masterClock);
template void MemoryAccessCounter::ProcessMemoryExec<2>(AddressInfo& addressInfo, uint64_t masterClock);
template void MemoryAccessCounter::ProcessMemoryExec<4>(AddressInfo& addressInfo, uint64_t masterClock);

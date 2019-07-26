#include "stdafx.h"
#include "MemoryAccessCounter.h"
#include "MemoryManager.h"
#include "DebugBreakHelper.h"
#include "Debugger.h"
#include "MemoryDumper.h"
#include "Spc.h"
#include "Sa1.h"
#include "BaseCartridge.h"

MemoryAccessCounter::MemoryAccessCounter(Debugger* debugger, Console *console)
{
	_debugger = debugger;
	_memoryManager = console->GetMemoryManager().get();
	_spc = console->GetSpc().get();
	_sa1 = console->GetCartridge()->GetSa1();

	for(int i = (int)SnesMemoryType::PrgRom; i < (int)SnesMemoryType::Register; i++) {
		uint32_t memSize = _debugger->GetMemoryDumper()->GetMemorySize((SnesMemoryType)i);

		_readCounts[i].insert(_readCounts[i].end(), memSize, 0);
		_writeCounts[i].insert(_writeCounts[i].end(), memSize, 0);
		_execCounts[i].insert(_execCounts[i].end(), memSize, 0);

		_readStamps[i].insert(_readStamps[i].end(), memSize, 0);
		_writeStamps[i].insert(_writeStamps[i].end(), memSize, 0);
		_execStamps[i].insert(_execStamps[i].end(), memSize, 0);

		_uninitReads[i].insert(_uninitReads[i].end(), memSize, false);
	}
}

vector<uint32_t>& MemoryAccessCounter::GetCountArray(MemoryOperationType operationType, SnesMemoryType memType)
{
	switch(operationType) {
		case MemoryOperationType::DmaRead:
		case MemoryOperationType::Read: return _readCounts[(int)memType];
		
		case MemoryOperationType::DmaWrite:
		case MemoryOperationType::Write: return _writeCounts[(int)memType];

		default:
		case MemoryOperationType::ExecOpCode:
		case MemoryOperationType::ExecOperand: return _execCounts[(int)memType];
	}
}

vector<uint64_t>& MemoryAccessCounter::GetStampArray(MemoryOperationType operationType, SnesMemoryType memType)
{
	switch(operationType) {
		case MemoryOperationType::DmaRead:
		case MemoryOperationType::Read: return _readStamps[(int)memType];

		case MemoryOperationType::DmaWrite:
		case MemoryOperationType::Write: return _writeStamps[(int)memType];

		default:
		case MemoryOperationType::ExecOpCode:
		case MemoryOperationType::ExecOperand: return _execStamps[(int)memType];
	}
}

bool MemoryAccessCounter::IsAddressUninitialized(AddressInfo &addressInfo)
{
	if(addressInfo.Type != SnesMemoryType::PrgRom && addressInfo.Type != SnesMemoryType::SaveRam) {
		return _writeCounts[(int)addressInfo.Type][addressInfo.Address] == 0;
	}
	return false;
}

bool MemoryAccessCounter::ProcessMemoryAccess(AddressInfo &addressInfo, MemoryOperationType operation, uint64_t masterClock)
{
	if(addressInfo.Address < 0) {
		return false;
	}

	vector<uint32_t> &counts = GetCountArray(operation, addressInfo.Type);
	counts[addressInfo.Address]++;

	vector<uint64_t> &stamps = GetStampArray(operation, addressInfo.Type);
	stamps[addressInfo.Address] = masterClock;

	if(operation == MemoryOperationType::Read && IsAddressUninitialized(addressInfo)) {
		//Mark address as read before being written to (if trying to read/execute)
		_uninitReads[(int)addressInfo.Type][addressInfo.Address] = true;
		return true;
	}

	return false;
}

void MemoryAccessCounter::ResetCounts()
{
	DebugBreakHelper helper(_debugger);
	for(int i = 0; i < (int)SnesMemoryType::Register; i++) {
		memset(_readCounts[i].data(), 0, _readCounts[i].size() * sizeof(uint32_t));
		memset(_writeCounts[i].data(), 0, _writeCounts[i].size() * sizeof(uint32_t));
		memset(_execCounts[i].data(), 0, _execCounts[i].size() * sizeof(uint32_t));

		memset(_readStamps[i].data(), 0, _readStamps[i].size() * sizeof(uint64_t));
		memset(_writeStamps[i].data(), 0, _writeStamps[i].size() * sizeof(uint64_t));
		memset(_execStamps[i].data(), 0, _execStamps[i].size() * sizeof(uint64_t));
	}
}

void MemoryAccessCounter::GetAccessStamps(uint32_t offset, uint32_t length, SnesMemoryType memoryType, MemoryOperationType operationType, uint64_t stamps[])
{
	switch(memoryType) {
		case SnesMemoryType::CpuMemory:
			for(uint32_t i = 0; i < length; i++) {
				AddressInfo info = _memoryManager->GetMemoryMappings()->GetAbsoluteAddress(offset + i);
				if(info.Address >= 0) {
					stamps[i] = GetStampArray(operationType, info.Type)[info.Address];
				}
			}
			break;

		case SnesMemoryType::SpcMemory:
			for(uint32_t i = 0; i < length; i++) {
				AddressInfo info = _spc->GetAbsoluteAddress(offset + i);
				if(info.Address >= 0) {
					stamps[i] = GetStampArray(operationType, info.Type)[info.Address];
				}
			}
			break;

		case SnesMemoryType::Sa1Memory:
			for(uint32_t i = 0; i < length; i++) {
				AddressInfo info = _sa1->GetMemoryMappings()->GetAbsoluteAddress(offset + i);
				if(info.Address >= 0) {
					stamps[i] = GetStampArray(operationType, info.Type)[info.Address];
				}
			}
			break;

		default:
			memcpy(stamps, GetStampArray(operationType, memoryType).data() + offset, length * sizeof(uint64_t));
			break;
	}
}

void MemoryAccessCounter::GetAccessCounts(uint32_t offset, uint32_t length, SnesMemoryType memoryType, MemoryOperationType operationType, uint32_t counts[])
{
	switch(memoryType) {
		case SnesMemoryType::CpuMemory:
			for(uint32_t i = 0; i < length; i++) {
				AddressInfo info = _memoryManager->GetMemoryMappings()->GetAbsoluteAddress(offset + i);
				if(info.Address >= 0) {
					counts[i] = GetCountArray(operationType, info.Type)[info.Address];
				}
			}
			break;

		case SnesMemoryType::SpcMemory:
			for(uint32_t i = 0; i < length; i++) {
				AddressInfo info = _spc->GetAbsoluteAddress(offset + i);
				if(info.Address >= 0) {
					counts[i] = GetCountArray(operationType, info.Type)[info.Address];
				}
			}
			break;

		case SnesMemoryType::Sa1Memory:
			for(uint32_t i = 0; i < length; i++) {
				AddressInfo info = _sa1->GetMemoryMappings()->GetAbsoluteAddress(offset + i);
				if(info.Address >= 0) {
					counts[i] = GetCountArray(operationType, info.Type)[info.Address];
				}
			}
			break;

		default:
			memcpy(counts, GetCountArray(operationType, memoryType).data() + offset, length * sizeof(uint32_t));
			break;
	}
}

void MemoryAccessCounter::GetUninitMemoryReads(SnesMemoryType memoryType, bool uninitReads[])
{
	for(size_t i = 0, len = _uninitReads[(int)memoryType].size(); i < len; i++) {
		uninitReads[i] = _uninitReads[(int)memoryType][i];
	}
}

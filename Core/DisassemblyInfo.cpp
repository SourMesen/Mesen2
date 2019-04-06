#include "stdafx.h"
#include <algorithm>
#include "DisassemblyInfo.h"
#include "CpuTypes.h"
#include "MemoryManager.h"
#include "CpuDisUtils.h"
#include "../Utilities/HexUtilities.h"
#include "../Utilities/FastString.h"

DisassemblyInfo::DisassemblyInfo()
{
}

DisassemblyInfo::DisassemblyInfo(uint8_t *opPointer, uint8_t cpuFlags, CpuType type)
{
	Initialize(opPointer, cpuFlags, type);
}

void DisassemblyInfo::Initialize(uint8_t *opPointer, uint8_t cpuFlags, CpuType type)
{
	_cpuType = type;
	_flags = cpuFlags & (ProcFlags::MemoryMode8 | ProcFlags::IndexMode8);
	_opSize = GetOperandSize(opPointer[0], _flags, _cpuType) + 1;
	memcpy(_byteCode, opPointer, _opSize);

	_initialized = true;
}

bool DisassemblyInfo::IsInitialized()
{
	return _initialized;
}

void DisassemblyInfo::Reset()
{
	_initialized = false;
}

void DisassemblyInfo::GetDisassembly(string &out, uint32_t memoryAddr)
{
	switch(_cpuType) {
		case CpuType::Cpu: CpuDisUtils::GetDisassembly(*this, out, memoryAddr);
	}
}

int32_t DisassemblyInfo::GetEffectiveAddress(Console *console, void *cpuState)
{
	switch(_cpuType) {
		case CpuType::Cpu: return CpuDisUtils::GetEffectiveAddress(*this, console, *(CpuState*)cpuState);
	}
	return -1;
}

uint8_t DisassemblyInfo::GetOpCode()
{
	return _byteCode[0];
}

uint8_t DisassemblyInfo::GetOperandSize()
{
	return _opSize - 1;
}

uint8_t DisassemblyInfo::GetFlags()
{
	return _flags;
}

uint8_t* DisassemblyInfo::GetByteCode()
{
	return _byteCode;
}

void DisassemblyInfo::GetByteCode(uint8_t copyBuffer[4])
{
	memcpy(copyBuffer, _byteCode, _opSize);
}

void DisassemblyInfo::GetByteCode(string &out)
{
	FastString str;
	for(int i = 0; i < _opSize; i++) {
		str.Write('$', HexUtilities::ToHex(_byteCode[i]));
		if(i < _opSize - 1) {
			str.Write(' ');
		}
	}
	out += str.ToString();
}

uint8_t DisassemblyInfo::GetOperandSize(uint8_t opCode, uint8_t flags, CpuType type)
{
	switch(type) {
		case CpuType::Cpu: return CpuDisUtils::GetOperandSize(opCode, flags);
	}
	return 0;
}

uint16_t DisassemblyInfo::GetMemoryValue(uint32_t effectiveAddress, MemoryManager *memoryManager, uint8_t &valueSize)
{
	if(_flags & ProcFlags::MemoryMode8) {
		valueSize = 1;
		return memoryManager->Peek(effectiveAddress);
	} else {
		valueSize = 2;
		return memoryManager->PeekWord(effectiveAddress);
	}
}

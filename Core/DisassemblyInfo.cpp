#include "stdafx.h"
#include <algorithm>
#include "DisassemblyInfo.h"
#include "CpuTypes.h"
#include "MemoryManager.h"
#include "CpuDisUtils.h"
#include "SpcDisUtils.h"
#include "NecDspDisUtils.h"
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
	_opSize = GetOpSize(opPointer[0], _flags, _cpuType);
	memcpy(_byteCode, opPointer, _opSize);

	_initialized = true;
}

bool DisassemblyInfo::IsInitialized()
{
	return _initialized;
}

bool DisassemblyInfo::IsValid(uint8_t cpuFlags)
{
	return _flags == (cpuFlags & (ProcFlags::MemoryMode8 | ProcFlags::IndexMode8));
}

void DisassemblyInfo::Reset()
{
	_initialized = false;
}

void DisassemblyInfo::GetDisassembly(string &out, uint32_t memoryAddr, LabelManager* labelManager)
{
	switch(_cpuType) {
		case CpuType::Cpu: CpuDisUtils::GetDisassembly(*this, out, memoryAddr, labelManager); break;
		case CpuType::Spc: SpcDisUtils::GetDisassembly(*this, out, memoryAddr, labelManager); break;
		case CpuType::NecDsp: NecDspDisUtils::GetDisassembly(*this, out, memoryAddr, labelManager); break;
	}
}

int32_t DisassemblyInfo::GetEffectiveAddress(Console *console, void *cpuState)
{
	switch(_cpuType) {
		case CpuType::Cpu: return CpuDisUtils::GetEffectiveAddress(*this, console, *(CpuState*)cpuState);
		case CpuType::Spc: return SpcDisUtils::GetEffectiveAddress(*this, console, *(SpcState*)cpuState);
		case CpuType::NecDsp: return -1;
	}
	return -1;
}

uint8_t DisassemblyInfo::GetOpCode()
{
	return _byteCode[0];
}

uint8_t DisassemblyInfo::GetOpSize()
{
	return _opSize;
}

uint8_t DisassemblyInfo::GetFlags()
{
	return _flags;
}

CpuType DisassemblyInfo::GetCpuType()
{
	return _cpuType;
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
		str.WriteAll('$', HexUtilities::ToHex(_byteCode[i]));
		if(i < _opSize - 1) {
			str.Write(' ');
		}
	}
	out += str.ToString();
}

uint8_t DisassemblyInfo::GetOpSize(uint8_t opCode, uint8_t flags, CpuType type)
{
	switch(type) {
		case CpuType::Cpu: return CpuDisUtils::GetOpSize(opCode, flags);
		case CpuType::Spc: return SpcDisUtils::GetOpSize(opCode);
		case CpuType::NecDsp: return 4;
	}
	return 0;
}

bool DisassemblyInfo::IsJumpToSub(uint8_t opCode, CpuType type)
{
	switch(type) {
		case CpuType::Cpu: return opCode == 0x20 || opCode == 0x22 || opCode == 0xFC; //JSR, JSL
		case CpuType::Spc: return opCode == 0x3F || opCode == 0x0F; //JSR, BRK
		case CpuType::NecDsp: return false;
	}
	return false;
}

bool DisassemblyInfo::IsReturnInstruction(uint8_t opCode, CpuType type)
{
	//RTS/RTI
	switch(type) {
		case CpuType::Cpu: return opCode == 0x60 || opCode == 0x6B || opCode == 0x40;
		case CpuType::Spc: return opCode == 0x6F || opCode == 0x7F;
		case CpuType::NecDsp: return false;
	}
	
	return false;
}

bool DisassemblyInfo::UpdateCpuFlags(uint8_t &cpuFlags)
{
	uint8_t opCode = GetOpCode();
	switch(_cpuType) {
		case CpuType::Cpu:
			if(opCode == 0x00 || opCode == 0x20 || opCode == 0x40 || opCode == 0x60 || opCode == 0x80 || opCode == 0x22 || opCode == 0xFC || opCode == 0x6B || opCode == 0x4C || opCode == 0x5C || opCode == 0x6C || opCode == 0x7C || opCode == 0x02) {
				//Jumps, RTI, RTS, BRK, COP, etc., stop disassembling
				return false;
			} else if(opCode == 0xC2) {
				//REP, update the flags and keep disassembling
				uint8_t flags = GetByteCode()[1];
				cpuFlags &= ~flags;
			} else if(opCode == 0xE2) {
				//SEP, update the flags and keep disassembling
				uint8_t flags = GetByteCode()[1];
				cpuFlags |= flags;
			} else if(opCode == 0x28) {
				//PLP, stop disassembling
				return false;
			}
			return true;
			
		default:
		case CpuType::Spc:
		case CpuType::NecDsp: return false;
	}
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

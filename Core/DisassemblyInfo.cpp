#include "stdafx.h"
#include <algorithm>
#include "DisassemblyInfo.h"
#include "CpuTypes.h"
#include "EmuSettings.h"
#include "MemoryDumper.h"
#include "CpuDisUtils.h"
#include "SpcDisUtils.h"
#include "GsuDisUtils.h"
#include "NecDspDisUtils.h"
#include "Cx4DisUtils.h"
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
	_flags = cpuFlags;
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
	return _flags == cpuFlags;
}

void DisassemblyInfo::Reset()
{
	_initialized = false;
}

void DisassemblyInfo::GetDisassembly(string &out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings)
{
	switch(_cpuType) {
		case CpuType::Sa1:
		case CpuType::Cpu:
			CpuDisUtils::GetDisassembly(*this, out, memoryAddr, labelManager, settings);
			break;

		case CpuType::Spc: SpcDisUtils::GetDisassembly(*this, out, memoryAddr, labelManager, settings); break;
		case CpuType::NecDsp: NecDspDisUtils::GetDisassembly(*this, out, memoryAddr, labelManager, settings); break;
		case CpuType::Gsu: GsuDisUtils::GetDisassembly(*this, out, memoryAddr, labelManager, settings); break;
		case CpuType::Cx4: Cx4DisUtils::GetDisassembly(*this, out, memoryAddr, labelManager, settings); break;
	}
}

int32_t DisassemblyInfo::GetEffectiveAddress(Console *console, void *cpuState)
{
	switch(_cpuType) {
		case CpuType::Sa1:
		case CpuType::Cpu:
			return CpuDisUtils::GetEffectiveAddress(*this, console, *(CpuState*)cpuState);

		case CpuType::Spc: return SpcDisUtils::GetEffectiveAddress(*this, console, *(SpcState*)cpuState);
		case CpuType::Gsu: return GsuDisUtils::GetEffectiveAddress(*this, console, *(GsuState*)cpuState);

		case CpuType::Cx4:
		case CpuType::NecDsp:
			return -1;
	}
	return -1;
}

CpuType DisassemblyInfo::GetCpuType()
{
	return _cpuType;
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
		case CpuType::Sa1:
		case CpuType::Cpu: 
			return CpuDisUtils::GetOpSize(opCode, flags);

		case CpuType::Spc: return SpcDisUtils::GetOpSize(opCode);
		
		case CpuType::Gsu: 
			if(opCode >= 0x05 && opCode <= 0x0F) {
				return 2;
			} else if(opCode >= 0xA0 && opCode <= 0xAF) {
				return 2;
			} else if(opCode >= 0xF0 && opCode <= 0xFF) {
				return 3;
			}
			return 1;

		case CpuType::NecDsp: return 4;
		case CpuType::Cx4: return 2;
	}
	return 0;
}

bool DisassemblyInfo::IsJumpToSub(uint8_t opCode, CpuType type)
{
	switch(type) {
		case CpuType::Sa1:
		case CpuType::Cpu:
			return opCode == 0x20 || opCode == 0x22 || opCode == 0xFC; //JSR, JSL

		case CpuType::Spc: return opCode == 0x3F || opCode == 0x0F; //JSR, BRK
		
		case CpuType::Gsu:
		case CpuType::NecDsp:
		case CpuType::Cx4:
			return false;
	}
	return false;
}

bool DisassemblyInfo::IsReturnInstruction(uint8_t opCode, CpuType type)
{
	//RTS/RTI
	switch(type) {
		case CpuType::Sa1:
		case CpuType::Cpu:
			return opCode == 0x60 || opCode == 0x6B || opCode == 0x40;

		case CpuType::Spc: return opCode == 0x6F || opCode == 0x7F;
		
		case CpuType::Gsu:
		case CpuType::NecDsp:
		case CpuType::Cx4:
			return false;
	}
	
	return false;
}

bool DisassemblyInfo::UpdateCpuFlags(uint8_t &cpuFlags)
{
	uint8_t opCode = GetOpCode();
	switch(_cpuType) {
		case CpuType::Sa1:
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
			
		case CpuType::Gsu:
		case CpuType::Spc:
		case CpuType::NecDsp:
		case CpuType::Cx4:
			return false;
	}

	return false;
}

uint16_t DisassemblyInfo::GetMemoryValue(uint32_t effectiveAddress, MemoryDumper *memoryDumper, SnesMemoryType memType, uint8_t &valueSize)
{
	if(_cpuType == CpuType::Spc || (_flags & ProcFlags::MemoryMode8)) {
		valueSize = 1;
		return memoryDumper->GetMemoryValue(memType, effectiveAddress);
	} else {
		valueSize = 2;
		return memoryDumper->GetMemoryValueWord(memType, effectiveAddress);
	}
}

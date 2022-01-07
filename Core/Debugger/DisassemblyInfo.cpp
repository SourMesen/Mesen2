#include "stdafx.h"
#include <algorithm>
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/MemoryDumper.h"
#include "Debugger/DebugUtilities.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/FastString.h"
#include "SNES/CpuTypes.h"
#include "SNES/Console.h"
#include "SNES/Debugger/CpuDisUtils.h"
#include "SNES/Debugger/SpcDisUtils.h"
#include "SNES/Debugger/GsuDisUtils.h"
#include "SNES/Debugger/NecDspDisUtils.h"
#include "SNES/Debugger/Cx4DisUtils.h"
#include "Gameboy/Debugger/GameboyDisUtils.h"
#include "NES/Debugger/NesDisUtils.h"
#include "Shared/EmuSettings.h"

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

void DisassemblyInfo::Initialize(uint32_t cpuAddress, uint8_t cpuFlags, CpuType type, MemoryDumper* memoryDumper)
{
	_cpuType = type;
	_flags = cpuFlags;

	SnesMemoryType cpuMemType = DebugUtilities::GetCpuMemoryType(type);
	_byteCode[0] = memoryDumper->GetMemoryValue(cpuMemType, cpuAddress);

	_opSize = GetOpSize(_byteCode[0], _flags, _cpuType);

	for(int i = 1; i < _opSize; i++) {
		_byteCode[i] = memoryDumper->GetMemoryValue(cpuMemType, cpuAddress+i);
	}

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
		case CpuType::Gameboy: GameboyDisUtils::GetDisassembly(*this, out, memoryAddr, labelManager, settings); break;
		case CpuType::Nes: NesDisUtils::GetDisassembly(*this, out, memoryAddr, labelManager, settings); break;

		default:
			throw std::runtime_error("GetDisassembly - Unsupported CPU type");
	}

}

int32_t DisassemblyInfo::GetEffectiveAddress(Debugger *debugger, void *cpuState, CpuType cpuType)
{
	switch(_cpuType) {
		case CpuType::Sa1:
		case CpuType::Cpu:
			return CpuDisUtils::GetEffectiveAddress(*this, (Console*)debugger->GetConsole(), *(CpuState*)cpuState, cpuType);

		case CpuType::Spc: return SpcDisUtils::GetEffectiveAddress(*this, (Console*)debugger->GetConsole(), *(SpcState*)cpuState);
		case CpuType::Gsu: return GsuDisUtils::GetEffectiveAddress(*this, (Console*)debugger->GetConsole(), *(GsuState*)cpuState);

		case CpuType::Cx4:
		case CpuType::NecDsp:
			return -1;

		case CpuType::Gameboy: return GameboyDisUtils::GetEffectiveAddress(*this, *(GbCpuState*)cpuState);

		case CpuType::Nes: return NesDisUtils::GetEffectiveAddress(*this, *(NesCpuState*)cpuState, debugger->GetMemoryDumper());
	}

	throw std::runtime_error("GetEffectiveAddress - Unsupported CPU type");
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

		case CpuType::NecDsp: return 3;
		case CpuType::Cx4: return 2;
		
		case CpuType::Gameboy: return GameboyDisUtils::GetOpSize(opCode);
		case CpuType::Nes: return NesDisUtils::GetOpSize(opCode);
	}

	throw std::runtime_error("GetOpSize - Unsupported CPU type");
}

//TODO: This is never called, removed?
bool DisassemblyInfo::IsJumpToSub(uint8_t opCode, CpuType type)
{
	switch(type) {
		case CpuType::Sa1:
		case CpuType::Cpu:
			return opCode == 0x20 || opCode == 0x22 || opCode == 0xFC; //JSR, JSL

		case CpuType::Spc: return opCode == 0x3F || opCode == 0x0F; //JSR, BRK
		
		case CpuType::Gameboy: return GameboyDisUtils::IsJumpToSub(opCode);
		
		case CpuType::Gsu:
		case CpuType::NecDsp:
		case CpuType::Cx4:
			return false;

		case CpuType::Nes:
			return opCode == 0x20;
	}

	throw std::runtime_error("IsJumpToSub - Unsupported CPU type");
}

bool DisassemblyInfo::IsReturnInstruction(uint8_t opCode, CpuType type)
{
	//RTS/RTI
	switch(type) {
		case CpuType::Sa1:
		case CpuType::Cpu:
			return opCode == 0x60 || opCode == 0x6B || opCode == 0x40;

		case CpuType::Spc: return opCode == 0x6F || opCode == 0x7F;

		case CpuType::Gameboy: return GameboyDisUtils::IsReturnInstruction(opCode);

		case CpuType::Gsu:
		case CpuType::NecDsp:
		case CpuType::Cx4:
			return false;

		case CpuType::Nes:
			return opCode == 0x60;
	}
	
	throw std::runtime_error("IsReturnInstruction - Unsupported CPU type");
}

bool DisassemblyInfo::IsUnconditionalJump()
{
	uint8_t opCode = GetOpCode();
	switch(_cpuType) {
		case CpuType::Sa1:
		case CpuType::Cpu:
			if(opCode == 0x00 || opCode == 0x20 || opCode == 0x40 || opCode == 0x60 || opCode == 0x80 || opCode == 0x22 || opCode == 0xFC || opCode == 0x6B || opCode == 0x4C || opCode == 0x5C || opCode == 0x6C || opCode == 0x7C || opCode == 0x02) {
				//Jumps, RTI, RTS, BRK, COP, etc., stop disassembling
				return true;
			} else if(opCode == 0x28) {
				//PLP, stop disassembling because the 8-bit/16-bit flags could change
				return true;
			}
			return false;

		case CpuType::Gameboy:
			if(opCode == 0x18 || opCode == 0xC3 || opCode == 0xEA || opCode == 0xCD || opCode == 0xC9 || opCode == 0xD9 || opCode == 0xC7 || opCode == 0xCF || opCode == 0xD7 || opCode == 0xDF || opCode == 0xE7 || opCode == 0xEF || opCode == 0xF7 || opCode == 0xFF) {
				return true;
			}
			return false;

		case CpuType::Gsu:
		case CpuType::Spc:
		case CpuType::Cx4:
			return true;

		case CpuType::NecDsp:
			return false;

		case CpuType::Nes:
			return (
				opCode == 0x4C || //JMP (Absolute)
				opCode == 0x6C || //JMP (Indirect)
				opCode == 0x20 || //JSR
				opCode == 0x40 || //RTI
				opCode == 0x60 //RTS
			);
	}

	throw std::runtime_error("IsUnconditionalJump - Unsupported CPU type");
}

bool DisassemblyInfo::IsJump()
{
	if(IsUnconditionalJump()) {
		return true;
	}

	uint8_t opCode = GetOpCode();
	switch(_cpuType) {
		case CpuType::Sa1:
		case CpuType::Cpu:
			//TODO
			return false;

		case CpuType::Gameboy:
			//TODO
			return false;

		case CpuType::Gsu:
		case CpuType::Spc:
		case CpuType::Cx4:
			//TODO
			return false;

		case CpuType::NecDsp:
			//TODO
			return false;

		case CpuType::Nes:
			return (
				opCode == 0x10 || //BPL
				opCode == 0x30 || //BMI
				opCode == 0x50 || //BVC
				opCode == 0x70 || //BVS
				opCode == 0x90 || //BCC
				opCode == 0xB0 || //BCS
				opCode == 0xD0 || //BNE
				opCode == 0xF0 //BEQ
			);
	}

	return false;
}

void DisassemblyInfo::UpdateCpuFlags(uint8_t& cpuFlags)
{
	if(_cpuType == CpuType::Cpu || _cpuType == CpuType::Sa1) {
		uint8_t opCode = GetOpCode();
		if(opCode == 0xC2) {
			//REP, update the flags and keep disassembling
			uint8_t flags = GetByteCode()[1];
			cpuFlags &= ~flags;
		} else if(opCode == 0xE2) {
			//SEP, update the flags and keep disassembling
			uint8_t flags = GetByteCode()[1];
			cpuFlags |= flags;
		}
	}
}

uint16_t DisassemblyInfo::GetMemoryValue(uint32_t effectiveAddress, MemoryDumper *memoryDumper, SnesMemoryType memType, uint8_t &valueSize)
{
	if((_cpuType == CpuType::Spc || _cpuType == CpuType::Gameboy || _cpuType == CpuType::Nes) || (_flags & ProcFlags::MemoryMode8)) {
		valueSize = 1;
		return memoryDumper->GetMemoryValue(memType, effectiveAddress);
	} else {
		valueSize = 2;
		return memoryDumper->GetMemoryValueWord(memType, effectiveAddress);
	}
}

#include "stdafx.h"
#include <algorithm>
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/MemoryDumper.h"
#include "Debugger/DebugUtilities.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/FastString.h"
#include "SNES/SnesCpuTypes.h"
#include "SNES/SnesConsole.h"
#include "SNES/Debugger/SnesDisUtils.h"
#include "SNES/Debugger/SpcDisUtils.h"
#include "SNES/Debugger/GsuDisUtils.h"
#include "SNES/Debugger/NecDspDisUtils.h"
#include "SNES/Debugger/Cx4DisUtils.h"
#include "Gameboy/Debugger/GameboyDisUtils.h"
#include "NES/Debugger/NesDisUtils.h"
#include "PCE/Debugger/PceDisUtils.h"
#include "Shared/EmuSettings.h"

DisassemblyInfo::DisassemblyInfo()
{
}

DisassemblyInfo::DisassemblyInfo(uint32_t cpuAddress, uint8_t cpuFlags, CpuType cpuType, MemoryType memType, MemoryDumper* memoryDumper)
{
	Initialize(cpuAddress, cpuFlags, cpuType, memType, memoryDumper);
}

void DisassemblyInfo::Initialize(uint32_t cpuAddress, uint8_t cpuFlags, CpuType cpuType, MemoryType memType, MemoryDumper* memoryDumper)
{
	_cpuType = cpuType;
	_flags = cpuFlags;

	_byteCode[0] = memoryDumper->GetMemoryValue(memType, cpuAddress);

	_opSize = GetOpSize(_byteCode[0], _flags, _cpuType);

	for(int i = 1; i < _opSize; i++) {
		_byteCode[i] = memoryDumper->GetMemoryValue(memType, cpuAddress+i);
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
		case CpuType::Snes:
			SnesDisUtils::GetDisassembly(*this, out, memoryAddr, labelManager, settings);
			break;

		case CpuType::Spc: SpcDisUtils::GetDisassembly(*this, out, memoryAddr, labelManager, settings); break;
		case CpuType::NecDsp: NecDspDisUtils::GetDisassembly(*this, out, memoryAddr, labelManager, settings); break;
		case CpuType::Gsu: GsuDisUtils::GetDisassembly(*this, out, memoryAddr, labelManager, settings); break;
		case CpuType::Cx4: Cx4DisUtils::GetDisassembly(*this, out, memoryAddr, labelManager, settings); break;
		case CpuType::Gameboy: GameboyDisUtils::GetDisassembly(*this, out, memoryAddr, labelManager, settings); break;
		case CpuType::Nes: NesDisUtils::GetDisassembly(*this, out, memoryAddr, labelManager, settings); break;
		case CpuType::Pce: PceDisUtils::GetDisassembly(*this, out, memoryAddr, labelManager, settings); break;

		default:
			throw std::runtime_error("GetDisassembly - Unsupported CPU type");
	}

}

int32_t DisassemblyInfo::GetEffectiveAddress(Debugger *debugger, void *cpuState, CpuType cpuType)
{
	switch(_cpuType) {
		case CpuType::Sa1:
		case CpuType::Snes:
			return SnesDisUtils::GetEffectiveAddress(*this, (SnesConsole*)debugger->GetConsole(), *(SnesCpuState*)cpuState, cpuType);

		case CpuType::Spc: return SpcDisUtils::GetEffectiveAddress(*this, (SnesConsole*)debugger->GetConsole(), *(SpcState*)cpuState);
		case CpuType::Gsu: return GsuDisUtils::GetEffectiveAddress(*this, (SnesConsole*)debugger->GetConsole(), *(GsuState*)cpuState);
		case CpuType::Cx4: return Cx4DisUtils::GetEffectiveAddress(*this, *(Cx4State*)cpuState, debugger->GetMemoryDumper());
		
		case CpuType::NecDsp:
			return -1;

		case CpuType::Gameboy: return GameboyDisUtils::GetEffectiveAddress(*this, *(GbCpuState*)cpuState);

		case CpuType::Nes: return NesDisUtils::GetEffectiveAddress(*this, *(NesCpuState*)cpuState, debugger->GetMemoryDumper());
		case CpuType::Pce: return PceDisUtils::GetEffectiveAddress(*this, (PceConsole*)debugger->GetConsole(), *(PceCpuState*)cpuState);
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

void DisassemblyInfo::GetByteCode(uint8_t copyBuffer[8])
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
		case CpuType::Snes: return SnesDisUtils::GetOpSize(opCode, flags);
		case CpuType::Spc: return SpcDisUtils::GetOpSize(opCode);
		case CpuType::NecDsp: return 3;
		case CpuType::Sa1:return SnesDisUtils::GetOpSize(opCode, flags);
		case CpuType::Gsu: return GsuDisUtils::GetOpSize(opCode);
		case CpuType::Cx4: return 2;
		case CpuType::Gameboy: return GameboyDisUtils::GetOpSize(opCode);
		case CpuType::Nes: return NesDisUtils::GetOpSize(opCode);
		case CpuType::Pce: return PceDisUtils::GetOpSize(opCode);
	}

	throw std::runtime_error("GetOpSize - Unsupported CPU type");
}

bool DisassemblyInfo::IsJumpToSub()
{
	switch(_cpuType) {
		case CpuType::Snes: return SnesDisUtils::IsJumpToSub(GetOpCode());
		case CpuType::Spc: return SpcDisUtils::IsJumpToSub(GetOpCode());
		case CpuType::NecDsp: return NecDspDisUtils::IsJumpToSub(_byteCode[0] | (_byteCode[1] << 8) | (_byteCode[2] << 16));
		case CpuType::Sa1: return SnesDisUtils::IsJumpToSub(GetOpCode());
		case CpuType::Gsu: return false; //GSU has no JSR op codes
		case CpuType::Cx4: return Cx4DisUtils::IsJumpToSub(GetByteCode()[1]);
		case CpuType::Gameboy: return GameboyDisUtils::IsJumpToSub(GetOpCode());
		case CpuType::Nes: return NesDisUtils::IsJumpToSub(GetOpCode());
		case CpuType::Pce: return PceDisUtils::IsJumpToSub(GetOpCode());
	}

	throw std::runtime_error("IsJumpToSub - Unsupported CPU type");
}

bool DisassemblyInfo::IsReturnInstruction()
{
	switch(_cpuType) {
		case CpuType::Snes: return SnesDisUtils::IsReturnInstruction(GetOpCode());
		case CpuType::Spc: return SpcDisUtils::IsReturnInstruction(GetOpCode());
		case CpuType::NecDsp: return NecDspDisUtils::IsReturnInstruction(_byteCode[0] | (_byteCode[1] << 8) | (_byteCode[2] << 16));
		case CpuType::Sa1: return SnesDisUtils::IsReturnInstruction(GetOpCode());
		case CpuType::Gsu: return false; //GSU has no RTS/RTI op codes
		case CpuType::Cx4: return Cx4DisUtils::IsReturnInstruction(GetByteCode()[1]);
		case CpuType::Gameboy: return GameboyDisUtils::IsReturnInstruction(GetOpCode());
		case CpuType::Nes: return NesDisUtils::IsReturnInstruction(GetOpCode());
		case CpuType::Pce: return PceDisUtils::IsReturnInstruction(GetOpCode());
	}
	
	throw std::runtime_error("IsReturnInstruction - Unsupported CPU type");
}

bool DisassemblyInfo::CanDisassembleNextOp()
{
	if(IsUnconditionalJump()) {
		return false;
	}

	//TODO cleanup
	switch(_cpuType) {
		case CpuType::Sa1:
		case CpuType::Snes:
			if(GetOpCode() == 0x28) {
				//PLP, stop disassembling because the 8-bit/16-bit flags could change
				return false;
			}
			break;
	}

	return true;
}

bool DisassemblyInfo::IsUnconditionalJump()
{
	switch(_cpuType) {
		case CpuType::Snes: return SnesDisUtils::IsUnconditionalJump(GetOpCode());
		case CpuType::Spc: return SpcDisUtils::IsUnconditionalJump(GetOpCode());
		case CpuType::NecDsp: return NecDspDisUtils::IsUnconditionalJump(_byteCode[0] | (_byteCode[1] << 8) | (_byteCode[2] << 16));
		case CpuType::Sa1: return SnesDisUtils::IsUnconditionalJump(GetOpCode());
		case CpuType::Gsu: return GsuDisUtils::IsUnconditionalJump(GetOpCode());
		case CpuType::Cx4: return Cx4DisUtils::IsUnconditionalJump(GetByteCode()[1]);
		case CpuType::Gameboy: return GameboyDisUtils::IsUnconditionalJump(GetOpCode());
		case CpuType::Nes: return NesDisUtils::IsUnconditionalJump(GetOpCode());
		case CpuType::Pce: return PceDisUtils::IsUnconditionalJump(GetOpCode());
	}

	throw std::runtime_error("IsUnconditionalJump - Unsupported CPU type");
}

bool DisassemblyInfo::IsJump()
{
	if(IsUnconditionalJump()) {
		return true;
	}

	//Check for conditional jumps
	switch(_cpuType) {
		case CpuType::Snes: return SnesDisUtils::IsConditionalJump(GetOpCode());
		case CpuType::Spc: return SpcDisUtils::IsConditionalJump(GetOpCode());
		case CpuType::NecDsp: return NecDspDisUtils::IsConditionalJump(_byteCode[0] | (_byteCode[1] << 8) | (_byteCode[2] << 16));
		case CpuType::Sa1: return SnesDisUtils::IsConditionalJump(GetOpCode());
		case CpuType::Gsu: return GsuDisUtils::IsConditionalJump(GetOpCode());
		case CpuType::Cx4: return Cx4DisUtils::IsConditionalJump(GetByteCode()[1], GetByteCode()[0]);
		case CpuType::Gameboy: return GameboyDisUtils::IsConditionalJump(GetOpCode());
		case CpuType::Nes: return NesDisUtils::IsConditionalJump(GetOpCode());
		case CpuType::Pce: return PceDisUtils::IsConditionalJump(GetOpCode());
	}

	throw std::runtime_error("IsJump - Unsupported CPU type");
}

void DisassemblyInfo::UpdateCpuFlags(uint8_t& cpuFlags)
{
	//TODO cleanup
	if(_cpuType == CpuType::Snes || _cpuType == CpuType::Sa1) {
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

uint16_t DisassemblyInfo::GetMemoryValue(uint32_t effectiveAddress, MemoryDumper *memoryDumper, MemoryType memType, uint8_t &valueSize)
{
	//TODO cleanup
	if((_cpuType == CpuType::Spc || _cpuType == CpuType::Gameboy || _cpuType == CpuType::Nes || _cpuType == CpuType::Pce) || (_flags & ProcFlags::MemoryMode8)) {
		valueSize = 1;
		return memoryDumper->GetMemoryValue(memType, effectiveAddress);
	} else {
		valueSize = 2;
		return memoryDumper->GetMemoryValueWord(memType, effectiveAddress);
	}
}

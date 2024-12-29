#include "pch.h"
#include <algorithm>
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/MemoryDumper.h"
#include "Debugger/DebugUtilities.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/FastString.h"
#include "SNES/SnesCpuTypes.h"
#include "SNES/SnesConsole.h"
#include "SNES/BaseCartridge.h"
#include "SNES/Debugger/SnesDisUtils.h"
#include "SNES/Debugger/SpcDisUtils.h"
#include "SNES/Debugger/GsuDisUtils.h"
#include "SNES/Debugger/NecDspDisUtils.h"
#include "SNES/Debugger/Cx4DisUtils.h"
#include "SNES/Debugger/St018DisUtils.h"
#include "Gameboy/Debugger/GameboyDisUtils.h"
#include "NES/Debugger/NesDisUtils.h"
#include "PCE/Debugger/PceDisUtils.h"
#include "SMS/Debugger/SmsDisUtils.h"
#include "GBA/Debugger/GbaDisUtils.h"
#include "WS/Debugger/WsDisUtils.h"
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

	_opSize = GetOpSize(_byteCode[0], _flags, _cpuType, cpuAddress, memType, memoryDumper);

	for(int i = 1; i < _opSize && i < 8; i++) {
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
		case CpuType::St018: GbaDisUtils::GetDisassembly(*this, out, memoryAddr, labelManager, settings); break;
		case CpuType::Gameboy: GameboyDisUtils::GetDisassembly(*this, out, memoryAddr, labelManager, settings); break;
		case CpuType::Nes: NesDisUtils::GetDisassembly(*this, out, memoryAddr, labelManager, settings); break;
		case CpuType::Pce: PceDisUtils::GetDisassembly(*this, out, memoryAddr, labelManager, settings); break;
		case CpuType::Sms: SmsDisUtils::GetDisassembly(*this, out, memoryAddr, labelManager, settings); break;
		case CpuType::Gba: GbaDisUtils::GetDisassembly(*this, out, memoryAddr, labelManager, settings); break;
		case CpuType::Ws: WsDisUtils::GetDisassembly(*this, out, memoryAddr, labelManager, settings); break;

		default:
			throw std::runtime_error("GetDisassembly - Unsupported CPU type");
	}

}

EffectiveAddressInfo DisassemblyInfo::GetEffectiveAddress(Debugger *debugger, void *cpuState, CpuType cpuType)
{
	switch(_cpuType) {
		case CpuType::Sa1:
		case CpuType::Snes:
			return SnesDisUtils::GetEffectiveAddress(*this, (SnesConsole*)debugger->GetConsole(), *(SnesCpuState*)cpuState, cpuType);

		case CpuType::Spc: return SpcDisUtils::GetEffectiveAddress(*this, (SnesConsole*)debugger->GetConsole(), *(SpcState*)cpuState);
		case CpuType::Gsu: return GsuDisUtils::GetEffectiveAddress(*this, (SnesConsole*)debugger->GetConsole(), *(GsuState*)cpuState);
		case CpuType::Cx4: return Cx4DisUtils::GetEffectiveAddress(*this, *(Cx4State*)cpuState, debugger->GetMemoryDumper());
		case CpuType::St018: return St018DisUtils::GetEffectiveAddress(*this, (SnesConsole*)debugger->GetConsole(), *(ArmV3CpuState*)cpuState);
		
		case CpuType::NecDsp:
			return {};

		case CpuType::Gameboy: {
			if(debugger->GetMainCpuType() == CpuType::Snes) {
				Gameboy* gb = ((SnesConsole*)debugger->GetConsole())->GetCartridge()->GetGameboy();
				return GameboyDisUtils::GetEffectiveAddress(*this, gb, *(GbCpuState*)cpuState);
			} else {
				return GameboyDisUtils::GetEffectiveAddress(*this, (Gameboy*)debugger->GetConsole(), *(GbCpuState*)cpuState);
			}
		}

		case CpuType::Nes: return NesDisUtils::GetEffectiveAddress(*this, *(NesCpuState*)cpuState, debugger->GetMemoryDumper());
		case CpuType::Pce: return PceDisUtils::GetEffectiveAddress(*this, (PceConsole*)debugger->GetConsole(), *(PceCpuState*)cpuState);
		case CpuType::Sms: return SmsDisUtils::GetEffectiveAddress(*this, (SmsConsole*)debugger->GetConsole(), *(SmsCpuState*)cpuState);
		case CpuType::Gba: return GbaDisUtils::GetEffectiveAddress(*this, (GbaConsole*)debugger->GetConsole(), *(GbaCpuState*)cpuState);
		case CpuType::Ws: return WsDisUtils::GetEffectiveAddress(*this, (WsConsole*)debugger->GetConsole(), *(WsCpuState*)cpuState);
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

template<CpuType type>
uint32_t DisassemblyInfo::GetFullOpCode()
{
	switch(type) {
		default: return _byteCode[0];
		case CpuType::NecDsp: return _byteCode[0] | (_byteCode[1] << 8) | (_byteCode[2] << 16);
		case CpuType::Cx4: return _byteCode[1];
		case CpuType::St018: return _byteCode[0] | (_byteCode[1] << 8) | (_opSize == 4 ? ((_byteCode[2] << 16) | (_byteCode[3] << 24)) : 0);
		case CpuType::Gba: return _byteCode[0] | (_byteCode[1] << 8) | (_opSize == 4 ? ((_byteCode[2] << 16) | (_byteCode[3] << 24)) : 0);
		case CpuType::Ws: return WsDisUtils::GetFullOpCode(*this);
	}
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

uint8_t DisassemblyInfo::GetOpSize(uint32_t opCode, uint8_t flags, CpuType type, uint32_t cpuAddress, MemoryType memType, MemoryDumper* memoryDumper)
{
	switch(type) {
		case CpuType::Snes: return SnesDisUtils::GetOpSize(opCode, flags);
		case CpuType::Spc: return SpcDisUtils::GetOpSize(opCode);
		case CpuType::NecDsp: return NecDspDisUtils::GetOpSize();
		case CpuType::Sa1:return SnesDisUtils::GetOpSize(opCode, flags);
		case CpuType::Gsu: return GsuDisUtils::GetOpSize(opCode);
		case CpuType::Cx4: return Cx4DisUtils::GetOpSize();
		case CpuType::St018: return GbaDisUtils::GetOpSize(opCode, flags);
		case CpuType::Gameboy: return GameboyDisUtils::GetOpSize(opCode);
		case CpuType::Nes: return NesDisUtils::GetOpSize(opCode);
		case CpuType::Pce: return PceDisUtils::GetOpSize(opCode);
		case CpuType::Sms: return SmsDisUtils::GetOpSize(opCode, cpuAddress, memType, memoryDumper);
		case CpuType::Gba: return GbaDisUtils::GetOpSize(opCode, flags);
		case CpuType::Ws: return WsDisUtils::GetOpSize(cpuAddress, memType, memoryDumper);
	}

	throw std::runtime_error("GetOpSize - Unsupported CPU type");
}

bool DisassemblyInfo::IsJumpToSub()
{
	switch(_cpuType) {
		case CpuType::Snes: return SnesDisUtils::IsJumpToSub(GetOpCode());
		case CpuType::Spc: return SpcDisUtils::IsJumpToSub(GetOpCode());
		case CpuType::NecDsp: return NecDspDisUtils::IsJumpToSub(GetFullOpCode<CpuType::NecDsp>());
		case CpuType::Sa1: return SnesDisUtils::IsJumpToSub(GetOpCode());
		case CpuType::Gsu: return false; //GSU has no JSR op codes
		case CpuType::Cx4: return Cx4DisUtils::IsJumpToSub(GetFullOpCode<CpuType::Cx4>());
		case CpuType::St018: return GbaDisUtils::IsJumpToSub(GetFullOpCode<CpuType::St018>(), _flags);
		case CpuType::Gameboy: return GameboyDisUtils::IsJumpToSub(GetOpCode());
		case CpuType::Nes: return NesDisUtils::IsJumpToSub(GetOpCode());
		case CpuType::Pce: return PceDisUtils::IsJumpToSub(GetOpCode());
		case CpuType::Sms: return SmsDisUtils::IsJumpToSub(GetOpCode());
		case CpuType::Gba: return GbaDisUtils::IsJumpToSub(GetFullOpCode<CpuType::Gba>(), _flags);
		case CpuType::Ws: return WsDisUtils::IsJumpToSub(GetFullOpCode<CpuType::Ws>());
	}

	throw std::runtime_error("IsJumpToSub - Unsupported CPU type");
}

bool DisassemblyInfo::IsReturnInstruction()
{
	switch(_cpuType) {
		case CpuType::Snes: return SnesDisUtils::IsReturnInstruction(GetOpCode());
		case CpuType::Spc: return SpcDisUtils::IsReturnInstruction(GetOpCode());
		case CpuType::NecDsp: return NecDspDisUtils::IsReturnInstruction(GetFullOpCode<CpuType::NecDsp>());
		case CpuType::Sa1: return SnesDisUtils::IsReturnInstruction(GetOpCode());
		case CpuType::Gsu: return false; //GSU has no RTS/RTI op codes
		case CpuType::Cx4: return Cx4DisUtils::IsReturnInstruction(GetFullOpCode<CpuType::Cx4>());
		case CpuType::St018: return GbaDisUtils::IsReturnInstruction(GetFullOpCode<CpuType::St018>(), _flags);
		case CpuType::Gameboy: return GameboyDisUtils::IsReturnInstruction(GetOpCode());
		case CpuType::Nes: return NesDisUtils::IsReturnInstruction(GetOpCode());
		case CpuType::Pce: return PceDisUtils::IsReturnInstruction(GetOpCode());
		case CpuType::Sms: return SmsDisUtils::IsReturnInstruction(_byteCode[0] | (_byteCode[1] << 8));
		case CpuType::Gba: return GbaDisUtils::IsReturnInstruction(GetFullOpCode<CpuType::Gba>(), _flags);
		case CpuType::Ws: return WsDisUtils::IsReturnInstruction(GetFullOpCode<CpuType::Ws>());
	}
	
	throw std::runtime_error("IsReturnInstruction - Unsupported CPU type");
}

bool DisassemblyInfo::CanDisassembleNextOp()
{
	if(IsUnconditionalJump()) {
		return false;
	}

	switch(_cpuType) {
		case CpuType::Snes: return SnesDisUtils::CanDisassembleNextOp(GetOpCode());
		case CpuType::Sa1: return SnesDisUtils::CanDisassembleNextOp(GetOpCode());
		case CpuType::Gsu: return GsuDisUtils::CanDisassembleNextOp(GetOpCode());
		case CpuType::Cx4: return Cx4DisUtils::CanDisassembleNextOp(GetByteCode()[1]);
		default: return true;
	}
}

bool DisassemblyInfo::IsUnconditionalJump()
{
	switch(_cpuType) {
		case CpuType::Snes: return SnesDisUtils::IsUnconditionalJump(GetOpCode());
		case CpuType::Spc: return SpcDisUtils::IsUnconditionalJump(GetOpCode());
		case CpuType::NecDsp: return NecDspDisUtils::IsUnconditionalJump(GetFullOpCode<CpuType::NecDsp>());
		case CpuType::Sa1: return SnesDisUtils::IsUnconditionalJump(GetOpCode());
		case CpuType::Gsu: return GsuDisUtils::IsUnconditionalJump(GetOpCode());
		case CpuType::Cx4: return Cx4DisUtils::IsUnconditionalJump(GetFullOpCode<CpuType::Cx4>());
		case CpuType::St018: return GbaDisUtils::IsUnconditionalJump(GetFullOpCode<CpuType::St018>(), _flags);
		case CpuType::Gameboy: return GameboyDisUtils::IsUnconditionalJump(GetOpCode());
		case CpuType::Nes: return NesDisUtils::IsUnconditionalJump(GetOpCode());
		case CpuType::Pce: return PceDisUtils::IsUnconditionalJump(GetOpCode());
		case CpuType::Sms: return SmsDisUtils::IsUnconditionalJump(GetOpCode());
		case CpuType::Gba: return GbaDisUtils::IsUnconditionalJump(GetFullOpCode<CpuType::Gba>(), _flags);
		case CpuType::Ws: return WsDisUtils::IsUnconditionalJump(GetFullOpCode<CpuType::Ws>());
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
		case CpuType::NecDsp: return NecDspDisUtils::IsConditionalJump(GetFullOpCode<CpuType::NecDsp>());
		case CpuType::Sa1: return SnesDisUtils::IsConditionalJump(GetOpCode());
		case CpuType::Gsu: return GsuDisUtils::IsConditionalJump(GetOpCode());
		case CpuType::Cx4: return Cx4DisUtils::IsConditionalJump(GetFullOpCode<CpuType::Cx4>(), GetByteCode()[0]);
		case CpuType::St018: return GbaDisUtils::IsConditionalJump(GetFullOpCode<CpuType::St018>(), GetByteCode()[0]);
		case CpuType::Gameboy: return GameboyDisUtils::IsConditionalJump(GetOpCode());
		case CpuType::Nes: return NesDisUtils::IsConditionalJump(GetOpCode());
		case CpuType::Pce: return PceDisUtils::IsConditionalJump(GetOpCode());
		case CpuType::Sms: return SmsDisUtils::IsConditionalJump(GetOpCode());
		case CpuType::Gba: return GbaDisUtils::IsConditionalJump(GetFullOpCode<CpuType::Gba>(), _flags);
		case CpuType::Ws: return WsDisUtils::IsConditionalJump(GetFullOpCode<CpuType::Ws>());
	}

	throw std::runtime_error("IsJump - Unsupported CPU type");
}

void DisassemblyInfo::UpdateCpuFlags(uint8_t& cpuFlags)
{
	switch(_cpuType) {
		case CpuType::Snes: SnesDisUtils::UpdateCpuFlags(GetOpCode(), GetByteCode(), cpuFlags); break;
		case CpuType::Sa1: SnesDisUtils::UpdateCpuFlags(GetOpCode(), GetByteCode(), cpuFlags); break;
		case CpuType::Gsu: GsuDisUtils::UpdateCpuFlags(GetOpCode(), cpuFlags); break;
		default: break;
	}
}

uint32_t DisassemblyInfo::GetMemoryValue(EffectiveAddressInfo effectiveAddress, MemoryDumper *memoryDumper, MemoryType memType)
{
	MemoryType effectiveMemType = effectiveAddress.Type == MemoryType::None ? memType : effectiveAddress.Type;
	switch(effectiveAddress.ValueSize) {
		default:
		case 1: return memoryDumper->GetMemoryValue(effectiveMemType, effectiveAddress.Address);
		case 2: return memoryDumper->GetMemoryValue16(effectiveMemType, effectiveAddress.Address);
		case 4: return memoryDumper->GetMemoryValue32(effectiveMemType, effectiveAddress.Address);
	}
}

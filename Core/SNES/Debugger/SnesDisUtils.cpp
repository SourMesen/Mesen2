#include "pch.h"
#include "SNES/Debugger/SnesDisUtils.h"
#include "SNES/SnesCpuTypes.h"
#include "SNES/SnesCpu.h"
#include "SNES/SnesConsole.h"
#include "Shared/EmuSettings.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/LabelManager.h"
#include "SNES/Debugger/DummySnesCpu.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/FastString.h"

void SnesDisUtils::GetDisassembly(DisassemblyInfo &info, string &out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings)
{
	FastString str(settings->GetDebugConfig().UseLowerCaseDisassembly);

	uint8_t opCode = info.GetOpCode();
	SnesAddrMode addrMode = SnesDisUtils::OpMode[opCode];
	str.Write(SnesDisUtils::OpName[opCode]);
	str.Write(' ');

	uint32_t opAddr = SnesDisUtils::GetOperandAddress(info, memoryAddr);
	uint32_t opSize = info.GetOpSize();

	FastString operand(settings->GetDebugConfig().UseLowerCaseDisassembly);
	if(opSize > 1) {
		if(addrMode == SnesAddrMode::Rel || addrMode == SnesAddrMode::RelLng || opSize == 4) {
			AddressInfo address { (int32_t)opAddr, MemoryType::SnesMemory };
			string label = labelManager ? labelManager->GetLabel(address) : "";
			if(label.size()) {
				operand.Write(label, true);
			} else {
				operand.WriteAll('$', HexUtilities::ToHex24(opAddr));
			}
		} else if(opSize == 2) {
			operand.WriteAll('$', HexUtilities::ToHex((uint8_t)opAddr));
		} else if(opSize == 3) {
			operand.WriteAll('$', HexUtilities::ToHex((uint16_t)opAddr));
		}
	}

	switch(addrMode) {
		case SnesAddrMode::Abs: str.Write(operand); break;
		case SnesAddrMode::AbsJmp: str.Write(operand); break;
		case SnesAddrMode::AbsIdxXInd: str.WriteAll('(', operand, ",X)"); break;
		case SnesAddrMode::AbsIdxX: str.WriteAll(operand, ",X"); break;
		case SnesAddrMode::AbsIdxY: str.WriteAll(operand, ",Y"); break;
		case SnesAddrMode::AbsInd:  str.WriteAll('(', operand, ')'); break;
		case SnesAddrMode::AbsIndLng:  str.WriteAll('[', operand, ']'); break;
		case SnesAddrMode::AbsLngIdxX: str.WriteAll(operand, ",X"); break;
		case SnesAddrMode::AbsLng: str.Write(operand); break;
		case SnesAddrMode::AbsLngJmp: str.Write(operand); break;
		case SnesAddrMode::Acc: break;
		case SnesAddrMode::BlkMov: str.WriteAll('$', operand[1], operand[2], ','); str.WriteAll('$', operand[3], operand[4]); break;
		case SnesAddrMode::DirIdxIndX: str.WriteAll('(', operand, ",X)"); break;
		case SnesAddrMode::DirIdxX: str.WriteAll(operand, ",X"); break;
		case SnesAddrMode::DirIdxY: str.WriteAll(operand, ",Y"); break;
		case SnesAddrMode::DirIndIdxY: str.WriteAll("(", operand, "),Y"); break;
		case SnesAddrMode::DirIndLngIdxY: str.WriteAll("[", operand, "],Y"); break;
		case SnesAddrMode::DirIndLng: str.WriteAll("[", operand, "]"); break;
		case SnesAddrMode::DirInd: str.WriteAll("(", operand, ")"); break;
		case SnesAddrMode::Dir: str.Write(operand); break;

		case SnesAddrMode::Imm8: case SnesAddrMode::Imm16: case SnesAddrMode::ImmX: case SnesAddrMode::ImmM:
			str.WriteAll('#', operand);
			break;

		case SnesAddrMode::Imp: break;
		case SnesAddrMode::RelLng: str.Write(operand); break;
		case SnesAddrMode::Rel: str.Write(operand); break;
		case SnesAddrMode::Stk: break;
		case SnesAddrMode::StkRel: str.WriteAll(operand, ",S"); break;
		case SnesAddrMode::StkRelIndIdxY: str.WriteAll('(', operand, ",S),Y"); break;

		default: throw std::runtime_error("invalid address mode");
	}

	out += str.ToString();
}

uint32_t SnesDisUtils::GetOperandAddress(DisassemblyInfo &info, uint32_t memoryAddr)
{
	uint32_t opSize = info.GetOpSize();
	uint32_t opAddr = 0;
	uint8_t* byteCode = info.GetByteCode();
	if(opSize == 2) {
		opAddr = byteCode[1];
	} else if(opSize == 3) {
		opAddr = byteCode[1] | (byteCode[2] << 8);
	} else if(opSize == 4) {
		opAddr = byteCode[1] | (byteCode[2] << 8) | (byteCode[3] << 16);
	}

	SnesAddrMode addrMode = SnesDisUtils::OpMode[byteCode[0]];
	if(addrMode == SnesAddrMode::Rel || addrMode == SnesAddrMode::RelLng) {
		if(opSize == 2) {
			opAddr = (memoryAddr & 0xFF0000) | (((int8_t)opAddr + memoryAddr + 2) & 0xFFFF);
		} else {
			opAddr = (memoryAddr & 0xFF0000) | (((int16_t)opAddr + memoryAddr + 3) & 0xFFFF);
		}
	}

	return opAddr;
}

EffectiveAddressInfo SnesDisUtils::GetEffectiveAddress(DisassemblyInfo &info, SnesConsole *console, SnesCpuState &state, CpuType type)
{
	SnesAddrMode opMode = SnesDisUtils::OpMode[info.GetOpCode()];
	if(opMode == SnesAddrMode::Stk) {
		//Show nothing for stack operations (push/pull)
		return {};
	}

	bool showEffectiveAddress = HasEffectiveAddress(opMode);

	DummySnesCpu dummyCpu(console, type);
	state.PS &= ~(ProcFlags::IndexMode8 | ProcFlags::MemoryMode8);
	state.PS |= info.GetFlags();
	dummyCpu.SetDummyState(state);
	dummyCpu.Exec();

	bool isJump = SnesDisUtils::IsUnconditionalJump(info.GetOpCode()) || SnesDisUtils::IsConditionalJump(info.GetOpCode());
	if(isJump) {
		if(info.GetOpSize() == 3) {
			//For 3-byte jumps, return the target address, and show no value
			if(SnesDisUtils::IsUnconditionalJump(info.GetOpCode())) {
				//Display the dummy cpu's current address - this allows indirect jumps to display the correct address based on K
				return { (dummyCpu.GetState().K << 16) | dummyCpu.GetState().PC, 0, true };
			} else {
				return { dummyCpu.GetLastOperand(), 0, true };
			}
		} else {
			//Relative or long jumps already show the final address in the disassembly, show nothing
			return {};
		}
	}

	//For everything else, return the last read/write address
	uint32_t count = dummyCpu.GetOperationCount();
	for(int i = count - 1; i > 0; i--) {
		MemoryOperationInfo opInfo = dummyCpu.GetOperationInfo(i);

		if(opInfo.Type != MemoryOperationType::ExecOperand) {
			MemoryOperationInfo prevOpInfo = dummyCpu.GetOperationInfo(i - 1);
			EffectiveAddressInfo result;
			if(prevOpInfo.Type == opInfo.Type && prevOpInfo.Address == opInfo.Address - 1) {
				//For 16-bit read/writes, return the first address
				result.Address = prevOpInfo.Address;
				result.Type = prevOpInfo.MemType;
				result.ValueSize = 2;
			} else if(opInfo.Type == MemoryOperationType::Write && prevOpInfo.Type == opInfo.Type && prevOpInfo.Address == opInfo.Address + 1) {
				//For 16-bit RMW instructions, the 2nd byte is written first
				result.Address = opInfo.Address;
				result.Type = opInfo.MemType;
				result.ValueSize = 2;
			} else {
				result.Address = opInfo.Address;
				result.Type = opInfo.MemType;
				result.ValueSize = 1;
			}
			result.ShowAddress = showEffectiveAddress;
			return result;
		}
	}

	return {};
}

bool SnesDisUtils::CanDisassembleNextOp(uint8_t opCode)
{
	//Stop disassembling on PLP (because it can alter X/M flags)
	return opCode != 0x28;
}

void SnesDisUtils::UpdateCpuFlags(uint8_t opCode, uint8_t* byteCode, uint8_t& cpuFlags)
{
	if(opCode == 0xC2) {
		//REP, update the flags and keep disassembling
		uint8_t flags = byteCode[1];
		cpuFlags &= ~flags;
	} else if(opCode == 0xE2) {
		//SEP, update the flags and keep disassembling
		uint8_t flags = byteCode[1];
		cpuFlags |= flags;
	}
}

bool SnesDisUtils::HasEffectiveAddress(SnesAddrMode addrMode)
{
	switch(addrMode) {
		case SnesAddrMode::Acc:
		case SnesAddrMode::Imp:
		case SnesAddrMode::Stk:
		case SnesAddrMode::Sig8:
		case SnesAddrMode::Imm8:
		case SnesAddrMode::Rel:
		case SnesAddrMode::RelLng:
		case SnesAddrMode::Imm16:
		case SnesAddrMode::BlkMov:
		case SnesAddrMode::AbsLngJmp:
		case SnesAddrMode::AbsLng:
		case SnesAddrMode::ImmX:
		case SnesAddrMode::ImmM:
		case SnesAddrMode::AbsJmp:
			return false;

		case SnesAddrMode::DirIdxIndX:
		case SnesAddrMode::DirIdxX:
		case SnesAddrMode::DirIdxY:
		case SnesAddrMode::DirIndIdxY:
		case SnesAddrMode::DirIndLngIdxY:
		case SnesAddrMode::DirIndLng:
		case SnesAddrMode::DirInd:
		case SnesAddrMode::Dir:
		case SnesAddrMode::StkRel:
		case SnesAddrMode::StkRelIndIdxY:
		case SnesAddrMode::Abs:
		case SnesAddrMode::AbsIdxXInd:
		case SnesAddrMode::AbsIdxX:
		case SnesAddrMode::AbsIdxY:
		case SnesAddrMode::AbsLngIdxX:
		case SnesAddrMode::AbsInd:
		case SnesAddrMode::AbsIndLng:
			return true;
	}

	throw std::runtime_error("Invalid mode");
}

uint8_t SnesDisUtils::GetOpSize(SnesAddrMode addrMode, uint8_t flags)
{
	if(addrMode == SnesAddrMode::ImmX) {
		return (flags & ProcFlags::IndexMode8) ? 2 : 3;
	} else if(addrMode == SnesAddrMode::ImmM) {
		return (flags & ProcFlags::MemoryMode8) ? 2 : 3;
	}

	return SnesDisUtils::OpSize[(int)addrMode];
}

uint8_t SnesDisUtils::GetOpSize(uint8_t opCode, uint8_t flags)
{
	return GetOpSize(SnesDisUtils::OpMode[opCode], flags);
}

bool SnesDisUtils::IsUnconditionalJump(uint8_t opCode)
{
	switch(opCode) {
		case 0x00: //BRK
		case 0x02: //COP
		case 0x20: //JSR
		case 0x22: //JSR
		case 0x40: //RTI
		case 0x4C: //JMP
		case 0x5C: //JMP
		case 0x60: //RTS
		case 0x6B: //RTL
		case 0x6C: //JMP
		case 0x7C: //JMP
		case 0x80: //BRA
		case 0x82: //BRL
		case 0xDC: //JMP
		case 0xFC: //JSR
			return true;

		default:
			return false;
	}
}

bool SnesDisUtils::IsConditionalJump(uint8_t opCode)
{
	switch(opCode) {
		case 0x10: //BPL
		case 0x30: //BMI
		case 0x50: //BVC
		case 0x70: //BVS
		case 0x90: //BCC
		case 0xB0: //BCS
		case 0xD0: //BNE
		case 0xF0: //BEQ
			return true;

		default:
			return false;
	}
}

CdlFlags::CdlFlags SnesDisUtils::GetOpFlags(uint8_t opCode, uint32_t pc, uint32_t prevPc)
{
	switch(opCode) {
		case 0x00: //BRK
		case 0x02: //COP
		case 0x20: //JSR
		case 0x22: //JSR
		case 0xFC: //JSR
			return CdlFlags::SubEntryPoint;

		case 0x10: //BPL
		case 0x30: //BMI
		case 0x50: //BVC
		case 0x70: //BVS
		case 0x90: //BCC
		case 0xB0: //BCS
		case 0xD0: //BNE
		case 0xF0: //BEQ
		case 0x4C: //JMP
		case 0x5C: //JMP
		case 0x6C: //JMP
		case 0x7C: //JMP
		case 0x80: //BRA
		case 0x82: //BRL
		case 0xDC: //JMP
			return pc != prevPc + SnesDisUtils::GetOpSize(opCode, 0) ? CdlFlags::JumpTarget : CdlFlags::None;

		default:
			return CdlFlags::None;
	}
}

bool SnesDisUtils::IsJumpToSub(uint8_t opCode)
{
	switch(opCode) {
		case 0x00: //BRK
		case 0x02: //COP
		case 0x20: //JSR
		case 0x22: //JSL
		case 0xFC: //JSR
			return true;

		default:
			return false;
	}
}

bool SnesDisUtils::IsReturnInstruction(uint8_t opCode)
{
	return opCode == 0x60 || opCode == 0x6B || opCode == 0x40;
}

uint8_t SnesDisUtils::OpSize[0x1F] = {
	2, 2, 3, 0, 0, 3, 3, 3, 3, 3,
	3, 4, 4, 3, 4, 1, 3, 2, 2, 2,
	2, 2, 2, 2, 2, 1, 3, 2, 1, 2,
	2
};

string SnesDisUtils::OpName[256] = {
	//0    1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
	"BRK", "ORA", "COP", "ORA", "TSB", "ORA", "ASL", "ORA", "PHP", "ORA", "ASL", "PHD", "TSB", "ORA", "ASL", "ORA", // 0
	"BPL", "ORA", "ORA", "ORA", "TRB", "ORA", "ASL", "ORA", "CLC", "ORA", "INC", "TCS", "TRB", "ORA", "ASL", "ORA", // 1
	"JSR", "AND", "JSL", "AND", "BIT", "AND", "ROL", "AND", "PLP", "AND", "ROL", "PLD", "BIT", "AND", "ROL", "AND", // 2
	"BMI", "AND", "AND", "AND", "BIT", "AND", "ROL", "AND", "SEC", "AND", "DEC", "TSC", "BIT", "AND", "ROL", "AND", // 3
	"RTI", "EOR", "WDM", "EOR", "MVP", "EOR", "LSR", "EOR", "PHA", "EOR", "LSR", "PHK", "JMP", "EOR", "LSR", "EOR", // 4
	"BVC", "EOR", "EOR", "EOR", "MVN", "EOR", "LSR", "EOR", "CLI", "EOR", "PHY", "TCD", "JML", "EOR", "LSR", "EOR", // 5
	"RTS", "ADC", "PER", "ADC", "STZ", "ADC", "ROR", "ADC", "PLA", "ADC", "ROR", "RTL", "JMP", "ADC", "ROR", "ADC", // 6
	"BVS", "ADC", "ADC", "ADC", "STZ", "ADC", "ROR", "ADC", "SEI", "ADC", "PLY", "TDC", "JMP", "ADC", "ROR", "ADC", // 7
	"BRA", "STA", "BRL", "STA", "STY", "STA", "STX", "STA", "DEY", "BIT", "TXA", "PHB", "STY", "STA", "STX", "STA", // 8
	"BCC", "STA", "STA", "STA", "STY", "STA", "STX", "STA", "TYA", "STA", "TXS", "TXY", "STZ", "STA", "STZ", "STA", // 9
	"LDY", "LDA", "LDX", "LDA", "LDY", "LDA", "LDX", "LDA", "TAY", "LDA", "TAX", "PLB", "LDY", "LDA", "LDX", "LDA", // A
	"BCS", "LDA", "LDA", "LDA", "LDY", "LDA", "LDX", "LDA", "CLV", "LDA", "TSX", "TYX", "LDY", "LDA", "LDX", "LDA", // B
	"CPY", "CMP", "REP", "CMP", "CPY", "CMP", "DEC", "CMP", "INY", "CMP", "DEX", "WAI", "CPY", "CMP", "DEC", "CMP", // C
	"BNE", "CMP", "CMP", "CMP", "PEI", "CMP", "DEC", "CMP", "CLD", "CMP", "PHX", "STP", "JML", "CMP", "DEC", "CMP", // D
	"CPX", "SBC", "SEP", "SBC", "CPX", "SBC", "INC", "SBC", "INX", "SBC", "NOP", "XBA", "CPX", "SBC", "INC", "SBC", // E
	"BEQ", "SBC", "SBC", "SBC", "PEA", "SBC", "INC", "SBC", "SED", "SBC", "PLX", "XCE", "JSR", "SBC", "INC", "SBC"  // F
};

typedef SnesAddrMode M;
SnesAddrMode SnesDisUtils::OpMode[256] = {
	//0       1              2            3                 4           5           6           7                 8       9           A       B       C              D           E           F           
	M::Imm8,  M::DirIdxIndX, M::Imm8,     M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Stk, M::ImmM,    M::Acc, M::Stk, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // 0
	M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::Dir,     M::DirIdxX, M::DirIdxX, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Acc, M::Imp, M::Abs,        M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX, // 1
	M::Abs,   M::DirIdxIndX, M::AbsLng,   M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Stk, M::ImmM,    M::Acc, M::Stk, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // 2
	M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::DirIdxX, M::DirIdxX, M::DirIdxX, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Acc, M::Imp, M::AbsIdxX,    M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX, // 3
	M::Stk,   M::DirIdxIndX, M::Imm8,     M::StkRel,        M::BlkMov,  M::Dir,     M::Dir,     M::DirIndLng,     M::Stk, M::ImmM,    M::Acc, M::Stk, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // 4
	M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::BlkMov,  M::DirIdxX, M::DirIdxX, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Stk, M::Imp, M::AbsLng,     M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX, // 5
	M::Stk,   M::DirIdxIndX, M::RelLng,   M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Stk, M::ImmM,    M::Acc, M::Stk, M::AbsInd,     M::Abs,     M::Abs,     M::AbsLng,     // 6
	M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::DirIdxX, M::DirIdxX, M::DirIdxX, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Stk, M::Imp, M::AbsIdxXInd, M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX, // 7
	M::Rel,   M::DirIdxIndX, M::RelLng,   M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Imp, M::ImmM,    M::Imp, M::Stk, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // 8
	M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::DirIdxX, M::DirIdxX, M::DirIdxY, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Imp, M::Imp, M::Abs,        M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX, // 9
	M::ImmX,  M::DirIdxIndX, M::ImmX,     M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Imp, M::ImmM,    M::Imp, M::Stk, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // A
	M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::DirIdxX, M::DirIdxX, M::DirIdxY, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Imp, M::Imp, M::AbsIdxX,    M::AbsIdxX, M::AbsIdxY, M::AbsLngIdxX, // B
	M::ImmX,  M::DirIdxIndX, M::Imm8,     M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Imp, M::ImmM,    M::Imp, M::Imp, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // C
	M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::Dir,     M::DirIdxX, M::DirIdxX, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Stk, M::Imp, M::AbsIndLng,  M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX, // D
	M::ImmX,  M::DirIdxIndX, M::Imm8,     M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Imp, M::ImmM,    M::Imp, M::Imp, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // E
	M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::Imm16,   M::DirIdxX, M::DirIdxX, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Stk, M::Imp, M::AbsIdxXInd, M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX  // F
};
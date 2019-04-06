#include "stdafx.h"
#include "CpuDisUtils.h"
#include "CpuTypes.h"
#include "Cpu.h"
#include "Console.h"
#include "DisassemblyInfo.h"
#include "DummyCpu.h"
#include "../Utilities/HexUtilities.h"
#include "../Utilities/FastString.h"

void CpuDisUtils::GetDisassembly(DisassemblyInfo &info, string &out, uint32_t memoryAddr)
{
	FastString str;

	uint8_t opCode = info.GetOpCode();
	AddrMode addrMode = CpuDisUtils::OpMode[opCode];
	str.Write(CpuDisUtils::OpName[opCode]);
	str.Write(' ');

	uint32_t opAddr = CpuDisUtils::GetOperandAddress(info, memoryAddr);
	uint32_t opSize = info.GetOperandSize() + 1;

	FastString operand;
	if(opSize > 1) {
		operand.Write('$');
		if(addrMode == AddrMode::Rel || addrMode == AddrMode::RelLng) {
			operand.Write(HexUtilities::ToHex24(opAddr));
		} else if(opSize == 2) {
			operand.Write(HexUtilities::ToHex((uint8_t)opAddr));
		} else if(opSize == 3) {
			operand.Write(HexUtilities::ToHex((uint16_t)opAddr));
		} else if(opSize == 4) {
			operand.Write(HexUtilities::ToHex24(opAddr));
		}
	}

	switch(addrMode) {
		case AddrMode::Abs: str.Write(operand); break;
		case AddrMode::AbsJmp: str.Write(operand); break;
		case AddrMode::AbsIdxXInd: str.Write('(', operand, ",X)"); break;
		case AddrMode::AbsIdxX: str.Write(operand, ",X"); break;
		case AddrMode::AbsIdxY: str.Write(operand, ",Y"); break;
		case AddrMode::AbsInd:  str.Write('(', operand, ')'); break;
		case AddrMode::AbsIndLng:  str.Write('[', operand, ']'); break;
		case AddrMode::AbsLngIdxX: str.Write(operand, ",X"); break;
		case AddrMode::AbsLng: str.Write(operand); break;
		case AddrMode::AbsLngJmp: str.Write(operand); break;
		case AddrMode::Acc: break;
		case AddrMode::BlkMov: str.Write('$', operand[1], operand[2], " -> "); str.Write('$', operand[3], operand[4]); break;
		case AddrMode::DirIdxIndX: str.Write('(', operand, ",X)"); break;
		case AddrMode::DirIdxX: str.Write(operand, ",X"); break;
		case AddrMode::DirIdxY: str.Write(operand, ",Y"); break;
		case AddrMode::DirIndIdxY: str.Write("(", operand, "),Y"); break;
		case AddrMode::DirIndLngIdxY: str.Write("[", operand, "],Y"); break;
		case AddrMode::DirIndLng: str.Write("[", operand, "]"); break;
		case AddrMode::DirInd: str.Write("(", operand, ")"); break;
		case AddrMode::Dir: str.Write(operand); break;

		case AddrMode::Imm8: case AddrMode::Imm16: case AddrMode::ImmX: case AddrMode::ImmM:
			str.Write('#', operand);
			break;

		case AddrMode::Sig8: break; //BRK/COP signature, ignore them
		case AddrMode::Imp: break;
		case AddrMode::RelLng: str.Write(operand); break;
		case AddrMode::Rel: str.Write(operand); break;
		case AddrMode::Stk: break;
		case AddrMode::StkRel: str.Write(operand, ",S"); break;
		case AddrMode::StkRelIndIdxY: str.Write('(', operand, ",S),Y"); break;

		default: throw std::runtime_error("invalid address mode");
	}

	out += str.ToString();
}

uint32_t CpuDisUtils::GetOperandAddress(DisassemblyInfo &info, uint32_t memoryAddr)
{
	uint32_t opSize = info.GetOperandSize() + 1;
	uint32_t opAddr = 0;
	uint8_t* byteCode = info.GetByteCode();
	if(opSize == 2) {
		opAddr = byteCode[1];
	} else if(opSize == 3) {
		opAddr = byteCode[1] | (byteCode[2] << 8);
	} else if(opSize == 4) {
		opAddr = byteCode[1] | (byteCode[2] << 8) | (byteCode[3] << 16);
	}

	AddrMode addrMode = CpuDisUtils::OpMode[byteCode[0]];
	if(addrMode == AddrMode::Rel || addrMode == AddrMode::RelLng) {
		if(opSize == 2) {
			opAddr = (memoryAddr & 0xFF0000) | (((int8_t)opAddr + memoryAddr + 2) & 0xFFFF);
		} else {
			opAddr = (memoryAddr & 0xFF0000) | (((int16_t)opAddr + memoryAddr + 3) & 0xFFFF);
		}
	}

	return opAddr;
}

int32_t CpuDisUtils::GetEffectiveAddress(DisassemblyInfo &info, Console *console, CpuState &state)
{
	AddrMode addrMode = CpuDisUtils::OpMode[info.GetOpCode()];
	if(addrMode > AddrMode::ImmM && addrMode != AddrMode::Acc && addrMode != AddrMode::Imp && addrMode != AddrMode::Stk && addrMode != AddrMode::Rel && addrMode != AddrMode::RelLng && addrMode != AddrMode::AbsLngJmp && addrMode != AddrMode::BlkMov) {
		DummyCpu cpu(console);
		state.PS &= ~(ProcFlags::IndexMode8 | ProcFlags::MemoryMode8);
		state.PS |= info.GetFlags();
		cpu.SetDummyState(state);
		cpu.Exec();
		return cpu.GetLastOperand();
	}
	return -1;
}

uint8_t CpuDisUtils::GetOperandSize(AddrMode addrMode, uint8_t flags)
{
	switch(addrMode) {
		case AddrMode::Acc:
		case AddrMode::Imp:
		case AddrMode::Stk:
			return 0;

		case AddrMode::DirIdxIndX:
		case AddrMode::DirIdxX:
		case AddrMode::DirIdxY:
		case AddrMode::DirIndIdxY:
		case AddrMode::DirIndLngIdxY:
		case AddrMode::DirIndLng:
		case AddrMode::DirInd:
		case AddrMode::Dir:
		case AddrMode::Sig8:
		case AddrMode::Imm8:
		case AddrMode::Rel:
		case AddrMode::StkRel:
		case AddrMode::StkRelIndIdxY:
			return 1;

		case AddrMode::Abs:
		case AddrMode::AbsIdxXInd:
		case AddrMode::AbsIdxX:
		case AddrMode::AbsIdxY:
		case AddrMode::AbsInd:
		case AddrMode::AbsIndLng:
		case AddrMode::AbsJmp:
		case AddrMode::BlkMov:
		case AddrMode::Imm16:
		case AddrMode::RelLng:
			return 2;

		case AddrMode::AbsLngJmp:
		case AddrMode::AbsLngIdxX:
		case AddrMode::AbsLng:
			return 3;

		case AddrMode::ImmX: return (flags & ProcFlags::IndexMode8) ? 1 : 2;
		case AddrMode::ImmM: return (flags & ProcFlags::MemoryMode8) ? 1 : 2;
	}

	throw std::runtime_error("Invalid mode");
}

uint8_t CpuDisUtils::GetOperandSize(uint8_t opCode, uint8_t flags)
{
	return GetOperandSize(CpuDisUtils::OpMode[opCode], flags);
}

string CpuDisUtils::OpName[256] = {
	//0    1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
	"BRK", "ORA", "COP", "ORA", "TSB", "ORA", "ASL", "ORA", "PHP", "ORA", "ASL", "PHD", "TSB", "ORA", "ASL", "ORA", // 0
	"BPL", "ORA", "ORA", "ORA", "TRB", "ORA", "ASL", "ORA", "CLC", "ORA", "INC", "TCS", "TRB", "ORA", "ASL", "ORA", // 1
	"JSR", "AND", "JSL", "AND", "BIT", "AND", "ROL", "AND", "PLP", "AND", "ROL", "PLD", "BIT", "AND", "ROL", "AND", // 2
	"BMI", "AND", "AND", "AND", "BIT", "AND", "ROL", "AND", "SEC", "AND", "DEC", "TSC", "BIT", "AND", "ROL", "AND", // 3
	"RTI", "EOR", "WDM", "EOR", "MVP", "EOR", "LSR", "EOR", "PHA", "EOR", "LSR", "PHK", "JMP", "EOR", "LSR", "EOR", // 4
	"BVC", "EOR", "EOR", "EOR", "MVN", "EOR", "LSR", "EOR", "CLI", "EOR", "PHY", "TCD", "JMP", "EOR", "LSR", "EOR", // 5
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

typedef AddrMode M;
AddrMode CpuDisUtils::OpMode[256] = {
	//0       1              2            3                 4           5           6           7                 8       9           A       B       C              D           E           F           
	M::Sig8,  M::DirIdxIndX, M::Sig8,     M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Stk, M::ImmM,    M::Acc, M::Stk, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // 0
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
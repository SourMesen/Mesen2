#include "stdafx.h"
#include <algorithm>
#include "DisassemblyInfo.h"
#include "CpuTypes.h"
#include "MemoryManager.h"
#include "../Utilities/HexUtilities.h"
#include "../Utilities/FastString.h"

DisassemblyInfo::DisassemblyInfo()
{
}

DisassemblyInfo::DisassemblyInfo(CpuState &state, MemoryManager *memoryManager)
{
	_flags = state.PS;

	uint32_t addr = (state.K << 16) | state.PC;
	_byteCode[0] = memoryManager->Peek(addr);
	_addrMode = DisassemblyInfo::OpMode[_byteCode[0]];
	_opSize = GetOperandSize() + 1;

	for(int i = 1; i < _opSize; i++) {
		_byteCode[i] = memoryManager->Peek(addr+i);
	}

	_emulationMode = state.EmulationMode;
}

void DisassemblyInfo::GetDisassembly(string &out, uint32_t memoryAddr)
{
	FastString str;
	str.Write(DisassemblyInfo::OpName[_byteCode[0]]);
	str.Write(' ');

	uint32_t opAddr = GetOperandAddress(memoryAddr);

	FastString operand;
	if(_opSize > 1) {
		operand.Write('$');
		operand.Write(HexUtilities::ToHex(opAddr));
	}


	switch(_addrMode) {
		case AddrMode::Abs: str.Write(operand); break;
		case AddrMode::AbsIdxXInd: str.Write('(', operand, ",X)"); break;
		case AddrMode::AbsIdxX: str.Write(operand, ",X"); break;
		case AddrMode::AbsIdxY: str.Write(operand, ",Y"); break;
		case AddrMode::AbsInd:  str.Write('(', operand, ')'); break;
		case AddrMode::AbsIndLng:  str.Write('[', operand, ']'); break;
		case AddrMode::AbsLngIdxX: str.Write(operand, ",X"); break;
		case AddrMode::AbsLng: str.Write(operand); break;
		case AddrMode::Acc: break;
		case AddrMode::BlkMov: str.Write(operand[0], operand[1], " <- ", operand[2], operand[3]); break; //TODO
		case AddrMode::DirIdxIndX: str.Write('(', operand, ",X)"); break;
		case AddrMode::DirIdxX: str.Write(operand, ",X"); break;
		case AddrMode::DirIdxY: str.Write(operand, ",Y"); break;
		case AddrMode::DirIndIdxY: str.Write("(", operand, "),Y"); break;
		case AddrMode::DirIndLngIdxY: str.Write("[", operand, "],Y"); break;
		case AddrMode::DirIndLng: str.Write("[", operand, "]"); break;
		case AddrMode::DirInd: str.Write("(", operand, ")"); break;
		case AddrMode::Dir: str.Write(operand); break;
		
		case AddrMode::Imm8: case AddrMode::ImmX: case AddrMode::ImmM:
			str.Write('#', operand);
			break;
		
		case AddrMode::Imp: break;;
		case AddrMode::RelLng: str.Write(operand);
		case AddrMode::Rel: str.Write(operand);
		case AddrMode::Stk: break;
		case AddrMode::StkRel: str.Write(operand, ",S"); break;
		case AddrMode::StkRelIndIdxY: str.Write('(', operand, ",S),Y"); break;
	}

	out += str.ToString();
}

uint32_t DisassemblyInfo::GetOperandAddress(uint32_t memoryAddr)
{
	uint32_t opAddr = 0;
	if(_opSize == 2) {
		opAddr = _byteCode[1];
	} else if(_opSize == 3) {
		opAddr = _byteCode[1] | (_byteCode[2] << 8);
	} else if(_opSize == 4) {
		opAddr = _byteCode[1] | (_byteCode[2] << 8) | (_byteCode[3] << 16);
	}

	if(_addrMode == AddrMode::Rel) {
		if(_opSize == 2) {
			opAddr = (int8_t)opAddr + memoryAddr + 2;
		} else {
			opAddr = (int16_t)opAddr + memoryAddr + 2;
		}
	}

	return opAddr;
}

uint8_t DisassemblyInfo::GetOperandSize()
{
	switch(_addrMode) {
		case AddrMode::Abs: return 2;
		case AddrMode::AbsIdxXInd: return 2;
		case AddrMode::AbsIdxX: return 2;
		case AddrMode::AbsIdxY: return 2;
		
		case AddrMode::AbsInd: return 2;
		case AddrMode::AbsIndLng: return 2;

		case AddrMode::AbsLngIdxX: return 3;
		case AddrMode::AbsLng: return 3;

		case AddrMode::Acc: return 0;
		case AddrMode::BlkMov: return 2;
		
		case AddrMode::DirIdxIndX: return 1;
		case AddrMode::DirIdxX: return 1;
		case AddrMode::DirIdxY: return 1;
		case AddrMode::DirIndIdxY: return 1;
		case AddrMode::DirIndLngIdxY: return 1;
		case AddrMode::DirIndLng: return 1;
		case AddrMode::DirInd: return 1;
		case AddrMode::Dir: return 1;

		case AddrMode::Imm8: return 1;
		case AddrMode::ImmX: return (_flags & ProcFlags::IndexMode8) ? 1 : 2;
		case AddrMode::ImmM: return (_flags & ProcFlags::MemoryMode8) ? 1 : 2;
		
		case AddrMode::Imp: return 0;
		case AddrMode::RelLng: return 2;
		case AddrMode::Rel: return 1;

		case AddrMode::Stk: return 0; //TODO
		case AddrMode::StkRel: return 1;
		case AddrMode::StkRelIndIdxY: return 1;
	}

	throw new std::runtime_error("Invalid mode");
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

int32_t DisassemblyInfo::GetEffectiveAddress(CpuState &cpuState, MemoryManager *memoryManager)
{
	uint32_t bank = (cpuState.K << 16);
	//uint32_t opAddr = GetOperandAddress();
	switch(_addrMode) {

	}
}

string DisassemblyInfo::OpName[256] = {
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
AddrMode DisassemblyInfo::OpMode[256] = {
	//0       1              2            3                 4           5           6           7                 8       9           A       B       C              D           E           F           
	M::Stk,   M::DirIdxIndX, M::Stk,      M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Stk, M::ImmM,    M::Acc, M::Stk, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // 0
	M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::Dir,     M::DirIdxX, M::DirIdxX, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Acc, M::Imp, M::Abs,        M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX, // 1
	M::Abs,   M::DirIdxIndX, M::AbsLng,   M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Stk, M::ImmM,    M::Acc, M::Stk, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // 2
	M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::DirIdxX, M::DirIdxX, M::DirIdxX, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Acc, M::Imp, M::AbsIdxX,    M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX, // 3
	M::Stk,   M::DirIdxIndX, M::Imm8,     M::StkRel,        M::BlkMov,  M::Dir,     M::Dir,     M::DirIndLng,     M::Stk, M::ImmM,    M::Acc, M::Stk, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // 4
	M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::BlkMov,  M::DirIdxX, M::DirIdxX, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Stk, M::Imp, M::AbsLng,     M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX, // 5
	M::Stk,   M::DirIdxIndX, M::Stk,      M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Stk, M::ImmM,    M::Acc, M::Stk, M::AbsInd,     M::Abs,     M::Abs,     M::AbsLng,     // 6
	M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::DirIdxX, M::DirIdxX, M::DirIdxX, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Stk, M::Imp, M::AbsIdxXInd, M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX, // 7
	M::Rel,   M::DirIdxIndX, M::RelLng,   M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Imp, M::ImmM,    M::Imp, M::Stk, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // 8
	M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::DirIdxX, M::DirIdxX, M::DirIdxY, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Imp, M::Imp, M::Abs,        M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX, // 9
	M::ImmX,  M::DirIdxIndX, M::ImmX,     M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Imp, M::ImmM,    M::Imp, M::Stk, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // A
	M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::DirIdxX, M::DirIdxX, M::DirIdxY, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Imp, M::Imp, M::AbsIdxX,    M::AbsIdxX, M::AbsIdxY, M::AbsLngIdxX, // B
	M::ImmX,  M::DirIdxIndX, M::Imm8,     M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Imp, M::ImmM,    M::Imp, M::Imp, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // C
	M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::Stk,     M::DirIdxX, M::DirIdxX, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Stk, M::Imp, M::AbsIndLng,  M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX, // D
	M::ImmX,  M::DirIdxIndX, M::Imm8,     M::StkRel,        M::Dir,     M::Dir,     M::Dir,     M::DirIndLng,     M::Imp, M::ImmM,    M::Imp, M::Imp, M::Abs,        M::Abs,     M::Abs,     M::AbsLng,     // E
	M::Rel,   M::DirIndIdxY, M::DirInd,   M::StkRelIndIdxY, M::Stk,     M::DirIdxX, M::DirIdxX, M::DirIndLngIdxY, M::Imp, M::AbsIdxY, M::Stk, M::Imp, M::AbsIdxXInd, M::AbsIdxX, M::AbsIdxX, M::AbsLngIdxX  // F
};
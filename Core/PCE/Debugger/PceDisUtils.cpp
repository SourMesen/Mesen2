#include "stdafx.h"
#include "PCE/Debugger/PceDisUtils.h"
#include "PCE/PceTypes.h"
#include "Shared/EmuSettings.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/LabelManager.h"
#include "Debugger/MemoryDumper.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/FastString.h"
#include "MemoryType.h"

static constexpr uint8_t _opSize[24] = {
	1, 1, 1, 2, 2,
	2, 3, 2, 2,
	3, 2, 2,
	3, 3,
	2, 3,
	7,
	3, 3, 4, 4,
	3
};

static constexpr const char* _opName[256] = {
//	0			1			2			3			4			5			6			7			8			9			A			B			C			D			E			F
	"BRK",	"ORA",	"SXY",	"ST0",	"TSB",	"ORA",	"ASL",	"RMB0",	"PHP",	"ORA",	"ASL",	"NOP*",	"TSB",	"ORA",	"ASL",	"BBR0", //0
	"BPL",	"ORA",	"ORA",	"ST1",	"TRB",	"ORA",	"ASL",	"RMB1",	"CLC",	"ORA",	"INC",	"NOP*",	"TRB",	"ORA",	"ASL",	"BBR1", //1
	"JSR",	"AND",	"SAX",	"ST2",	"BIT",	"AND",	"ROL",	"RMB2",	"PLP",	"AND",	"ROL",	"NOP*",	"BIT",	"AND",	"ROL",	"BBR2", //2
	"BMI",	"AND",	"AND",	"NOP*",	"BIT",	"AND",	"ROL",	"RMB3",	"SEC",	"AND",	"DEC",	"NOP*",	"BIT",	"AND",	"ROL",	"BBR3", //3
	"RTI",	"EOR",	"SAY",	"TMA",	"BSR",	"EOR",	"LSR",	"RMB4",	"PHA",	"EOR",	"LSR",	"NOP*",	"JMP",	"EOR",	"LSR",	"BBR4", //4
	"BVC",	"EOR",	"EOR",	"TAM",	"CSL",	"EOR",	"LSR",	"RMB5",	"CLI",	"EOR",	"PHY",	"NOP*",	"NOP*",	"EOR",	"LSR",	"BBR5", //5
	"RTS",	"ADC",	"CLA",	"NOP*",	"STZ",	"ADC",	"ROR",	"RMB6",	"PLA",	"ADC",	"ROR",	"NOP*",	"JMP",	"ADC",	"ROR",	"BBR6", //6
	"BVS",	"ADC",	"ADC",	"TII",	"STZ",	"ADC",	"ROR",	"RMB7",	"SEI",	"ADC",	"PLY",	"NOP*",	"JMP",	"ADC",	"ROR",	"BBR7", //7
	"BRA",	"STA",	"CLX",	"TST",	"STY",	"STA",	"STX",	"SMB0",	"DEY",	"BIT",	"TXA",	"NOP*",	"STY",	"STA",	"STX",	"BBS0", //8
	"BCC",	"STA",	"STA",	"TST",	"STY",	"STA",	"STX",	"SMB1",	"TYA",	"STA",	"TXS",	"NOP*",	"STZ",	"STA",	"STZ",	"BBS1", //9
	"LDY",	"LDA",	"LDX",	"TST",	"LDY",	"LDA",	"LDX",	"SMB2",	"TAY",	"LDA",	"TAX",	"NOP*",	"LDY",	"LDA",	"LDX",	"BBS2", //A
	"BCS",	"LDA",	"LDA",	"TST",	"LDY",	"LDA",	"LDX",	"SMB3",	"CLV",	"LDA",	"TSX",	"NOP*",	"LDY",	"LDA",	"LDX",	"BBS3", //B
	"CPY",	"CMP",	"CLY",	"TDD",	"CPY",	"CMP",	"DEC",	"SMB4",	"INY",	"CMP",	"DEX",	"NOP*",	"CPY",	"CMP",	"DEC",	"BBS4", //C
	"BNE",	"CMP",	"CMP",	"TIN",	"CSH",	"CMP",	"DEC",	"SMB5",	"CLD",	"CMP",	"PHX",	"NOP*",	"NOP*",	"CMP",	"DEC",	"BBS5", //D
	"CPX",	"SBC",	"NOP*",	"TIA",	"CPX",	"SBC",	"INC",	"SMB6",	"INX",	"SBC",	"NOP",	"NOP*",	"CPX",	"SBC",	"INC",	"BBS6", //E
	"BEQ",	"SBC",	"SBC",	"TAI",	"SET",	"SBC",	"INC",	"SMB7",	"SED",	"SBC",	"PLX",	"NOP*",	"NOP*",	"SBC",	"INC",	"BBS7"  //F
};

typedef PceAddrMode M;
static constexpr PceAddrMode _opMode[] = {
//	0			1				2			3				4				5				6				7				8			9			A			B			C				D			E			F
	M::Imm,	M::IndX,		M::Imp,	M::Imm,		M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Acc,	M::Imp,	M::Abs,		M::Abs,	M::Abs,	M::ZeroRel,	//0
	M::Rel,	M::IndY,		M::ZInd,	M::Imm,		M::Zero,		M::ZeroX,	M::ZeroX,	M::Zero,		M::Imp,	M::AbsY,	M::Imp,	M::Imp,	M::Abs,		M::AbsX,	M::AbsX,	M::ZeroRel,	//1
	M::Abs,	M::IndX,		M::Imp,	M::Imm,		M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Acc,	M::Imp,	M::Abs,		M::Abs,	M::Abs,	M::ZeroRel,	//2
	M::Rel,	M::IndY,		M::ZInd,	M::Imp,		M::ZeroX,	M::ZeroX,	M::ZeroX,	M::Zero,		M::Imp,	M::AbsY,	M::Imp,	M::Imp,	M::AbsX,		M::AbsX,	M::AbsX,	M::ZeroRel,	//3
	M::Imp,	M::IndX,		M::Imp,	M::Imm,		M::Rel,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Acc,	M::Imp,	M::Abs,		M::Abs,	M::Abs,	M::ZeroRel,	//4
	M::Rel,	M::IndY,		M::ZInd,	M::Imm,		M::Imp,		M::ZeroX,	M::ZeroX,	M::Zero,		M::Imp,	M::AbsY,	M::Imp,	M::Imp,	M::Imp,		M::AbsX,	M::AbsX,	M::ZeroRel,	//5
	M::Imp,	M::IndX,		M::Imp,	M::Imp,		M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Acc,	M::Imp,	M::Ind,		M::Abs,	M::Abs,	M::ZeroRel,	//6
	M::Rel,	M::IndY,		M::ZInd,	M::Block,	M::ZeroX,	M::ZeroX,	M::ZeroX,	M::Zero,		M::Imp,	M::AbsY,	M::Imp,	M::Imp,	M::AbsXInd,	M::AbsX,	M::AbsX,	M::ZeroRel,	//7
	M::Rel,	M::IndX,		M::Imp,	M::ImZero,	M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Imp,	M::Imp,	M::Abs,		M::Abs,	M::Abs,	M::ZeroRel,	//8
	M::Rel,	M::IndY,		M::ZInd,	M::ImAbs,	M::ZeroX,	M::ZeroX,	M::ZeroY,	M::Zero,		M::Imp,	M::AbsY,	M::Imp,	M::Imp,	M::Abs,		M::AbsX,	M::AbsX,	M::ZeroRel,	//9
	M::Imm,	M::IndX,		M::Imm,	M::ImZeroX,	M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Imp,	M::Imp,	M::Abs,		M::Abs,	M::Abs,	M::ZeroRel,	//A
	M::Rel,	M::IndY,		M::ZInd,	M::ImAbsX,	M::ZeroX,	M::ZeroX,	M::ZeroY,	M::Zero,		M::Imp,	M::AbsY,	M::Imp,	M::Imp,	M::AbsX,		M::AbsX,	M::AbsY,	M::ZeroRel,	//B
	M::Imm,	M::IndX,		M::Imp,	M::Block,	M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Imp,	M::Imp,	M::Abs,		M::Abs,	M::Abs,	M::ZeroRel,	//C
	M::Rel,	M::IndY,		M::ZInd,	M::Block,	M::Imp,		M::ZeroX,	M::ZeroX,	M::Zero,		M::Imp,	M::AbsY,	M::Imp,	M::Imp,	M::Imp,		M::AbsX,	M::AbsX,	M::ZeroRel,	//D
	M::Imm,	M::IndX,		M::Imp,	M::Block,	M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Imp,	M::Imp,	M::Abs,		M::Abs,	M::Abs,	M::ZeroRel,	//E
	M::Rel,	M::IndY,		M::ZInd,	M::Block,	M::Imp,		M::ZeroX,	M::ZeroX,	M::Zero,		M::Imp,	M::AbsY,	M::Imp,	M::Imp,	M::Imp,		M::AbsX,	M::AbsX,	M::ZeroRel,	//F

};

void PceDisUtils::GetDisassembly(DisassemblyInfo& info, string& out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings)
{
	FastString str(settings->CheckDebuggerFlag(DebuggerFlags::UseLowerCaseDisassembly));

	uint8_t opCode = info.GetOpCode();
	uint8_t* byteCode = info.GetByteCode();
	PceAddrMode addrMode = _opMode[opCode];
	str.Write(_opName[opCode]);
	str.Write(' ');

	auto writeLabelOrAddr = [&str, &info, labelManager](uint16_t addr) {
		AddressInfo address { addr, MemoryType::PceMemory };
		string label = labelManager ? labelManager->GetLabel(address, !info.IsJump()) : "";
		if(label.empty()) {
			str.WriteAll('$', HexUtilities::ToHex(addr));
		} else {
			str.Write(label, true);
		}
	};

	auto writeZeroAddr = [&str, &info, &writeLabelOrAddr, labelManager](uint8_t zero) {
		uint16_t addr = 0x2000 + zero;
		writeLabelOrAddr(addr);
	};

	switch(addrMode) {
		case PceAddrMode::Acc: str.Write('A'); break;
		case PceAddrMode::Imm: str.WriteAll("#$", HexUtilities::ToHex(byteCode[1])); break;
		
		case PceAddrMode::Ind:
			str.Write('(');
			writeLabelOrAddr(byteCode[1] | (byteCode[2] << 8));
			str.Write(')');
			break;

		case PceAddrMode::ZInd:
			str.Write('(');
			writeZeroAddr(byteCode[1]);
			str.Write(')');
			break;

		case PceAddrMode::IndY:
			str.Write('(');
			writeZeroAddr(byteCode[1]);
			str.Write("),Y");
			break;

		case PceAddrMode::IndX:
			str.WriteAll('(');
			writeZeroAddr(byteCode[1]);
			str.Write(",X)");
			break;

		case PceAddrMode::AbsXInd: 
			str.WriteAll('(');
			writeLabelOrAddr(byteCode[1] | (byteCode[2] << 8));
			str.Write(",X)");
			break;

		case PceAddrMode::Abs: writeLabelOrAddr(byteCode[1] | (byteCode[2] << 8)); break;
		case PceAddrMode::Zero: writeZeroAddr(byteCode[1]); break;

		case PceAddrMode::Rel: writeLabelOrAddr(((int8_t)byteCode[1] + memoryAddr + 2) & 0xFFFF); break;

		case PceAddrMode::AbsX: writeLabelOrAddr(byteCode[1] | (byteCode[2] << 8)); str.Write(",X"); break;
		case PceAddrMode::AbsY: writeLabelOrAddr(byteCode[1] | (byteCode[2] << 8)); str.Write(",Y"); break;

		case PceAddrMode::ZeroX: writeZeroAddr(byteCode[1]); str.Write(",X"); break;
		case PceAddrMode::ZeroY: writeZeroAddr(byteCode[1]); str.Write(",Y"); break;

		case PceAddrMode::ZeroRel:
			writeZeroAddr(byteCode[1]);
			str.Write(',');
			writeLabelOrAddr(((int8_t)byteCode[2] + memoryAddr + 3) & 0xFFFF);
			break;

		case PceAddrMode::Block:
			writeLabelOrAddr((uint16_t)(byteCode[1] | (byteCode[2] << 8)));
			str.Write(",");
			writeLabelOrAddr((uint16_t)(byteCode[3] | (byteCode[4] << 8)));
			str.WriteAll(",#$", HexUtilities::ToHex((uint16_t)(byteCode[5] | (byteCode[6] << 8))));
			break;

		case PceAddrMode::ImZero:
			str.WriteAll("#$", HexUtilities::ToHex(byteCode[1]), ",");
			writeZeroAddr(byteCode[2]);
			break;
		
		case PceAddrMode::ImAbs:
			str.WriteAll("#$", HexUtilities::ToHex(byteCode[1]), ",");
			writeLabelOrAddr((uint16_t)(byteCode[2] | (byteCode[3] << 8)));
			break;
		
		case PceAddrMode::ImZeroX:
			str.WriteAll("#$", HexUtilities::ToHex(byteCode[1]), ",");
			writeZeroAddr(byteCode[2]);
			str.WriteAll(",X");
			break;

		case PceAddrMode::ImAbsX:
			str.WriteAll("#$", HexUtilities::ToHex(byteCode[1]), ",");
			writeLabelOrAddr((uint16_t)(byteCode[2] | (byteCode[3] << 8)));
			str.WriteAll(",X");
			break;

		default: break;
	}

	out += str.ToString();
}

int32_t PceDisUtils::GetEffectiveAddress(DisassemblyInfo& info, PceCpuState& state, MemoryDumper* memoryDumper)
{
	uint8_t* byteCode = info.GetByteCode();
	switch(_opMode[info.GetOpCode()]) {
		default: break;

		case PceAddrMode::ZeroX: return (uint8_t)(byteCode[1] + state.X); break;
		case PceAddrMode::ZeroY: return (uint8_t)(byteCode[1] + state.Y); break;

		case PceAddrMode::IndX: {
			uint8_t zeroAddr = byteCode[1] + state.X;
			return memoryDumper->GetMemoryValue(MemoryType::PceMemory, zeroAddr) | memoryDumper->GetMemoryValue(MemoryType::PceMemory, (uint8_t)(zeroAddr + 1)) << 8;
		}

		case PceAddrMode::IndY: {
			uint8_t zeroAddr = byteCode[1];
			uint16_t addr = memoryDumper->GetMemoryValue(MemoryType::PceMemory, zeroAddr) | memoryDumper->GetMemoryValue(MemoryType::PceMemory, (uint8_t)(zeroAddr + 1)) << 8;
			return (uint16_t)(addr + state.Y);
		}

		case PceAddrMode::Ind: {
			uint16_t addr = byteCode[1] | (byteCode[2] << 8);
			if((addr & 0xFF) == 0xFF) {
				//CPU bug when indirect address starts at the end of a page
				uint8_t lo = memoryDumper->GetMemoryValue(MemoryType::PceMemory, addr);
				uint8_t hi = memoryDumper->GetMemoryValue(MemoryType::PceMemory, addr & 0xFF00);
				return lo | (hi << 8);
			} else {
				return memoryDumper->GetMemoryValue(MemoryType::PceMemory, addr);
			}
		}
	
		case PceAddrMode::AbsX:
		case PceAddrMode::AbsXInd:
			return (uint16_t)((byteCode[1] | (byteCode[2] << 8)) + state.X) & 0xFFFF;

		case PceAddrMode::AbsY:
			return (uint16_t)((byteCode[1] | (byteCode[2] << 8)) + state.Y) & 0xFFFF;
	}

	return -1;
}

uint8_t PceDisUtils::GetOpSize(PceAddrMode addrMode)
{
	return _opSize[(int)addrMode];
}

uint8_t PceDisUtils::GetOpSize(uint8_t opCode)
{
	return GetOpSize(_opMode[opCode]);
}

char const* const PceDisUtils::GetOpName(uint8_t opCode)
{
	return _opName[opCode];
}

PceAddrMode PceDisUtils::GetOpMode(uint8_t opCode)
{
	return _opMode[opCode];
}

bool PceDisUtils::IsUnconditionalJump(uint8_t opCode)
{
	switch(opCode) {
		case 0x20: //JSR
		case 0x40: //RTI
		case 0x44: //BSR
		case 0x4C: //JMP (Absolute)
		case 0x60: //RTS
		case 0x6C: //JMP (Indirect)
		case 0x7C: //JMP (Absolute,X)
		case 0x80: //BRA
			return true;

		default:
			return false;
	}
}

bool PceDisUtils::IsConditionalJump(uint8_t opCode)
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

		//BBR
		case 0x0F: case 0x1F: case 0x2F: case 0x3F:
		case 0x4F: case 0x5F: case 0x6F: case 0x7F:

		//BBS
		case 0x8F: case 0x9F: case 0xAF: case 0xBF:
		case 0xCF: case 0xDF: case 0xEF: case 0xFF:
			return true;

		default:
			return false;
	}
}

bool PceDisUtils::IsJumpToSub(uint8_t opCode)
{
	return opCode == 0x20 || opCode == 0x44 || opCode == 0x00; //JSR, BSR, BRK
}

bool PceDisUtils::IsReturnInstruction(uint8_t opCode)
{
	return opCode == 0x60 || opCode == 0x40; //RTS & RTI
}

bool PceDisUtils::IsOpUnofficial(uint8_t opCode)
{
	switch(opCode) {
		case 0x33:
		case 0x5C:
		case 0x63:
		case 0xDC:
		case 0xE2:
		case 0xFC:
			return true;

		default:
			return (opCode & 0x0F) == 0x0B;
	}
}

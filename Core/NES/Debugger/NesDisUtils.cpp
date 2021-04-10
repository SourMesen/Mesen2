#include "stdafx.h"
#include "NES/Debugger/NesDisUtils.h"

#include "NES/NesTypes.h"
#include "Shared/EmuSettings.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/LabelManager.h"
#include "Debugger/MemoryDumper.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/FastString.h"
#include "SnesMemoryType.h"

void NesDisUtils::GetDisassembly(DisassemblyInfo& info, string& out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings)
{
	FastString str(settings->CheckDebuggerFlag(DebuggerFlags::UseLowerCaseDisassembly));

	uint8_t opCode = info.GetOpCode();
	NesAddrMode addrMode = NesDisUtils::OpMode[opCode];
	str.Write(NesDisUtils::OpName[opCode]);
	str.Write(' ');

	uint32_t opAddr = NesDisUtils::GetOperandAddress(info, memoryAddr);
	uint32_t opSize = info.GetOpSize();

	FastString operand(settings->CheckDebuggerFlag(DebuggerFlags::UseLowerCaseDisassembly));
	if(opSize > 1) {
		if(addrMode == NesAddrMode::Rel) {
			AddressInfo address { (int32_t)opAddr, SnesMemoryType::CpuMemory };
			string label = labelManager ? labelManager->GetLabel(address) : "";
			if(label.size()) {
				operand.Write(label, true);
			} else {
				operand.WriteAll('$', HexUtilities::ToHex((uint16_t)opAddr));
			}
		} else if(opSize == 2) {
			operand.WriteAll('$', HexUtilities::ToHex((uint8_t)opAddr));
		} else if(opSize == 3) {
			operand.WriteAll('$', HexUtilities::ToHex((uint16_t)opAddr));
		}
	}

	switch(addrMode) {
		case NesAddrMode::Acc: str.Write('A'); break;
		case NesAddrMode::Imm: str.WriteAll("#", operand); break;
		case NesAddrMode::Ind: str.WriteAll("(", operand, ")"); break;
		case NesAddrMode::IndX: str.WriteAll("(", operand, ",X)"); break;

		case NesAddrMode::IndY:
		case NesAddrMode::IndYW:
			str.WriteAll("(", operand, "),Y");
			break;

		case NesAddrMode::Abs:
		case NesAddrMode::Rel:
		case NesAddrMode::Zero:
			str.Write(operand);
			break;

		case NesAddrMode::AbsX:
		case NesAddrMode::AbsXW:
		case NesAddrMode::ZeroX:
			str.WriteAll(operand, ",X");
			break;

		case NesAddrMode::AbsY:
		case NesAddrMode::AbsYW:
		case NesAddrMode::ZeroY:
			str.WriteAll(operand, ",Y");
			break;

		default: break;
	}

	out += str.ToString();
}

uint32_t NesDisUtils::GetOperandAddress(DisassemblyInfo& info, uint32_t memoryAddr)
{
	uint32_t opSize = info.GetOpSize();
	uint32_t opAddr = 0;
	uint8_t* byteCode = info.GetByteCode();
	if(opSize == 2) {
		opAddr = byteCode[1];
	} else if(opSize == 3) {
		opAddr = byteCode[1] | (byteCode[2] << 8);
	}

	NesAddrMode addrMode = NesDisUtils::OpMode[byteCode[0]];
	if(addrMode == NesAddrMode::Rel) {
		opAddr = (((int8_t)opAddr + memoryAddr + 2) & 0xFFFF);
	}

	return opAddr;
}

int32_t NesDisUtils::GetEffectiveAddress(DisassemblyInfo& info, NesCpuState& state, MemoryDumper* memoryDumper)
{
	uint8_t* byteCode = info.GetByteCode();
	switch(NesDisUtils::OpMode[info.GetOpCode()]) {
		default: break;

		case NesAddrMode::ZeroX: return (uint8_t)(byteCode[1] + state.X); break;
		case NesAddrMode::ZeroY: return (uint8_t)(byteCode[1] + state.Y); break;

		case NesAddrMode::IndX: {
			uint8_t zeroAddr = byteCode[1] + state.X;
			return memoryDumper->GetMemoryValue(SnesMemoryType::NesMemory, zeroAddr) | memoryDumper->GetMemoryValue(SnesMemoryType::NesMemory, (uint8_t)(zeroAddr + 1)) << 8;
		}

		case NesAddrMode::IndY:
		case NesAddrMode::IndYW: {
			uint8_t zeroAddr = byteCode[1];
			uint16_t addr = memoryDumper->GetMemoryValue(SnesMemoryType::NesMemory, zeroAddr) | memoryDumper->GetMemoryValue(SnesMemoryType::NesMemory, (uint8_t)(zeroAddr + 1)) << 8;
			return (uint16_t)(addr + state.Y);
		}

		case NesAddrMode::Ind: {
			uint16_t addr = byteCode[1] | (byteCode[2] << 8);
			if((addr & 0xFF) == 0xFF) {
				//CPU bug when indirect address starts at the end of a page
				uint8_t lo = memoryDumper->GetMemoryValue(SnesMemoryType::NesMemory, addr);
				uint8_t hi = memoryDumper->GetMemoryValue(SnesMemoryType::NesMemory, addr & 0xFF00);
				return lo | (hi << 8);
			} else {
				return memoryDumper->GetMemoryValue(SnesMemoryType::NesMemory, addr);
			}
		}
	
		case NesAddrMode::AbsX:
		case NesAddrMode::AbsXW:
			return (uint16_t)((byteCode[1] | (byteCode[2] << 8)) + state.X) & 0xFFFF;

		case NesAddrMode::AbsY:
		case NesAddrMode::AbsYW:
			return (uint16_t)((byteCode[1] | (byteCode[2] << 8)) + state.Y) & 0xFFFF;
	}

	return -1;
}

uint8_t NesDisUtils::GetOpSize(NesAddrMode addrMode)
{
	return NesDisUtils::OpSize[(int)addrMode];
}

uint8_t NesDisUtils::GetOpSize(uint8_t opCode)
{
	return GetOpSize(NesDisUtils::OpMode[opCode]);
}

uint8_t NesDisUtils::OpSize[17] = {
	1, 1, 1, 2, 2,
	2, 3, 2, 2,
	3, 2, 2, 2,
	3, 3, 3, 3
};

string NesDisUtils::OpName[256] = {
//	0			1			2			3			4			5			6			7			8			9			A			B			C			D			E			F
	"BRK",	"ORA",	"STP",	"SLO",	"NOP",	"ORA",	"ASL",	"SLO",	"PHP",	"ORA",	"ASL",	"ANC",	"NOP",	"ORA",	"ASL",	"SLO", //0
	"BPL",	"ORA",	"STP",	"SLO",	"NOP",	"ORA",	"ASL",	"SLO",	"CLC",	"ORA",	"NOP",	"SLO",	"NOP",	"ORA",	"ASL",	"SLO", //1
	"JSR",	"AND",	"STP",	"RLA",	"BIT",	"AND",	"ROL",	"RLA",	"PLP",	"AND",	"ROL",	"ANC",	"BIT",	"AND",	"ROL",	"RLA", //2
	"BMI",	"AND",	"STP",	"RLA",	"NOP",	"AND",	"ROL",	"RLA",	"SEC",	"AND",	"NOP",	"RLA",	"NOP",	"AND",	"ROL",	"RLA", //3
	"RTI",	"EOR",	"STP",	"SRE",	"NOP",	"EOR",	"LSR",	"SRE",	"PHA",	"EOR",	"LSR",	"ALR",	"JMP",	"EOR",	"LSR",	"SRE", //4
	"BVC",	"EOR",	"STP",	"SRE",	"NOP",	"EOR",	"LSR",	"SRE",	"CLI",	"EOR",	"NOP",	"SRE",	"NOP",	"EOR",	"LSR",	"SRE", //5
	"RTS",	"ADC",	"STP",	"RRA",	"NOP",	"ADC",	"ROR",	"RRA",	"PLA",	"ADC",	"ROR",	"ARR",	"JMP",	"ADC",	"ROR",	"RRA", //6
	"BVS",	"ADC",	"STP",	"RRA",	"NOP",	"ADC",	"ROR",	"RRA",	"SEI",	"ADC",	"NOP",	"RRA",	"NOP",	"ADC",	"ROR",	"RRA", //7
	"NOP",	"STA",	"NOP",	"SAX",	"STY",	"STA",	"STX",	"SAX",	"DEY",	"NOP",	"TXA",	"XAA",	"STY",	"STA",	"STX",	"SAX", //8
	"BCC",	"STA",	"STP",	"AHX",	"STY",	"STA",	"STX",	"SAX",	"TYA",	"STA",	"TXS",	"TAS",	"SHY",	"STA",	"SHX",	"AXA", //9
	"LDY",	"LDA",	"LDX",	"LAX",	"LDY",	"LDA",	"LDX",	"LAX",	"TAY",	"LDA",	"TAX",	"LAX",	"LDY",	"LDA",	"LDX",	"LAX", //A
	"BCS",	"LDA",	"STP",	"LAX",	"LDY",	"LDA",	"LDX",	"LAX",	"CLV",	"LDA",	"TSX",	"LAS",	"LDY",	"LDA",	"LDX",	"LAX", //B
	"CPY",	"CMP",	"NOP",	"DCP",	"CPY",	"CMP",	"DEC",	"DCP",	"INY",	"CMP",	"DEX",	"AXS",	"CPY",	"CMP",	"DEC",	"DCP", //C
	"BNE",	"CMP",	"STP",	"DCP",	"NOP",	"CMP",	"DEC",	"DCP",	"CLD",	"CMP",	"NOP",	"DCP",	"NOP",	"CMP",	"DEC",	"DCP", //D
	"CPX",	"SBC",	"NOP",	"ISC",	"CPX",	"SBC",	"INC",	"ISC",	"INX",	"SBC",	"NOP",	"SBC",	"CPX",	"SBC",	"INC",	"ISC", //E
	"BEQ",	"SBC",	"STP",	"ISC",	"NOP",	"SBC",	"INC",	"ISC",	"SED",	"SBC",	"NOP",	"ISC",	"NOP",	"SBC",	"INC",	"ISC"  //F
};

bool unofficial[256] = {
	//	0		1		2		3		4		5		6		7		8		9		A		B		C		D		E		F
		false,false,true, true, true, false,false,true, false,false,false,true, true, false,false,true, //0
		false,false,true, true, true, false,false,true, false,false,true, true, true, false,false,true, //1
		false,false,true, true, false,false,false,true, false,false,false,true, false,false,false,true, //2
		false,false,true, true, true, false,false,true, false,false,true, true, true, false,false,true, //3
		false,false,true, true, true, false,false,true, false,false,false,true, false,false,false,true, //4
		false,false,true, true, true, false,false,true, false,false,true, true, true, false,false,true, //5
		false,false,true, true, true, false,false,true, false,false,false,true, false,false,false,true, //6
		false,false,true, true, true, false,false,true, false,false,true, true, true, false,false,true, //7
		true, false,true, true, false,false,false,true, false,true, false,true, false,false,false,true, //8
		false,false,true, true, false,false,false,true, false,false,false,true, true, false,true, true, //9
		false,false,false,true, false,false,false,true, false,false,false,true, false,false,false,true, //A
		false,false,true, true, false,false,false,true, false,false,false,true, false,false,false,true, //B
		false,false,true, true, false,false,false,true, false,false,false,true, false,false,false,true, //C
		false,false,true, true, true, false,false,true, false,false,true, true, true, false,false,true, //D
		false,false,true, true, false,false,false,true, false,false,false,true, false,false,false,true, //E
		false,false,true, true, true, false,false,true, false,false,true, true, true, false,false,true  //F
};

typedef NesAddrMode M;
NesAddrMode NesDisUtils::OpMode[] = {
//	0			1				2			3				4				5				6				7				8			9			A			B			C			D			E			F
	M::Imp,	M::IndX,		M::None,	M::IndX,		M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Acc,	M::Imm,	M::Abs,	M::Abs,	M::Abs,	M::Abs,	//0
	M::Rel,	M::IndY,		M::None,	M::IndYW,	M::ZeroX,	M::ZeroX,	M::ZeroX,	M::ZeroX,	M::Imp,	M::AbsY,	M::Imp,	M::AbsYW,M::AbsX,	M::AbsX,	M::AbsXW,M::AbsXW,//1
	M::Abs,	M::IndX,		M::None,	M::IndX,		M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Acc,	M::Imm,	M::Abs,	M::Abs,	M::Abs,	M::Abs,	//2
	M::Rel,	M::IndY,		M::None,	M::IndYW,	M::ZeroX,	M::ZeroX,	M::ZeroX,	M::ZeroX,	M::Imp,	M::AbsY,	M::Imp,	M::AbsYW,M::AbsX,	M::AbsX,	M::AbsXW,M::AbsXW,//3
	M::Imp,	M::IndX,		M::None,	M::IndX,		M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Acc,	M::Imm,	M::Abs,	M::Abs,	M::Abs,	M::Abs,	//4
	M::Rel,	M::IndY,		M::None,	M::IndYW,	M::ZeroX,	M::ZeroX,	M::ZeroX,	M::ZeroX,	M::Imp,	M::AbsY,	M::Imp,	M::AbsYW,M::AbsX,	M::AbsX,	M::AbsXW,M::AbsXW,//5
	M::Imp,	M::IndX,		M::None,	M::IndX,		M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Acc,	M::Imm,	M::Ind,	M::Abs,	M::Abs,	M::Abs,	//6
	M::Rel,	M::IndY,		M::None,	M::IndYW,	M::ZeroX,	M::ZeroX,	M::ZeroX,	M::ZeroX,	M::Imp,	M::AbsY,	M::Imp,	M::AbsYW,M::AbsX,	M::AbsX,	M::AbsXW,M::AbsXW,//7
	M::Imm,	M::IndX,		M::Imm,	M::IndX,		M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Imp,	M::Imm,	M::Abs,	M::Abs,	M::Abs,	M::Abs,	//8
	M::Rel,	M::IndYW,	M::None,	M::IndYW,	M::ZeroX,	M::ZeroX,	M::ZeroY,	M::ZeroY,	M::Imp,	M::AbsYW,M::Imp,	M::AbsYW,M::AbsXW,M::AbsXW,M::AbsYW,M::AbsYW,//9
	M::Imm,	M::IndX,		M::Imm,	M::IndX,		M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Imp,	M::Imm,	M::Abs,	M::Abs,	M::Abs,	M::Abs,	//A
	M::Rel,	M::IndY,		M::None,	M::IndY,		M::ZeroX,	M::ZeroX,	M::ZeroY,	M::ZeroY,	M::Imp,	M::AbsY,	M::Imp,	M::AbsY,	M::AbsX,	M::AbsX,	M::AbsY,	M::AbsY,	//B
	M::Imm,	M::IndX,		M::Imm,	M::IndX,		M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Imp,	M::Imm,	M::Abs,	M::Abs,	M::Abs,	M::Abs,	//C
	M::Rel,	M::IndY,		M::None,	M::IndYW,	M::ZeroX,	M::ZeroX,	M::ZeroX,	M::ZeroX,	M::Imp,	M::AbsY,	M::Imp,	M::AbsYW,M::AbsX,	M::AbsX,	M::AbsXW,M::AbsXW,//D
	M::Imm,	M::IndX,		M::Imm,	M::IndX,		M::Zero,		M::Zero,		M::Zero,		M::Zero,		M::Imp,	M::Imm,	M::Imp,	M::Imm,	M::Abs,	M::Abs,	M::Abs,	M::Abs,	//E
	M::Rel,	M::IndY,		M::None,	M::IndYW,	M::ZeroX,	M::ZeroX,	M::ZeroX,	M::ZeroX,	M::Imp,	M::AbsY,	M::Imp,	M::AbsYW,M::AbsX,	M::AbsX,	M::AbsXW,M::AbsXW,//F
};
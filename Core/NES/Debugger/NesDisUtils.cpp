#include "pch.h"
#include "NES/Debugger/NesDisUtils.h"

#include "NES/NesTypes.h"
#include "Shared/EmuSettings.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/LabelManager.h"
#include "Debugger/MemoryDumper.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/FastString.h"
#include "Shared/MemoryType.h"

static constexpr uint8_t _opSize[17] = {
	1, 1, 1, 2, 2,
	2, 3, 2, 2,
	3, 2, 2, 2,
	3, 3, 3, 3
};

static constexpr const char* _opName[256] = {
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

typedef NesAddrMode M;
static constexpr NesAddrMode _opMode[] = {
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

static constexpr bool _isUnofficial[256] = {
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

void NesDisUtils::GetDisassembly(DisassemblyInfo& info, string& out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings)
{
	FastString str(settings->GetDebugConfig().UseLowerCaseDisassembly);

	uint8_t opCode = info.GetOpCode();
	NesAddrMode addrMode = _opMode[opCode];
	str.Write(_opName[opCode]);
	str.Write(' ');

	uint32_t opAddr = NesDisUtils::GetOperandAddress(info, memoryAddr);
	uint32_t opSize = info.GetOpSize();

	FastString operand(settings->GetDebugConfig().UseLowerCaseDisassembly);
	if(opSize > 1) {
		if(addrMode != NesAddrMode::Imm) {
			AddressInfo address { (int32_t)opAddr, MemoryType::NesMemory };
			string label = labelManager ? labelManager->GetLabel(address, !info.IsJump()) : "";
			if(label.size()) {
				operand.Write(label, true);
			}
		} 
		
		if(operand.GetSize() == 0) {
			if(opSize == 3 || addrMode == NesAddrMode::Rel) {
				operand.WriteAll('$', HexUtilities::ToHex((uint16_t)opAddr));
			} else if(opSize == 2) {
				operand.WriteAll('$', HexUtilities::ToHex((uint8_t)opAddr));
			}
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

	NesAddrMode addrMode = _opMode[byteCode[0]];
	if(addrMode == NesAddrMode::Rel) {
		opAddr = (((int8_t)opAddr + memoryAddr + 2) & 0xFFFF);
	}

	return opAddr;
}

EffectiveAddressInfo NesDisUtils::GetEffectiveAddress(DisassemblyInfo& info, NesCpuState& state, MemoryDumper* memoryDumper)
{
	bool isJump = NesDisUtils::IsUnconditionalJump(info.GetOpCode()) || NesDisUtils::IsConditionalJump(info.GetOpCode());
	NesAddrMode addrMode = _opMode[info.GetOpCode()];
	if(isJump && addrMode != NesAddrMode::Ind) {
		//For jumps, show no address/value (except indirect jump)
		return { };
	}

	uint8_t* byteCode = info.GetByteCode();
	switch(_opMode[info.GetOpCode()]) {
		default: break;

		case NesAddrMode::Abs: return { byteCode[1] | (byteCode[2] << 8), 1, false };
		case NesAddrMode::Zero: return { byteCode[1], 1, false };

		case NesAddrMode::ZeroX: return { (uint8_t)(byteCode[1] + state.X), 1, true }; break;
		case NesAddrMode::ZeroY: return { (uint8_t)(byteCode[1] + state.Y), 1, true }; break;

		case NesAddrMode::IndX: {
			uint8_t zeroAddr = byteCode[1] + state.X;
			return { memoryDumper->GetMemoryValue(MemoryType::NesMemory, zeroAddr) | memoryDumper->GetMemoryValue(MemoryType::NesMemory, (uint8_t)(zeroAddr + 1)) << 8, 1, true };
		}

		case NesAddrMode::IndY:
		case NesAddrMode::IndYW: {
			uint8_t zeroAddr = byteCode[1];
			uint16_t addr = memoryDumper->GetMemoryValue(MemoryType::NesMemory, zeroAddr) | memoryDumper->GetMemoryValue(MemoryType::NesMemory, (uint8_t)(zeroAddr + 1)) << 8;
			return { (uint16_t)(addr + state.Y), 1, true };
		}

		case NesAddrMode::Ind: {
			uint16_t addr = byteCode[1] | (byteCode[2] << 8);
			uint8_t lo = memoryDumper->GetMemoryValue(MemoryType::NesMemory, addr);
			//CPU bug when indirect address starts at the end of a page
			uint8_t hi = memoryDumper->GetMemoryValue(MemoryType::NesMemory, (addr & 0xFF00) | ((addr + 1) & 0xFF));
			return { lo | (hi << 8), 1, true };
		}
	
		case NesAddrMode::AbsX:
		case NesAddrMode::AbsXW:
			return { (uint16_t)((byteCode[1] | (byteCode[2] << 8)) + state.X) & 0xFFFF, 1, true };

		case NesAddrMode::AbsY:
		case NesAddrMode::AbsYW:
			return { (uint16_t)((byteCode[1] | (byteCode[2] << 8)) + state.Y) & 0xFFFF, 1, true };
	}

	return {};
}

uint8_t NesDisUtils::GetOpSize(NesAddrMode addrMode)
{
	return _opSize[(int)addrMode];
}

uint8_t NesDisUtils::GetOpSize(uint8_t opCode)
{
	return GetOpSize(_opMode[opCode]);
}

char const* const NesDisUtils::GetOpName(uint8_t opCode)
{
	return _opName[opCode];
}

NesAddrMode NesDisUtils::GetOpMode(uint8_t opCode)
{
	return _opMode[opCode];
}

bool NesDisUtils::IsOpUnofficial(uint8_t opCode)
{
	return _isUnofficial[opCode];
}

bool NesDisUtils::IsUnconditionalJump(uint8_t opCode)
{
	switch(opCode) {
		case 0x00: //BRK
		case 0x20: //JSR
		case 0x40: //RTI
		case 0x4C: //JMP (Absolute)
		case 0x60: //RTS
		case 0x6C: //JMP (Indirect)
			return true;

		default:
			return false;
	}
}

bool NesDisUtils::IsConditionalJump(uint8_t opCode)
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

CdlFlags::CdlFlags NesDisUtils::GetOpFlags(uint8_t opCode, uint16_t pc, uint16_t prevPc)
{
	switch(opCode) {
		case 0x00: //BRK
		case 0x20: //JSR
			return CdlFlags::SubEntryPoint;
		
		case 0x10: //BPL
		case 0x30: //BMI
		case 0x4C: //JMP (Absolute)
		case 0x50: //BVC
		case 0x6C: //JMP (Indirect)
		case 0x70: //BVS
		case 0x90: //BCC
		case 0xB0: //BCS
		case 0xD0: //BNE
		case 0xF0: //BEQ
			return pc != prevPc + NesDisUtils::GetOpSize(opCode) ? CdlFlags::JumpTarget : CdlFlags::None;

		default:
			return CdlFlags::None;
	}
}

bool NesDisUtils::IsJumpToSub(uint8_t opCode)
{
	return opCode == 0x20 || opCode == 0x00; //JSR, BRK
}

bool NesDisUtils::IsReturnInstruction(uint8_t opCode)
{
	return opCode == 0x60 || opCode == 0x40; //RTS, RTI
}
#include "pch.h"
#include "PCE/Debugger/PceDisUtils.h"
#include "PCE/Debugger/DummyPceCpu.h"
#include "PCE/PceConsole.h"
#include "PCE/PceTypes.h"
#include "Shared/EmuSettings.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/LabelManager.h"
#include "Debugger/MemoryDumper.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/FastString.h"
#include "Shared/MemoryType.h"

static constexpr uint8_t _opSize[22] = {
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
	"BRK",	"ORA",	"SXY",	"ST0",	"TSB",	"ORA",	"ASL",	"RMB0",	"PHP",	"ORA",	"ASL",	"NOP_0B","TSB",	"ORA",	"ASL",	"BBR0", //0
	"BPL",	"ORA",	"ORA",	"ST1",	"TRB",	"ORA",	"ASL",	"RMB1",	"CLC",	"ORA",	"INC",	"NOP_1B","TRB",	"ORA",	"ASL",	"BBR1", //1
	"JSR",	"AND",	"SAX",	"ST2",	"BIT",	"AND",	"ROL",	"RMB2",	"PLP",	"AND",	"ROL",	"NOP_2B","BIT",	"AND",	"ROL",	"BBR2", //2
	"BMI",	"AND",	"AND",	"NOP_33","BIT",	"AND",	"ROL",	"RMB3",	"SEC",	"AND",	"DEC",	"NOP_3B","BIT",	"AND",	"ROL",	"BBR3", //3
	"RTI",	"EOR",	"SAY",	"TMA",	"BSR",	"EOR",	"LSR",	"RMB4",	"PHA",	"EOR",	"LSR",	"NOP_4B","JMP",	"EOR",	"LSR",	"BBR4", //4
	"BVC",	"EOR",	"EOR",	"TAM",	"CSL",	"EOR",	"LSR",	"RMB5",	"CLI",	"EOR",	"PHY",	"NOP_5B","NOP_5C","EOR",	"LSR",	"BBR5", //5
	"RTS",	"ADC",	"CLA",	"NOP_63","STZ",	"ADC",	"ROR",	"RMB6",	"PLA",	"ADC",	"ROR",	"NOP_6B","JMP",	"ADC",	"ROR",	"BBR6", //6
	"BVS",	"ADC",	"ADC",	"TII",	"STZ",	"ADC",	"ROR",	"RMB7",	"SEI",	"ADC",	"PLY",	"NOP_7B","JMP",	"ADC",	"ROR",	"BBR7", //7
	"BRA",	"STA",	"CLX",	"TST",	"STY",	"STA",	"STX",	"SMB0",	"DEY",	"BIT",	"TXA",	"NOP_8B","STY",	"STA",	"STX",	"BBS0", //8
	"BCC",	"STA",	"STA",	"TST",	"STY",	"STA",	"STX",	"SMB1",	"TYA",	"STA",	"TXS",	"NOP_9B","STZ",	"STA",	"STZ",	"BBS1", //9
	"LDY",	"LDA",	"LDX",	"TST",	"LDY",	"LDA",	"LDX",	"SMB2",	"TAY",	"LDA",	"TAX",	"NOP_AB","LDY",	"LDA",	"LDX",	"BBS2", //A
	"BCS",	"LDA",	"LDA",	"TST",	"LDY",	"LDA",	"LDX",	"SMB3",	"CLV",	"LDA",	"TSX",	"NOP_BB","LDY",	"LDA",	"LDX",	"BBS3", //B
	"CPY",	"CMP",	"CLY",	"TDD",	"CPY",	"CMP",	"DEC",	"SMB4",	"INY",	"CMP",	"DEX",	"NOP_CB","CPY",	"CMP",	"DEC",	"BBS4", //C
	"BNE",	"CMP",	"CMP",	"TIN",	"CSH",	"CMP",	"DEC",	"SMB5",	"CLD",	"CMP",	"PHX",	"NOP_DB","NOP_DC","CMP",	"DEC",	"BBS5", //D
	"CPX",	"SBC",	"NOP_E2","TIA",	"CPX",	"SBC",	"INC",	"SMB6",	"INX",	"SBC",	"NOP",	"NOP_EB","CPX",	"SBC",	"INC",	"BBS6", //E
	"BEQ",	"SBC",	"SBC",	"TAI",	"SET",	"SBC",	"INC",	"SMB7",	"SED",	"SBC",	"PLX",	"NOP_FB","NOP_FC","SBC",	"INC",	"BBS7"  //F
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
	FastString str(settings->GetDebugConfig().UseLowerCaseDisassembly);

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

	auto writeZeroAddr = [&str](uint8_t zero) {
		str.WriteAll('$', HexUtilities::ToHex(zero));
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

EffectiveAddressInfo PceDisUtils::GetEffectiveAddress(DisassemblyInfo& info, PceConsole* console, PceCpuState& state)
{
	bool isJump = PceDisUtils::IsUnconditionalJump(info.GetOpCode()) || PceDisUtils::IsConditionalJump(info.GetOpCode());
	if(isJump) {
		//For jumps, show no address/value
		return { };
	}

	bool showEffectiveAddress = false;
	switch(_opMode[info.GetOpCode()]) {
		case PceAddrMode::Imp:
			//Show no address/value for stack operations, etc.
			return {};

		case PceAddrMode::Zero:
		case PceAddrMode::ZeroX:
		case PceAddrMode::ZeroY:
		case PceAddrMode::IndX:
		case PceAddrMode::IndY:
		case PceAddrMode::Ind:
		case PceAddrMode::AbsX:
		case PceAddrMode::AbsXInd:
		case PceAddrMode::AbsY:
		case PceAddrMode::ZInd:
		case PceAddrMode::ZeroRel:
		case PceAddrMode::ImZero:
		case PceAddrMode::ImZeroX:
		case PceAddrMode::ImAbs:
		case PceAddrMode::ImAbsX:
			showEffectiveAddress = true;
			break;

		default:
			showEffectiveAddress = false;
			break;
	}

	DummyPceCpu pceCpu(nullptr, console->GetMemoryManager());
	pceCpu.SetDummyState(state);
	pceCpu.Exec();

	uint32_t count = pceCpu.GetOperationCount();
	for(int i = count - 1; i > 0; i--) {
		MemoryOperationInfo opInfo = pceCpu.GetOperationInfo(i);
		if(opInfo.Type != MemoryOperationType::ExecOperand && opInfo.Type != MemoryOperationType::DummyRead) {
			return { (int32_t)opInfo.Address, 1, showEffectiveAddress };
		}
	}

	return {};
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

CdlFlags::CdlFlags PceDisUtils::GetOpFlags(uint8_t opCode, uint16_t pc, uint16_t prevPc)
{
	switch(opCode) {
		case 0x00: //BRK
		case 0x20: //JSR
		case 0x44: //BSR
			return CdlFlags::SubEntryPoint;

		case 0x10: //BPL
		case 0x30: //BMI
		case 0x4C: //JMP (Absolute)
		case 0x50: //BVC
		case 0x6C: //JMP (Indirect)
		case 0x70: //BVS
		case 0x7C: //JMP (Absolute,X)
		case 0x80: //BRA
		case 0x90: //BCC
		case 0xB0: //BCS
		case 0xD0: //BNE
		case 0xF0: //BEQ
		case 0x0F: case 0x1F: case 0x2F: case 0x3F: //BBR
		case 0x4F: case 0x5F: case 0x6F: case 0x7F: //BBR
		case 0x8F: case 0x9F: case 0xAF: case 0xBF: //BBS
		case 0xCF: case 0xDF: case 0xEF: case 0xFF: //BBS
			return pc != prevPc + PceDisUtils::GetOpSize(opCode) ? CdlFlags::JumpTarget : CdlFlags::None;

		default:
			return CdlFlags::None;
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

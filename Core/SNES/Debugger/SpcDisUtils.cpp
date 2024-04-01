#include "pch.h"
#include "SNES/SnesConsole.h"
#include "SNES/Spc.h"
#include "SNES/Debugger/SpcDisUtils.h"
#include "SNES/Debugger/DummySpc.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/LabelManager.h"
#include "Shared/EmuSettings.h"
#include "Utilities/FastString.h"
#include "Utilities/HexUtilities.h"

constexpr const char* _opTemplate[256] = {
	"NOP",	"TCALL 0",	"SET1 d.0",	"BBS d.0,q",	"OR A,d",		"OR A,!a",		"OR A,(X)",		"OR A,[d+X]",	"OR A,#i",	"OR t,s",		"OR1 C,m.b",	"ASL d",			"ASL !a",	"PUSH PSW",	"TSET1 !a",		"BRK",
	"BPL r",	"TCALL 1",	"CLR1 d.0",	"BBC d.0,q",	"OR A,d+X",		"OR A,!a+X",	"OR A,!a+Y",	"OR A,[d]+Y",	"OR e,#i",	"OR (X),(Y)",	"DECW d",		"ASL d+X",		"ASL A",		"DEC X",		"CMP X,!a",		"JMP [!a+X]",
	"CLRP",	"TCALL 2",	"SET1 d.1",	"BBS d.1,q",	"AND A,d",		"AND A,!a",		"AND A,(X)",	"AND A,[d+X]",	"AND A,#i",	"AND t,s",		"OR1 C,/m.b",	"ROL d",			"ROL !a",	"PUSH A",	"CBNE d,q",		"BRA r",
	"BMI r",	"TCALL 3",	"CLR1 d.1",	"BBC d.1,q",	"AND A,d+X",	"AND A,!a+X",	"AND A,!a+Y",	"AND A,[d]+Y",	"AND e,#i",	"AND (X),(Y)",	"INCW d",		"ROL d+X",		"ROL A",		"INC X",		"CMP X,d",		"CALL !a",
	"SETP",	"TCALL 4",	"SET1 d.2",	"BBS d.2,q",	"EOR A,d",		"EOR A,!a",		"EOR A,(X)",	"EOR A,[d+X]",	"EOR A,#i",	"EOR t,s",		"AND1 C,m.b",	"LSR d",			"LSR !a",	"PUSH X",	"TCLR1 !a",		"PCALL u",
	"BVC r",	"TCALL 5",	"CLR1 d.2",	"BBC d.2,q",	"EOR A,d+X",	"EOR A,!a+X",	"EOR A,!a+Y",	"EOR A,[d]+Y",	"EOR e,#i",	"EOR (X),(Y)",	"CMPW YA,d",	"LSR d+X",		"LSR A",		"MOV X,A",	"CMP Y,!a",		"JMP !a",
	"CLRC",	"TCALL 6",	"SET1 d.3",	"BBS d.3,q",	"CMP A,d",		"CMP A,!a",		"CMP A,(X)",	"CMP A,[d+X]",	"CMP A,#i",	"CMP t,s",		"AND1 C,/m.b",	"ROR d",			"ROR !a",	"PUSH Y",	"DBNZ d,q",		"RET",
	"BVS r",	"TCALL 7",	"CLR1 d.3",	"BBC d.3,q",	"CMP A,d+X",	"CMP A,!a+X",	"CMP A,!a+Y",	"CMP A,[d]+Y",	"CMP e,#i",	"CMP (X),(Y)",	"ADDW YA,d",	"ROR d+X",		"ROR A",		"MOV A,X",	"CMP Y,d",		"RET1",
	"SETC",	"TCALL 8",	"SET1 d.4",	"BBS d.4,q",	"ADC A,d",		"ADC A,!a",		"ADC A,(X)",	"ADC A,[d+X]",	"ADC A,#i",	"ADC t,s",		"EOR1 C,m.b",	"DEC d",			"DEC !a",	"MOV Y,#i",	"POP PSW",		"MOV e,#i",
	"BCC r",	"TCALL 9",	"CLR1 d.4",	"BBC d.4,q",	"ADC A,d+X",	"ADC A,!a+X",	"ADC A,!a+Y",	"ADC A,[d]+Y",	"ADC e,#i",	"ADC (X),(Y)",	"SUBW YA,d",	"DEC d+X",		"DEC A",		"MOV X,SP",	"DIV YA,X",		"XCN A",
	"EI",		"TCALL 10",	"SET1 d.5",	"BBS d.5,q",	"SBC A,d",		"SBC A,!a",		"SBC A,(X)",	"SBC A,[d+X]",	"SBC A,#i",	"SBC t,s",		"MOV1 C,m.b",	"INC d",			"INC !a",	"CMP Y,#i",	"POP A",			"MOV (X)+,A",
	"BCS r",	"TCALL 11",	"CLR1 d.5",	"BBC d.5,q",	"SBC A,d+X",	"SBC A,!a+X",	"SBC A,!a+Y",	"SBC A,[d]+Y",	"SBC e,#i",	"SBC (X),(Y)",	"MOVW YA,d",	"INC d+X",		"INC A",		"MOV SP,X",	"DAS A",			"MOV A,(X)+",
	"DI",		"TCALL 12",	"SET1 d.6",	"BBS d.6,q",	"MOV d,A",		"MOV !a,A",		"MOV (X),A",	"MOV [d+X],A",	"CMP X,#i",	"MOV !a,X",		"MOV1 m.b,C",	"MOV d,Y",		"MOV !a,Y",	"MOV X,#i",	"POP X",			"MUL YA",
	"BNE r",	"TCALL 13",	"CLR1 d.6",	"BBC d.6,q",	"MOV d+X,A",	"MOV !a+X,A",	"MOV !a+Y,A",	"MOV [d]+Y,A",	"MOV d,X",	"MOV d+Y,X",	"MOVW d,YA",	"MOV d+X,Y",	"DEC Y",		"MOV A,Y",	"CBNE d+X,q",	"DAA A",
	"CLRV",	"TCALL 14",	"SET1 d.7",	"BBS d.7,q",	"MOV A,d",		"MOV A,!a",		"MOV A,(X)",	"MOV A,[d+X]",	"MOV A,#i",	"MOV X,!a",		"NOT1 m.b",		"MOV Y,d",		"MOV Y,!a",	"NOTC",		"POP Y",			"SLEEP",
	"BEQ r",	"TCALL 15",	"CLR1 d.7",	"BBC d.7,q",	"MOV A,d+X",	"MOV A,!a+X",	"MOV A,!a+Y",	"MOV A,[d]+Y",	"MOV X,d",	"MOV X,d+Y",	"MOV t,s",		"MOV Y,d+X",	"INC Y",		"MOV Y,A",	"DBNZ Y,r",		"STOP"
};

constexpr const char* _altOpTemplate[256] = {
	"NOP",	"JST0",	"SET1 d.0",	"BBS d.0,q",	"ORA d",		"ORA a",		"ORA (X)",	"ORA [d,X]",	"ORA #i",	"OR t,s",		"ORC m.b",		"ASL d",		"ASL a",	"PHP",	"SET1 a",		"BRK",
	"BPL r",	"JST1",	"CLR1 d.0",	"BBC d.0,q",	"ORA d,X",	"ORA a,X",	"ORA a,Y",	"ORA [d],Y",	"OR e,#i",	"OR (X),(Y)",	"DEW d",			"ASL d,X",	"ASL A",	"DEX",	"CPX a",			"JMP [a,X]",
	"CLP",	"JST2",	"SET1 d.1",	"BBS d.1,q",	"AND d",		"AND a",		"AND (X)",	"AND [d,X]",	"AND #i",	"AND t,s",		"ORC /m.b",		"ROL d",		"ROL a",	"PHA",	"CBNE d,q",		"BRA r",
	"BMI r",	"JST3",	"CLR1 d.1",	"BBC d.1,q",	"AND d,X",	"AND a,X",	"AND a,Y",	"AND [d],Y",	"AND e,#i",	"AND (X),(Y)",	"INW d",			"ROL d,X",	"ROL A",	"INX",	"CPX d",			"JSR a",
	"SEP",	"JST4",	"SET1 d.2",	"BBS d.2,q",	"EOR d",		"EOR a",		"EOR (X)",	"EOR [d,X]",	"EOR #i",	"EOR t,s",		"ANDC m.b",		"LSR d",		"LSR a",	"PHX",	"CLR1 a",		"JSP u",
	"BVC r",	"JST5",	"CLR1 d.2",	"BBC d.2,q",	"EOR d,X",	"EOR a,X",	"EOR a,Y",	"EOR [d],Y",	"EOR e,#i",	"EOR (X),(Y)",	"CPW d",			"LSR d,X",	"LSR A",	"TAX",	"CPY a",			"JMP a",
	"CLC",	"JST6",	"SET1 d.3",	"BBS d.3,q",	"CMP d",		"CMP a",		"CMP (X)",	"CMP [d,X]",	"CMP #i",	"CMP t,s",		"ANDC /m.b",	"ROR d",		"ROR a",	"PHY",	"DBNZ d,q",		"RTS",
	"BVS r",	"JST7",	"CLR1 d.3",	"BBC d.3,q",	"CMP d,X",	"CMP a,X",	"CMP a,Y",	"CMP [d],Y",	"CMP e,#i",	"CMP (X),(Y)",	"ADW d",			"ROR d,X",	"ROR A",	"TXA",	"CPY d",			"RTI",
	"SEC",	"JST8",	"SET1 d.4",	"BBS d.4,q",	"ADC d",		"ADC a",		"ADC (X)",	"ADC [d,X]",	"ADC #i",	"ADC t,s",		"EORC m.b",		"DEC d",		"DEC a",	"LDY #i","PLP",			"MOV e,#i",
	"BCC r",	"JST9",	"CLR1 d.4",	"BBC d.4,q",	"ADC d,X",	"ADC a,X",	"ADC a,Y",	"ADC [d],Y",	"ADC e,#i",	"ADC (X),(Y)",	"SBW d",			"DEC d,X",	"DEC A",	"TSX",	"DIV YA,X",		"XCN A",
	"CLI",	"JSTA",	"SET1 d.5",	"BBS d.5,q",	"SBC d",		"SBC a",		"SBC (X)",	"SBC [d,X]",	"SBC #i",	"SBC t,s",		"LDC m.b",		"INC d",		"INC a",	"CPY #i","PLA",			"STA (X)+",
	"BCS r",	"JSTB",	"CLR1 d.5",	"BBC d.5,q",	"SBC d,X",	"SBC a,X",	"SBC a,Y",	"SBC [d],Y",	"SBC e,#i",	"SBC (X),(Y)",	"LDW d",			"INC d,X",	"INC A",	"TXS",	"DAS A",			"LDA (X)+",
	"SEI",	"JSTC",	"SET1 d.6",	"BBS d.6,q",	"STA d",		"STA a",		"STA (X)",	"STA [d,X]",	"CPX #i",	"STX a",			"STC m.b",		"STY d",		"STY a",	"LDX #i","PLX",			"MUL YA",
	"BNE r",	"JSTD",	"CLR1 d.6",	"BBC d.6,q",	"STA d,X",	"STA a,X",	"STA a,Y",	"STA [d],Y",	"STX d",		"STX d,Y",		"STW d",			"STY d,X",	"DEY",	"TYA",	"CBNE d,X, q",	"DAA A",
	"CLV",	"JSTE",	"SET1 d.7",	"BBS d.7,q",	"LDA d",		"LDA a",		"LDA (X)",	"LDA [d,X]",	"LDA #i",	"LDX a",			"NOT m.b",		"LDY d",		"LDY a",	"NOTC",	"PLY",			"WAI",
	"BEQ r",	"JSTF",	"CLR1 d.7",	"BBC d.7,q",	"LDA d,X",	"LDA a,X",	"LDA a,Y",	"LDA [d],Y",	"LDX d",		"LDX d,Y",		"MOV t,s",		"LDY d,X",	"INY",	"TAY",	"DBNZ Y,r",		"STP"
};

constexpr const uint8_t _opSize[256] = {
	1, 1, 2, 3, 2, 3, 1, 2, 2, 3, 3, 2, 3, 1, 3, 1,
	2, 1, 2, 3, 2, 3, 3, 2, 3, 1, 2, 2, 1, 1, 3, 3,
	1, 1, 2, 3, 2, 3, 1, 2, 2, 3, 3, 2, 3, 1, 3, 2,
	2, 1, 2, 3, 2, 3, 3, 2, 3, 1, 2, 2, 1, 1, 2, 3,
	1, 1, 2, 3, 2, 3, 1, 2, 2, 3, 3, 2, 3, 1, 3, 2,
	2, 1, 2, 3, 2, 3, 3, 2, 3, 1, 2, 2, 1, 1, 3, 3,
	1, 1, 2, 3, 2, 3, 1, 2, 2, 3, 3, 2, 3, 1, 3, 1,
	2, 1, 2, 3, 2, 3, 3, 2, 3, 1, 2, 2, 1, 1, 2, 1,
	1, 1, 2, 3, 2, 3, 1, 2, 2, 3, 3, 2, 3, 2, 1, 3,
	2, 1, 2, 3, 2, 3, 3, 2, 3, 1, 2, 2, 1, 1, 1, 1,
	1, 1, 2, 3, 2, 3, 1, 2, 2, 3, 3, 2, 3, 2, 1, 1,
	2, 1, 2, 3, 2, 3, 3, 2, 3, 1, 2, 2, 1, 1, 1, 1,
	1, 1, 2, 3, 2, 3, 1, 2, 2, 3, 3, 2, 3, 2, 1, 1,
	2, 1, 2, 3, 2, 3, 3, 2, 2, 2, 2, 2, 1, 1, 3, 1,
	1, 1, 2, 3, 2, 3, 1, 2, 2, 3, 3, 2, 3, 1, 1, 1,
	2, 1, 2, 3, 2, 3, 3, 2, 2, 2, 3, 2, 1, 1, 2, 1,
};

constexpr bool _needAddress[256] = {
	false, true, true, true, true, false, true, true, false, false, false, true, false, false, false, false,
	false, true, true, true, true, true,  true, true, true,  false, true,  true,  false, false, false, true,
	false, true, true, true, true, false, true, true, false, false, false, true, false, false, true, false,
	false, true, true, true, true, true,  true, true, true,  false, true,  true,  false, false, true, false,
	false, true, true, true, true, false, true, true, false, false, false, true, false, false, false, false,
	false, true, true, true, true, true,  true, true, true,  false, true,  true,  false, false, false, false,
	false, true, true, true, true, false, true, true, false, false, false, true, false, false, true, false,
	false, true, true, true, true, true,  true, true, true,  false, true,  true,  false, false, true, false,
	false, true, true, true, true, false, true, true, false, false, false, true, false, false, false, true,
	false, true, true, true, true, true,  true, true, true,  false, true,  true,  false, false, false, false,
	false, true, true, true, true, false, true, true, false, false, false, true, false, false, false, true,
	false, true, true, true, true, true,  true, true, true,  false, true,  true,  false, false, false, true,
	false, true, true, true, true, false, true, true, false, false, false, true, false, false, false, false,
	false, true, true, true, true, true,  true, true, true,  true,  true,  true,  false, false, true, false,
	false, true, true, true, true, false, true, true, false, false, false, true, false, false, false, false,
	false, true, true, true, true, true,  true, true, true,  true,  false, true,  false, false, false, false
};

constexpr bool _showValue[256] = {
	false, false, true, true, true, true, true, true, false, true, true, true, true,  false, true,  false,
	false, false, true, true, true, true, true, true, true,  true, true, true, false, false, true,  false,
	false, false, true, true, true, true, true, true, false, true, true, true, true,  false, true,  false,
	false, false, true, true, true, true, true, true, true,  true, true, true, false, false, true,  false,
	false, false, true, true, true, true, true, true, false, true, true, true, true,  false, true,  false,
	false, false, true, true, true, true, true, true, true,  true, true, true, false, false, true,  false,
	false, false, true, true, true, true, true, true, false, true, true, true, true,  false, true,  false,
	false, false, true, true, true, true, true, true, true,  true, true, true, false, false, true,  false,
	false, false, true, true, true, true, true, true, false, true, true, true, true,  false, false, true,
	false, false, true, true, true, true, true, true, true,  true, true, true, false, false, false, false,
	false, false, true, true, true, true, true, true, false, true, true, true, true,  false, false, true,
	false, false, true, true, true, true, true, true, true,  true, true, true, false, false, false, true,
	false, false, true, true, true, true, true, true, false, true, true, true, true,  false, false, false,
	false, false, true, true, true, true, true, true, true,  true, true, true, false, false, true,  false,
	false, false, true, true, true, true, true, true, false, true, true, true, true,  false, false, false,
	false, false, true, true, true, true, true, true, true,  true, true, true, false, false, true,  false
};

void SpcDisUtils::GetDisassembly(DisassemblyInfo &info, string &out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings)
{
	FastString str(settings->GetDebugConfig().UseLowerCaseDisassembly);

	AddressInfo addrInfo { 0, MemoryType::SpcMemory };
	auto getOperand = [&str, &addrInfo, labelManager](uint16_t addr) {
		addrInfo.Address = addr;
		string label = labelManager ? labelManager->GetLabel(addrInfo) : "";
		if(label.empty()) {
			str.WriteAll('$', HexUtilities::ToHex(addr));
		} else {
			str.Write(label, true);
		}
	};

	uint8_t* byteCode = info.GetByteCode();
	const char* op = settings->GetDebugConfig().SnesUseAltSpcOpNames ? _altOpTemplate[byteCode[0]] : _opTemplate[byteCode[0]];
	int i = 0;
	while(op[i]) {
		switch(op[i]) {
			case 'r': getOperand((uint16_t)(memoryAddr + (int8_t)byteCode[1] + GetOpSize(byteCode[0]))); break;
			case 'q': getOperand((uint16_t)(memoryAddr + (int8_t)byteCode[2] + GetOpSize(byteCode[0]))); break; //relative 2nd byte

			case 'a': getOperand((uint16_t)(byteCode[1] | (byteCode[2] << 8))); break;
			
			case 'd': str.WriteAll('$', HexUtilities::ToHex(byteCode[1])); break;
			case 'e': str.WriteAll('$', HexUtilities::ToHex(byteCode[2])); break; //direct 2nd byte

			case 's': str.WriteAll('$', HexUtilities::ToHex(byteCode[1])); break;
			case 't': str.WriteAll('$', HexUtilities::ToHex(byteCode[2])); break;

			case 'i': str.WriteAll('$', HexUtilities::ToHex(byteCode[1])); break;

			case 'm': getOperand((uint16_t)((byteCode[1] | (byteCode[2] << 8)) & 0x1FFF)); break;
			case 'b': str.WriteAll((char)('0' + (byteCode[2] >> 5))); break;

			default: str.Write(op[i]); break;
		}
		i++;
	}

	out += str.ToString();
}

EffectiveAddressInfo SpcDisUtils::GetEffectiveAddress(DisassemblyInfo &info, SnesConsole *console, SpcState &state)
{
	if(SpcDisUtils::IsConditionalJump(info.GetOpCode()) || SpcDisUtils::IsUnconditionalJump(info.GetOpCode())) {
		//Show nothing for jumps
		return {};
	} else if(!_showValue[info.GetOpCode()]) {
		return {};
	}

	Spc* spc = console->GetSpc();
	DummySpc dummySpc(spc->GetSpcRam());
	dummySpc.SetDummyState(state);
	dummySpc.Step();

	uint32_t count = dummySpc.GetOperationCount();
	for(int i = count - 1; i > 0; i--) {
		MemoryOperationInfo opInfo = dummySpc.GetOperationInfo(i);
		if(opInfo.Type != MemoryOperationType::ExecOperand && opInfo.Type != MemoryOperationType::DummyRead) {
			MemoryOperationInfo prevOpInfo = dummySpc.GetOperationInfo(i - 1);
			EffectiveAddressInfo result;
			if(prevOpInfo.Type == opInfo.Type && prevOpInfo.Address == opInfo.Address - 1) {
				//For 16-bit read/writes, return the first address
				result.Address = prevOpInfo.Address;
				result.Type = prevOpInfo.MemType;
				result.ValueSize = 2;
			} else {
				result.Address = opInfo.Address;
				result.Type = opInfo.MemType;
				result.ValueSize = 1;
			}
			result.ShowAddress = _needAddress[info.GetOpCode()];
			return result;
		}
	}

	return {};
}

uint8_t SpcDisUtils::GetOpSize(uint8_t opCode)
{
	return _opSize[opCode];
}

bool SpcDisUtils::IsUnconditionalJump(uint8_t opCode)
{
	switch(opCode) {
		case 0x01: //TCALL0
		case 0x0F: //BRK
		case 0x11: //TCALL1
		case 0x1F: //JMP
		case 0x21: //TCALL2
		case 0x2F: //BRA
		case 0x31: //TCALL3
		case 0x3F: //CALL
		case 0x41: //TCALL4
		case 0x4F: //PCALL
		case 0x51: //TCALL5
		case 0x5F: //JMP
		case 0x61: //TCALL6
		case 0x6F: //RET
		case 0x71: //TCALL7
		case 0x7F: //RETI
		case 0x81: //TCALL8
		case 0x91: //TCALL9
		case 0xA1: //TCALLA
		case 0xB1: //TCALLB
		case 0xC1: //TCALLC
		case 0xD1: //TCALLD
		case 0xE1: //TCALLE
		case 0xF1: //TCALLF
			return true;

		default:
			return false;
	}
}

bool SpcDisUtils::IsConditionalJump(uint8_t opCode)
{
	switch(opCode) {
		case 0x03: //BBS0
		case 0x10: //BPL
		case 0x13: //BBC0
		case 0x23: //BBS1
		case 0x2E: //CBNE
		case 0x30: //BMI
		case 0x33: //BBC1
		case 0x43: //BBS2
		case 0x50: //BVC
		case 0x53: //BBC2
		case 0x63: //BBS3
		case 0x6E: //DBNZ
		case 0x70: //BVS
		case 0x73: //BBC3
		case 0x83: //BBS4
		case 0x90: //BCC
		case 0x93: //BBC4
		case 0xA3: //BBS5
		case 0xB0: //BCS
		case 0xB3: //BBC5
		case 0xC3: //BBS6
		case 0xD0: //BNE
		case 0xD3: //BBC6
		case 0xDE: //CBNE
		case 0xE3: //BBS7
		case 0xF0: //BEQ
		case 0xF3: //BBC7
		case 0xFE: //DBNZ
			return true;

		default:
			return false;
	}
}

bool SpcDisUtils::IsJumpToSub(uint8_t opCode)
{
	return opCode == 0x3F || opCode == 0x0F || opCode == 0x4F || (opCode&0x0F) == 0x01; //JSR, BRK, PCALL, TCALL
}

bool SpcDisUtils::IsReturnInstruction(uint8_t opCode)
{
	return opCode == 0x6F || opCode == 0x7F;
}
#include "pch.h"
#include "SMS/Debugger/SmsDisUtils.h"
#include "SMS/SmsTypes.h"
#include "SMS/Debugger/DummySmsCpu.h"
#include "SMS/SmsConsole.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/LabelManager.h"
#include "Debugger/MemoryDumper.h"
#include "Shared/MemoryType.h"
#include "Shared/EmuSettings.h"
#include "Utilities/FastString.h"
#include "Utilities/HexUtilities.h"

constexpr const char* _opTemplate[256] = {
	"NOP",		"LD BC, e",	"LD (BC), A",	"INC BC",	"INC B",			"DEC B",		"LD B, d",	"RLCA",		"EX AF,AF'",	"ADD w, BC",	"LD A, (BC)",	"DEC BC",	"INC C",		"DEC C",		"LD C, d",	"RRCA",
	"DJNZ r",	"LD DE, e",	"LD (DE), A",	"INC DE",	"INC D",			"DEC D",		"LD D, d",	"RLA",		"JR r",			"ADD w, DE",	"LD A, (DE)",	"DEC DE",	"INC E",		"DEC E",		"LD E, d",	"RRA",
	"JR NZ, r",	"LD w, e",	"LD (a), w",	"INC w",		"INC y",			"DEC y",		"LD y, d",	"DAA",		"JR Z, r",		"ADD w, w",		"LD w, (a)",	"DEC w",		"INC z",		"DEC z",		"LD z, d",	"CPL",
	"JR NC, r",	"LD SP, e",	"LD (a), A",	"INC SP",	"INC x",			"DEC x",		"LD x, d",	"SCF",		"JR C, r",		"ADD w, SP",	"LD A, (a)",	"DEC SP",	"INC A",		"DEC A",		"LD A, d",	"CCF",
	"LD B, B",	"LD B, C",	"LD B, D",		"LD B, E",	"LD B, y",		"LD B, z",	"LD B, x",	"LD B, A",	"LD C, B",		"LD C, C",		"LD C, D",		"LD C, E",	"LD C, y",	"LD C, z",	"LD C, x",	"LD C, A",
	"LD D, B",	"LD D, C",	"LD D, D",		"LD D, E",	"LD D, y",		"LD D, z",	"LD D, x",	"LD D, A",	"LD E, B",		"LD E, C",		"LD E, D",		"LD E, E",	"LD E, y",	"LD E, z",	"LD E, x",	"LD E, A",
	"LD y, B",	"LD y, C",	"LD y, D",		"LD y, E",	"LD y, y",		"LD y, z",	"LD H, x",	"LD y, A",	"LD z, B",		"LD z, C",		"LD z, D",		"LD z, E",	"LD z, y",	"LD z, z",	"LD L, x",	"LD z, A",
	"LD x, B",	"LD x, C",	"LD x, D",		"LD x, E",	"LD x, H",		"LD x, L",	"HALT",		"LD x, A",	"LD A, B",		"LD A, C",		"LD A, D",		"LD A, E",	"LD A, y",	"LD A, z",	"LD A, x",	"LD A, A",
	"ADD A, B",	"ADD A, C",	"ADD A, D",		"ADD A, E",	"ADD A, y",		"ADD A, z",	"ADD A, x",	"ADD A, A",	"ADC A, B",		"ADC A, C",		"ADC A, D",		"ADC A, E",	"ADC A, y",	"ADC A, z",	"ADC A, x",	"ADC A, A",
	"SUB B",		"SUB C",		"SUB D",			"SUB E",		"SUB y",			"SUB z",		"SUB x",		"SUB A",		"SBC A, B",		"SBC A, C",		"SBC A, D",		"SBC A, E",	"SBC A, y",	"SBC A, z",	"SBC A, x",	"SBC A, A",
	"AND B",		"AND C",		"AND D",			"AND E",		"AND y",			"AND z",		"AND x",		"AND A",		"XOR B",			"XOR C",			"XOR D",			"XOR E",		"XOR y",		"XOR z",		"XOR x",		"XOR A",
	"OR B",		"OR C",		"OR D",			"OR E",		"OR y",			"OR z",		"OR x",		"OR A",		"CP B",			"CP C",			"CP D",			"CP E",		"CP y",		"CP z",		"CP x",		"CP A",
	"RET NZ",	"POP BC",	"JP NZ, a",		"JP a",		"CALL NZ, a",	"PUSH BC",	"ADD A, d",	"RST 00H",	"RET Z",			"RET",			"JP Z, a",		"PREFIX_CB","CALL Z, a","CALL a",	"ADC A, d",	"RST 08H",
	"RET NC",	"POP DE",	"JP NC, a",		"OUT (p), A","CALL NC, a",	"PUSH DE",	"SUB d",		"RST 10H",	"RET C",			"EXX",			"JP C, a",		"IN A, (p)","CALL C, a","PREFIX_DD","SBC A, d",	"RST 18H",
	"RET PO",	"POP w",		"JP PO, a",		"EX (SP), w","CALL PO, a",	"PUSH w",	"AND d",		"RST 20H",	"RET PE",		"JP w",			"JP PE, a",		"EX DE, w",	"CALL PE, a","PREFIX_ED","XOR d",	"RST 28H",
	"RET P",		"POP AF",	"JP P, a",		"DI",			"CALL P, a",	"PUSH AF",	"OR d",		"RST 30H",	"RET M",			"LD SP, w",		"JP M, a",		"EI",			"CALL M, a","PREFIX_FD","CP d",		"RST 38H"
};

constexpr const char* _cbTemplate[256] = {
	"RLC vB",		"RLC vC",		"RLC vD",		"RLC vE",		"RLC vH",		"RLC vL",		"RLC x",		"RLC vA",		"RRC vB",		"RRC vC",		"RRC vD",		"RRC vE",		"RRC vH",		"RRC vL",		"RRC x",		"RRC vA",	
	"RL vB",			"RL vC",			"RL vD",			"RL vE",			"RL vH",			"RL vL",			"RL x",		"RL vA",			"RR vB",			"RR vC",			"RR vD",			"RR vE",			"RR vH",			"RR vL",			"RR x",		"RR vA",	
	"SLA vB",		"SLA vC",		"SLA vD",		"SLA vE",		"SLA vH",		"SLA vL",		"SLA x",		"SLA vA",		"SRA vB",		"SRA vC",		"SRA vD",		"SRA vE",		"SRA vH",		"SRA vL",		"SRA x",		"SRA vA",	
	"SLL vB",		"SLL vC",		"SLL vD",		"SLL vE",		"SLL vH",		"SLL vL",		"SLL x",		"SWAP A",		"SRL vB",		"SRL vC",		"SRL vD",		"SRL vE",		"SRL vH",		"SRL vL",		"SRL x",		"SRL vA",	
	"BIT 0, vB",	"BIT 0, vC",	"BIT 0, vD",	"BIT 0, vE",	"BIT 0, vH",	"BIT 0, vL",	"BIT 0, x",	"BIT 0, vA",	"BIT 1, vB",	"BIT 1, vC",	"BIT 1, vD",	"BIT 1, vE",	"BIT 1, vH",	"BIT 1, vL",	"BIT 1, x",	"BIT 1, vA",	
	"BIT 2, vB",	"BIT 2, vC",	"BIT 2, vD",	"BIT 2, vE",	"BIT 2, vH",	"BIT 2, vL",	"BIT 2, x",	"BIT 2, vA",	"BIT 3, vB",	"BIT 3, vC",	"BIT 3, vD",	"BIT 3, vE",	"BIT 3, vH",	"BIT 3, vL",	"BIT 3, x",	"BIT 3, vA",	
	"BIT 4, vB",	"BIT 4, vC",	"BIT 4, vD",	"BIT 4, vE",	"BIT 4, vH",	"BIT 4, vL",	"BIT 4, x",	"BIT 4, vA",	"BIT 5, vB",	"BIT 5, vC",	"BIT 5, vD",	"BIT 5, vE",	"BIT 5, vH",	"BIT 5, vL",	"BIT 5, x",	"BIT 5, vA",	
	"BIT 6, vB",	"BIT 6, vC",	"BIT 6, vD",	"BIT 6, vE",	"BIT 6, vH",	"BIT 6, vL",	"BIT 6, x",	"BIT 6, vA",	"BIT 7, vB",	"BIT 7, vC",	"BIT 7, vD",	"BIT 7, vE",	"BIT 7, vH",	"BIT 7, vL",	"BIT 7, x",	"BIT 7, vA",	
	"RES 0, vB",	"RES 0, vC",	"RES 0, vD",	"RES 0, vE",	"RES 0, vH",	"RES 0, vL",	"RES 0, x",	"RES 0, vA",	"RES 1, vB",	"RES 1, vC",	"RES 1, vD",	"RES 1, vE",	"RES 1, vH",	"RES 1, vL",	"RES 1, x",	"RES 1, vA",	
	"RES 2, vB",	"RES 2, vC",	"RES 2, vD",	"RES 2, vE",	"RES 2, vH",	"RES 2, vL",	"RES 2, x",	"RES 2, vA",	"RES 3, vB",	"RES 3, vC",	"RES 3, vD",	"RES 3, vE",	"RES 3, vH",	"RES 3, vL",	"RES 3, x",	"RES 3, vA",	
	"RES 4, vB",	"RES 4, vC",	"RES 4, vD",	"RES 4, vE",	"RES 4, vH",	"RES 4, vL",	"RES 4, x",	"RES 4, vA",	"RES 5, vB",	"RES 5, vC",	"RES 5, vD",	"RES 5, vE",	"RES 5, vH",	"RES 5, vL",	"RES 5, x",	"RES 5, vA",	
	"RES 6, vB",	"RES 6, vC",	"RES 6, vD",	"RES 6, vE",	"RES 6, vH",	"RES 6, vL",	"RES 6, x",	"RES 6, vA",	"RES 7, vB",	"RES 7, vC",	"RES 7, vD",	"RES 7, vE",	"RES 7, vH",	"RES 7, vL",	"RES 7, x",	"RES 7, vA",	
	"SET 0, vB",	"SET 0, vC",	"SET 0, vD",	"SET 0, vE",	"SET 0, vH",	"SET 0, vL",	"SET 0, x",	"SET 0, vA",	"SET 1, vB",	"SET 1, vC",	"SET 1, vD",	"SET 1, vE",	"SET 1, vH",	"SET 1, vL",	"SET 1, x",	"SET 1, vA",	
	"SET 2, vB",	"SET 2, vC",	"SET 2, vD",	"SET 2, vE",	"SET 2, vH",	"SET 2, vL",	"SET 2, x",	"SET 2, vA",	"SET 3, vB",	"SET 3, vC",	"SET 3, vD",	"SET 3, vE",	"SET 3, vH",	"SET 3, vL",	"SET 3, x",	"SET 3, vA",	
	"SET 4, vB",	"SET 4, vC",	"SET 4, vD",	"SET 4, vE",	"SET 4, vH",	"SET 4, vL",	"SET 4, x",	"SET 4, vA",	"SET 5, vB",	"SET 5, vC",	"SET 5, vD",	"SET 5, vE",	"SET 5, vH",	"SET 5, vL",	"SET 5, x",	"SET 5, vA",	
	"SET 6, vB",	"SET 6, vC",	"SET 6, vD",	"SET 6, vE",	"SET 6, vH",	"SET 6, vL",	"SET 6, x",	"SET 6, vA",	"SET 7, vB",	"SET 7, vC",	"SET 7, vD",	"SET 7, vE",	"SET 7, vH",	"SET 7, vL",	"SET 7, x",	"SET 7, vA",	
};

constexpr const char* _edTemplate[256] = {
	"NOP",			"NOP",			"NOP",			"NOP",			"NOP",	"NOP",	"NOP",	"NOP",		"NOP",			"NOP",			"NOP",			"NOP",			"NOP",	"NOP",	"NOP",	"NOP",
	"NOP",			"NOP",			"NOP",			"NOP",			"NOP",	"NOP",	"NOP",	"NOP",		"NOP",			"NOP",			"NOP",			"NOP",			"NOP",	"NOP",	"NOP",	"NOP",
	"NOP",			"NOP",			"NOP",			"NOP",			"NOP",	"NOP",	"NOP",	"NOP",		"NOP",			"NOP",			"NOP",			"NOP",			"NOP",	"NOP",	"NOP",	"NOP",
	"NOP",			"NOP",			"NOP",			"NOP",			"NOP",	"NOP",	"NOP",	"NOP",		"NOP",			"NOP",			"NOP",			"NOP",			"NOP",	"NOP",	"NOP",	"NOP",

	"IN B, (C)",	"OUT (C), B",	"SBC HL, BC",	"LD (a), BC",	"NEG",	"RETN",	"IM 0",	"LD I, A",	"IN C, (C)",	"OUT (C), C",	"ADC HL, BC",	"LD BC, (a)",	"NEG",	"RETI",	"IM 0",	"LD R, A",
	"IN D, (C)",	"OUT (C), D",	"SBC HL, DE",	"LD (a), DE",	"NEG",	"RETI",	"IM 1",	"LD A, I",	"IN E, (C)",	"OUT (C), E",	"ADC HL, DE",	"LD DE, (a)",	"NEG",	"RETI",	"IM 2",	"LD A, R",
	"IN H, (C)",	"OUT (C), H",	"SBC HL, HL",	"LD (a), HL",	"NEG",	"RETI",	"IM 1",	"RRD",		"IN L, (C)",	"OUT (C), L",	"ADC HL, HL",	"LD HL, (a)",	"NEG",	"RETI",	"IM 2",	"LD A, R",
	"IN (C)",		"OUT (C), 0",	"SBC HL, SP",	"LD (a), SP",	"NEG",	"RETI",	"IM 0",	"NOP",		"IN A, (C)",	"OUT (C), A",	"ADC HL, SP",	"LD SP, (a)",	"NEG",	"RETI",	"IM 2",	"NOP",

	"NOP",			"NOP",			"NOP",			"NOP",			"NOP",	"NOP",	"NOP",	"NOP",		"NOP",			"NOP",			"NOP",			"NOP",			"NOP",	"NOP",	"NOP",	"NOP",
	"NOP",			"NOP",			"NOP",			"NOP",			"NOP",	"NOP",	"NOP",	"NOP",		"NOP",			"NOP",			"NOP",			"NOP",			"NOP",	"NOP",	"NOP",	"NOP",
	"LDI",			"CPI",			"INI",			"OUTI",			"NOP",	"NOP",	"NOP",	"NOP",		"LDD",			"CPD",			"IND",			"OUTD",			"NOP",	"NOP",	"NOP",	"NOP",
	"LDIR",			"CPIR",			"INIR",			"OTIR",			"NOP",	"NOP",	"NOP",	"NOP",		"LDDR",			"CPDR",			"INDR",			"OTDR",			"NOP",	"NOP",	"NOP",	"NOP",

	"NOP",			"NOP",			"NOP",			"NOP",			"NOP",	"NOP",	"NOP",	"NOP",		"NOP",			"NOP",			"NOP",			"NOP",			"NOP",	"NOP",	"NOP",	"NOP",
	"NOP",			"NOP",			"NOP",			"NOP",			"NOP",	"NOP",	"NOP",	"NOP",		"NOP",			"NOP",			"NOP",			"NOP",			"NOP",	"NOP",	"NOP",	"NOP",
	"NOP",			"NOP",			"NOP",			"NOP",			"NOP",	"NOP",	"NOP",	"NOP",		"NOP",			"NOP",			"NOP",			"NOP",			"NOP",	"NOP",	"NOP",	"NOP",
	"NOP",			"NOP",			"NOP",			"NOP",			"NOP",	"NOP",	"NOP",	"NOP",		"NOP",			"NOP",			"NOP",			"NOP",			"NOP",	"NOP",	"NOP",	"NOP",
};

constexpr const uint8_t _opSize[256] = {
	1,3,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	2,3,1,1,1,1,2,1,2,1,1,1,1,1,2,1,
	2,3,3,1,1,1,2,1,2,1,3,1,1,1,2,1,
	2,3,3,1,1,1,2,1,2,1,3,1,1,1,2,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,3,3,3,1,2,1,1,1,3,2,3,3,2,1,
	1,1,3,2,3,1,2,1,1,1,3,2,3,1,2,1,
	1,1,3,1,3,1,2,1,1,1,3,1,3,1,2,1,
	1,1,3,1,3,1,2,1,1,1,3,1,3,1,2,1,
};

constexpr const uint8_t _opSizeIxIy[256] = {
	1,3,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	2,3,1,1,1,1,2,1,2,1,1,1,1,1,2,1,
	2,3,3,1,1,1,2,1,2,1,3,1,1,1,2,1,
	2,3,3,1,2,2,3,1,2,1,3,1,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	2,2,2,2,2,2,1,2,1,1,1,1,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	1,1,3,3,3,1,2,1,1,1,3,2,3,3,2,1,
	1,1,3,2,3,1,2,1,1,1,3,2,3,1,2,1,
	1,1,3,1,3,1,2,1,1,1,3,1,3,1,2,1,
	1,1,3,1,3,1,2,1,1,1,3,1,3,1,2,1,
};

constexpr const uint8_t _opSizeEd[256] = {
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,

	2,2,2,4,2,2,2,2,2,2,2,4,2,2,2,2,
	2,2,2,4,2,2,2,2,2,2,2,4,2,2,2,2,
	2,2,2,4,2,2,2,2,2,2,2,4,2,2,2,2,
	2,2,2,4,2,2,2,2,2,2,2,4,2,2,2,2,

	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,

	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
};

void SmsDisUtils::GetDisassembly(DisassemblyInfo& info, string& out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings)
{
	FastString str(settings->GetDebugConfig().UseLowerCaseDisassembly);

	AddressInfo addrInfo { 0, MemoryType::SmsMemory };
	auto getOperand = [&str, &addrInfo, labelManager](uint16_t addr) {
		addrInfo.Address = addr;
		string label = labelManager ? labelManager->GetLabel(addrInfo) :"";
		if(label.empty()) {
			str.WriteAll('$', HexUtilities::ToHex(addr));
		} else {
			str.Write(label, true);
		}
	};

	SmsOpInfo opInfo = GetSmsOpInfo(info);
	if(!opInfo.Op) {
		return;
	}

	int i = 0;
	while(opInfo.Op[i]) {
		switch(opInfo.Op[i]) {
			//Relative jumps
			case 'r': getOperand((uint16_t)(memoryAddr + (int8_t)opInfo.ByteCode[1] + _opSize[opInfo.ByteCode[0]])); break;

			//Jump addresses, memory addresses
			case 'a': getOperand((uint16_t)(opInfo.ByteCode[1] | (opInfo.ByteCode[2] << 8))); break;

			//Immediate values
			case 'd': str.WriteAll("$", HexUtilities::ToHex(opInfo.ByteCode[1])); break;
			case 'e': str.WriteAll("$", HexUtilities::ToHex((uint16_t)(opInfo.ByteCode[1] | (opInfo.ByteCode[2] << 8)))); break;
			
			case 'p': {
				AddressInfo portAddr = { opInfo.ByteCode[1], MemoryType::SmsPort };
				string label = labelManager ? labelManager->GetLabel(portAddr) : "";
				if(label.empty()) {
					str.WriteAll('$', HexUtilities::ToHex((uint8_t)portAddr.Address));
				} else {
					str.Write(label, true);
				}
				break;
			}

			case 'v':
				switch(opInfo.HlType) {
					default: str.Write(""); break;
					case HlRegType::IX: str.WriteAll("(IX+$", HexUtilities::ToHex(opInfo.IndexOffset), "), "); break;
					case HlRegType::IY: str.WriteAll("(IY+$", HexUtilities::ToHex(opInfo.IndexOffset), "), "); break;
				}
				break;

			case 'w': 
				switch(opInfo.HlType) {
					default: str.Write("HL"); break;
					case HlRegType::IX: str.Write("IX"); break;
					case HlRegType::IY: str.Write("IY"); break;
				}
				break;

			case 'x':
				if(opInfo.IndexOffset < 0 && opInfo.HlType != HlRegType::HL) {
					opInfo.ByteCode++;
					opInfo.IndexOffset = opInfo.ByteCode[0];
				}

				switch(opInfo.HlType) {
					default: str.Write("(HL)"); break;
					case HlRegType::IX: str.WriteAll("(IX+$", HexUtilities::ToHex(opInfo.IndexOffset), ")"); break;
					case HlRegType::IY: str.WriteAll("(IY+$", HexUtilities::ToHex(opInfo.IndexOffset), ")"); break;
				}
				break;

			case 'y':
				switch(opInfo.HlType) {
					default: str.Write('H'); break;
					case HlRegType::IX: str.Write("IXH"); break;
					case HlRegType::IY: str.Write("IYH"); break;
				}
				break;

			case 'z':
				switch(opInfo.HlType) {
					default: str.Write('L'); break;
					case HlRegType::IX: str.Write("IXL"); break;
					case HlRegType::IY: str.Write("IYL"); break;
				}
				break;

			default: str.Write(opInfo.Op[i]); break;
		}
		i++;
	}

	out += str.ToString();
}

SmsOpInfo SmsDisUtils::GetSmsOpInfo(DisassemblyInfo& info)
{
	SmsOpInfo result = {};
	result.ByteCode = info.GetByteCode();
	uint8_t opSize = info.GetOpSize();
	
	for(int i = 0; i < opSize && !result.Op; i++) {
		switch(result.ByteCode[0]) {
			default:
				result.Op = _opTemplate[result.ByteCode[0]];
				break;

			case 0xCB:
				if(result.HlType != HlRegType::HL) {
					result.ByteCode++;
					result.IndexOffset = result.ByteCode[0];
				}
				result.ByteCode++;
				result.IsCbPrefix = true;
				result.Op = _cbTemplate[result.ByteCode[0]];
				break;

			case 0xDD:
				result.HlType = HlRegType::IX;
				result.ByteCode++;
				break;

			case 0xFD:
				result.HlType = HlRegType::IY;
				result.ByteCode++;
				break;

			case 0xED:
				result.ByteCode++;
				result.IsEdPrefix = true;
				result.HlType = HlRegType::HL;
				result.Op = _edTemplate[result.ByteCode[0]];
				break;
		}
	}

	return result;
}

EffectiveAddressInfo SmsDisUtils::GetEffectiveAddress(DisassemblyInfo& info, SmsConsole* console, SmsCpuState& state)
{
	bool isJump = SmsDisUtils::IsUnconditionalJump(info.GetOpCode()) || SmsDisUtils::IsConditionalJump(info.GetOpCode());
	if(isJump) {
		//For jumps, show no address/value
		return {};
	}

	SmsOpInfo smsOp = GetSmsOpInfo(info);
	if(smsOp.IsEdPrefix && smsOp.ByteCode[0] >= 0xB0 && smsOp.ByteCode[0] <= 0xBF) {
		//Don't show address/value for instructions that repeat (LDIR, etc.)
		return {};
	} else if(!smsOp.IsCbPrefix && !smsOp.IsEdPrefix) {
		switch(smsOp.ByteCode[0]) {
			case 0xC1: case 0xD1: case 0xE1: case 0xF1:
			case 0xC5: case 0xD5: case 0xE5: case 0xF5:
				//Don't show address/value for push/pop
				return {};
		}
	}

	DummySmsCpu dummyCpu;
	dummyCpu.Init(console->GetEmulator(), console, console->GetMemoryManager());
	dummyCpu.SetDummyState(state);
	dummyCpu.Exec();

	uint32_t count = dummyCpu.GetOperationCount();
	for(int i = count - 1; i > 0; i--) {
		MemoryOperationInfo opInfo = dummyCpu.GetOperationInfo(i);
		if(opInfo.Type != MemoryOperationType::ExecOperand && opInfo.Type != MemoryOperationType::DummyRead) {
			MemoryOperationInfo prevOpInfo = dummyCpu.GetOperationInfo(i - 1);
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

			result.ShowAddress = (
				(strstr(smsOp.Op, "(") != nullptr && strstr(smsOp.Op, "(a)") == nullptr && strstr(smsOp.Op, "(p)") == nullptr) ||
				strstr(smsOp.Op, "x") != nullptr ||
				(smsOp.HlType != HlRegType::HL && strstr(smsOp.Op, "v") != nullptr) ||
				(smsOp.IsEdPrefix && smsOp.ByteCode[0] >= 0xA0 && smsOp.ByteCode[0] <= 0xAF)
			);
			return result;
		}
	}
	
	return {};
}

uint8_t SmsDisUtils::GetOpSize(uint8_t opCode, uint32_t cpuAddress, MemoryType memType, MemoryDumper* memoryDumper)
{
	uint8_t prefixSize = 0;
	HlRegType hlRegType = HlRegType::HL;
	while(true) {
		switch(opCode) {
			default: return prefixSize + (hlRegType == HlRegType::HL ? _opSize[opCode] : _opSizeIxIy[opCode]);
	
			case 0xCB: return prefixSize + (hlRegType == HlRegType::HL ? 2 : 3);

			case 0xDD:
			case 0xFD:
				hlRegType = opCode == 0xDD ? HlRegType::IX : HlRegType::IY;
				prefixSize++;
				if(prefixSize > 6) {
					return 6;
				}
				opCode = memoryDumper->GetMemoryValue(memType, cpuAddress + prefixSize);
				break;

			case 0xED:
				opCode = memoryDumper->GetMemoryValue(memType, cpuAddress + prefixSize + 1);
				return prefixSize + _opSizeEd[opCode];
		}
	}
}

bool SmsDisUtils::IsJumpToSub(uint8_t opCode)
{
	switch(opCode) {
		case 0xC4: //CALL NZ,a16
		case 0xC7: //RST 00H
		case 0xCD: //CALL a16
		case 0xCC: //CALL Z,a16
		case 0xCF: //RST 08H
		case 0xD4: //CALL NC,a16
		case 0xD7: //RST 10H
		case 0xDC: //CALL C,a16
		case 0xDF: //RST 18H
		case 0xE4: //CALL PO,a16
		case 0xE7: //RST 20H
		case 0xEC: //CALL PE,a16
		case 0xEF: //RST 28H
		case 0xF4: //CALL P,a16
		case 0xF7: //RST 30H
		case 0xFC: //CALL M,a16
		case 0xFF: //RST 38H
			return true;

		default:
			return false;
	}
}

bool SmsDisUtils::IsReturnInstruction(uint16_t opCode)
{
	switch(opCode & 0xFF) {
		case 0xC0: //RET NZ
		case 0xC8: //RET Z
		case 0xC9: //RET
		case 0xD0: //RET NC
		case 0xD8: //RET C
		case 0xE0: //RET PO
		case 0xE8: //RET PE
		case 0xF0: //RET P
		case 0xF8: //RET M
			return true;

		case 0xED:
			return ((opCode >> 8) & 0xC7) == 0x45; //RETI/RETN

		default:
			return false;
	}
}

bool SmsDisUtils::IsUnconditionalJump(uint8_t opCode)
{
	switch(opCode) {
		case 0x18: //JR r8
		case 0xC3: //JP a16
		case 0xC7: //RST 00H
		case 0xC9: //RET
		case 0xCD: //CALL a16
		case 0xCF: //RST 08H
		case 0xD7: //RST 10H
		case 0xDF: //RST 18H
		case 0xE7: //RST 20H
		case 0xE9: //JP (HL)
		case 0xEF: //RST 28H
		case 0xF7: //RST 30H
		case 0xFF: //RST 38H
			return true;

		default:
			return false;
	}
}

bool SmsDisUtils::IsConditionalJump(uint8_t opCode)
{
	switch(opCode) {
		case 0x20: //JR NZ,r8
		case 0x28: //JR Z,r8
		case 0x30: //JR NC,r8
		case 0x38: //JR C,r8
		case 0xC0: //RET NZ
		case 0xC2: //JP NZ,a16
		case 0xC4: //CALL NZ,a16
		case 0xC8: //RET Z
		case 0xCA: //JP Z,a16
		case 0xCC: //CALL Z,a16
		case 0xD0: //RET NC
		case 0xD2: //JP NC,a16
		case 0xD4: //CALL NC,a16
		case 0xD8: //RET C
		case 0xDA: //JP C,a16
		case 0xDC: //CALL C,a16
		case 0xE0: //RET PO
		case 0xE2: //JP PO,a16
		case 0xE4: //CALL PO,a16
		case 0xE8: //RET PE
		case 0xEA: //JP PE,a16
		case 0xEC: //CALL PE,a16
		case 0xF0: //RET P
		case 0xF2: //JP P,a16
		case 0xF4: //CALL P,a16
		case 0xF8: //RET M
		case 0xFA: //JP M,a16
		case 0xFC: //CALL M,a16
			return true;

		default:
			return false;
	}
}


CdlFlags::CdlFlags SmsDisUtils::GetOpFlags(uint8_t opCode, uint16_t pc, uint16_t prevPc)
{
	switch(opCode) {
		case 0xC4: //CALL NZ,a16
		case 0xC7: //RST 00H
		case 0xCD: //CALL a16
		case 0xCC: //CALL Z,a16
		case 0xCF: //RST 08H
		case 0xD4: //CALL NC,a16
		case 0xD7: //RST 10H
		case 0xDC: //CALL C,a16
		case 0xDF: //RST 18H
		case 0xE4: //CALL PO,a16
		case 0xE7: //RST 20H
		case 0xEC: //CALL PE,a16
		case 0xEF: //RST 28H
		case 0xF4: //CALL P,a16
		case 0xF7: //RST 30H
		case 0xFC: //CALL M,a16
		case 0xFF: //RST 38H
			return (pc != prevPc + _opSize[opCode]) ? CdlFlags::SubEntryPoint : CdlFlags::None;

		case 0x18: //JR r8
		case 0xC3: //JP a16
		case 0xE9: //JP (HL)
		case 0x20: //JR NZ,r8
		case 0x28: //JR Z,r8
		case 0x30: //JR NC,r8
		case 0x38: //JR C,r8
		case 0xC2: //JP NZ,a16
		case 0xCA: //JP Z,a16
		case 0xD2: //JP NC,a16
		case 0xDA: //JP C,a16
		case 0xE2: //JP PO,a16
		case 0xEA: //JP PE,a16
		case 0xF2: //JP P,a16
		case 0xFA: //JP M,a16
			return (pc != prevPc + _opSize[opCode]) ? CdlFlags::JumpTarget : CdlFlags::None;

		default:
			return CdlFlags::None;
	}
}

string SmsDisUtils::GetOpTemplate(uint8_t op, bool cbPrefix, bool edPrefix)
{
	return cbPrefix ? _cbTemplate[op] : (edPrefix ? _edTemplate[op] : _opTemplate[op]);
}

#include "pch.h"
#include "WS/Debugger/WsDisUtils.h"
#include "WS/Debugger/DummyWsCpu.h"
#include "WS/WsTypes.h"
#include "WS/WsMemoryManager.h"
#include "WS/WsConsole.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/LabelManager.h"
#include "Debugger/MemoryDumper.h"
#include "Shared/MemoryType.h"
#include "Shared/EmuSettings.h"
#include "Utilities/FastString.h"
#include "Utilities/HexUtilities.h"

static constexpr const char* _modRegLut8[8] = { "AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH" };
static constexpr const char* _modRegLut16[8] = { "AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI" };
static constexpr const char* _modSegRegLut[4] = { "ES", "CS", "SS", "DS" };

constexpr const char* _opTemplate[256] = {
	//0-0F
	"ADD m", "ADD m", "ADD m", "ADD m", "ADD AL, i", "ADD AX, j", "PUSH ES", "POP ES",
	"OR m",  "OR m",  "OR m",  "OR m",  "OR AL, i",  "OR AX, j",  "PUSH CS", "UNDEFINED_0F",
	
	//10-1F
	"ADC m", "ADC m", "ADC m", "ADC m", "ADC AL, i", "ADC AX, j", "PUSH SS", "POP SS",
	"SBB m", "SBB m", "SBB m", "SBB m", "SBB AL, i", "SBB AX, j", "PUSH DS", "POP DS",

	//20-2F
	"AND m", "AND m", "AND m", "AND m", "AND AL, i", "AND AX, j", "ES:", "DAA",
	"SUB m", "SUB m", "SUB m", "SUB m", "SUB AL, i", "SUB AX, j", "CS:", "DAS",

	//30-3F
	"XOR m", "XOR m", "XOR m", "XOR m", "XOR AL, i", "XOR AX, j", "SS:", "AAA",
	"CMP m", "CMP m", "CMP m", "CMP m", "CMP AL, i", "CMP AX, j", "DS:", "AAS",

	//40-4F
	"INC AX", "INC CX", "INC DX", "INC BX", "INC SP", "INC BP", "INC SI", "INC DI",
	"DEC AX", "DEC CX", "DEC DX", "DEC BX", "DEC SP", "DEC BP", "DEC SI", "DEC DI",

	//50-5F
	"PUSH AX", "PUSH CX", "PUSH DX", "PUSH BX", "PUSH SP", "PUSH BP", "PUSH SI", "PUSH DI",
	"POP AX", "POP CX", "POP DX", "POP BX", "POP SP", "POP BP", "POP SI", "POP DI",

	//60-6F
	"PUSHA", "POPA", "BOUND p", "UNDEFINED_63", "UNDEFINED_64", "UNDEFINED_65", "UNDEFINED_66", "UNDEFINED_67",
	"PUSH j", "IMUL n, j", "PUSH i", "IMUL n, i", "INSB", "INSW", "OUTSB", "OUTSW",

	//70-7F
	"JO r", "JNO r", "JB r", "JNB r", "JZ r", "JNZ r", "JBE r", "JA r",
	"JS r", "JNS r", "JPE r", "JPO r", "JL r", "JGE r", "JLE r", "JG r",

	//80-8F
	"v", "v", "v", "v", "TEST m", "TEST m", "XCHG m", "XCHG m",
	"MOV m", "MOV m", "MOV m", "MOV m", "MOV q", "LEA o", "MOV q", "POP n",

	//90-9F
	"NOP", "XCHG CX, AX", "XCHG DX, AX", "XCHG BX, AX", "XCHG SP, AX", "XCHG BP, AX", "XCHG SI, AX", "XCHG DI, AX",
	"CBW", "CWD", "CALL t", "WAIT", "PUSHF", "POPF", "SAHF", "LAHF",

	//A0-AF
	"MOV AL, a", "MOV AX, a", "MOV a, AL", "MOV a, AX", "MOVSB", "MOVSW", "CMPSB", "CMPSW",
	"TEST AL, i", "TEST AX, j", "STOSB", "STOSW", "LODSB", "LODSW", "SCASB", "SCASW",

	//B0-BF
	"MOV AL, i", "MOV CL, i", "MOV DL, i", "MOV BL, i", "MOV AH, i", "MOV CH, i", "MOV DH, i", "MOV BH, i", 
	"MOV AX, j", "MOV CX, j", "MOV DX, j", "MOV BX, j", "MOV SP, j", "MOV BP, j", "MOV SI, j", "MOV DI, j",

	//C0-CF
	"w, i", "w, i", "RET j", "RET", "LES o", "LDS o", "MOV n, i", "MOV n, j",
	"ENTER j, i", "LEAVE", "RETF j", "RETF", "INT 3", "INT i" , "INTO", "IRET",

	//D0-DF
	"w", "w", "w, CL", "w, CL", "AAM i", "AAD i", "SALC", "XLAT",
	"UNDEFINED_D8", "UNDEFINED_D9", "UNDEFINED_DA", "UNDEFINED_DB", "UNDEFINED_DC", "UNDEFINED_DD", "UNDEFINED_DE", "UNDEFINED_DF",

	//E0-EF
	"LOOPNZ r", "LOOPZ r", "LOOP r", "JCXZ r", "IN AL, i", "IN AX, i", "OUT i, AL", "OUT i, AX",
	"CALL s", "JMP s", "JMP t", "JMP r", "IN AL, DX", "IN AX, DX", "OUT DX, AL", "OUT DX, AX",

	//F0-FF
	"LOCK", "UNDEFINED_F1", "REPNZ", "REPZ", "HLT", "CMC", "x", "x",
	"CLC", "STC", "CLI", "STI", "CLD", "STD", "y", "z"
};

void WsDisUtils::GetDisassembly(DisassemblyInfo& info, string& out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings)
{
	FastString str(settings->GetDebugConfig().UseLowerCaseDisassembly);
	
	uint8_t* byteCode = info.GetByteCode();
	uint8_t opCode = byteCode[0];

	WsSegment segment = WsSegment::Default;
	while(IsPrefix(opCode)) {
		switch(opCode) {
			case 0x26: segment = WsSegment::ES; break;
			case 0x36: segment = WsSegment::SS; break;
			case 0x2E: segment = WsSegment::CS; break;
			case 0x3E: segment = WsSegment::DS; break;

			case 0xF0:
			case 0xF2:
			case 0xF3:
				//rep, lock
				str.WriteAll(_opTemplate[opCode], ' ');
				break;
		}

		byteCode++;
		opCode = byteCode[0];
	}

	const char* op = _opTemplate[opCode];

	int i = 0;
	while(op[i]) {
		switch(op[i]) {
			//Address offset
			case 'a':
				GetSegment(str, segment, "DS");
				str.WriteAll("[$", HexUtilities::ToHex((uint16_t)(byteCode[1] | (byteCode[2] << 8))), "]");
				break;

			//Immediate values
			case 'i': str.WriteAll("$", HexUtilities::ToHex(byteCode[1])); byteCode++; break;
			case 'j': str.WriteAll("$", HexUtilities::ToHex((uint16_t)(byteCode[1] | (byteCode[2] << 8)))); byteCode += 2; break;

			//ModRM (2 operands)
			case 'm': {
				bool word = opCode & 0x01;
				bool direction = opCode & 0x02;

				if(direction) {
					GetModRegParam(str, byteCode, word);
					str.Write(", ");
					GetModRmParam(str, byteCode, segment, word);
				} else {
					GetModRmParam(str, byteCode, segment, word);
					str.Write(", ");
					GetModRegParam(str, byteCode, word);
				}
				break;
			}

			//ModRM (+ immediate value)
			case 'n': {
				bool word = op[i] == 'l';
				byteCode += GetModRmParam(str, byteCode, segment, word);
				break;
			}

			//ModRM (2 operands, LEA/LDS/LES)
			case 'o':
				GetModRegParam(str, byteCode, true);
				str.Write(", ");
				GetModRmParam(str, byteCode, segment, true, true);
				break;

			//ModRM (2 operands, BOUND)
			case 'p':
				GetModRegParam(str, byteCode, true);
				str.Write(", ");
				GetModRmParam(str, byteCode, segment, true, false);
				break;

			//ModRM (mov segment)
			case 'q': {
				bool direction = opCode & 0x02;
				if(direction) {
					GetModSegRegParam(str, byteCode);
					str.Write(", ");
					GetModRmParam(str, byteCode, segment, true);
				} else {
					GetModRmParam(str, byteCode, segment, true);
					str.Write(", ");
					GetModSegRegParam(str, byteCode);
				}
				break;	
			}

			//Relative jumps
			case 'r': GetJmpDestination(str, byteCode, 1); break;
			case 's': GetJmpDestination(str, byteCode, 2); break;
			case 't': GetJmpDestination(str, byteCode, 4); break;

			case 'v': //GRP1
				switch((byteCode[1] >> 3) & 0x07) {
					case 0: str.Write("ADD "); break;
					case 1: str.Write("OR "); break;
					case 2: str.Write("ADC "); break;
					case 3: str.Write("SBB "); break;
					case 4: str.Write("AND "); break;
					case 5: str.Write("SUB "); break;
					case 6: str.Write("XOR "); break;
					case 7: str.Write("CMP "); break;
				}

				byteCode += GetModRmParam(str, byteCode, segment, opCode & 0x01);

				str.Write(", ");
				if((opCode & 0x03) == 0x01) {
					str.WriteAll("$", HexUtilities::ToHex((uint16_t)(byteCode[1] | (byteCode[2] << 8))));
				} else if((opCode & 0x03) == 0x03) {
					str.WriteAll("$", HexUtilities::ToHex((uint16_t)(int16_t)(int8_t)byteCode[1]));
				} else {
					str.WriteAll("$", HexUtilities::ToHex(byteCode[1]));
				}
				break;

			case 'w': { //GRP2
				switch((byteCode[1] >> 3) & 0x07) {
					case 0: str.Write("ROL "); break;
					case 1: str.Write("ROR "); break;
					case 2: str.Write("RCL "); break;
					case 3: str.Write("RCR "); break;
					case 4: str.Write("SHL "); break;
					case 5: str.Write("SHR "); break;
					case 6: str.Write("UNDEFINED_GRP2_6 "); break;
					case 7: str.Write("SAR "); break;
				}

				//Followed by an immediate value for C0/C1
				byteCode += GetModRmParam(str, byteCode, segment, opCode & 0x01);
				break;
			}

			case 'x': { //GRP3
				uint8_t mode = (byteCode[1] >> 3) & 0x07;
				bool word = opCode & 0x01;
				switch(mode) {
					case 0: str.Write("TEST "); break;
					case 1: str.Write("UNDEFINED_GRP3_1 "); break;
					case 2: str.Write("NOT "); break;
					case 3: str.Write("NEG "); break;
					case 4: str.WriteAll("MUL ", word ? "AX, " : ""); break;
					case 5: str.WriteAll("IMUL ", word ? "AX, " : ""); break;
					case 6: str.WriteAll("DIV ", word ? "AX, " : ""); break;
					case 7: str.WriteAll("IDIV ", word ? "AX, " : ""); break;
				}
				byteCode += GetModRmParam(str, byteCode, segment, word);
				if(mode == 0) {
					if(word) {
						str.WriteAll(", $", HexUtilities::ToHex((uint16_t)(byteCode[1] | (byteCode[2] << 8))));
					} else {
						str.WriteAll(", $", HexUtilities::ToHex(byteCode[1]));
					}
				}
				break;
			}

			case 'y': //grp 4
			case 'z': //grp 5
				switch((byteCode[1] >> 3) & 0x07) {
					case 0: str.Write("INC "); break;
					case 1: str.Write("DEC "); break;
					case 2: str.Write("CALL "); break;
					case 3: str.Write("CALL FAR "); break;
					case 4: str.Write("JMP "); break;
					case 5: str.Write("JMP FAR "); break;
					case 6: str.Write("PUSH "); break;
					case 7: str.Write("UNDEFINED_GRP5_7 "); break;
				}
				GetModRmParam(str, byteCode, segment, opCode & 0x01);
				break;

			default: str.Write(op[i]); break;
		}
		i++;
	}

	out += str.ToString();
}

void WsDisUtils::GetJmpDestination(FastString& str, uint8_t* byteCode, uint8_t size)
{
	if(size == 4) {
		str.WriteAll("$", HexUtilities::ToHex((uint16_t)(byteCode[3] | (byteCode[4] << 8))));
		str.WriteAll(":$", HexUtilities::ToHex((uint16_t)(byteCode[1] | (byteCode[2] << 8))));
	} else if(size == 2) {
		uint16_t jmpOffset = (uint16_t)(byteCode[1] | (byteCode[2] << 8));
		str.WriteAll("+$", HexUtilities::ToHex(jmpOffset));
	} else if(size == 1) {
		uint8_t jmpOffset = (uint8_t)byteCode[1];
		str.WriteAll("+$", HexUtilities::ToHex(jmpOffset));
	}
}

void WsDisUtils::GetSegment(FastString& str, WsSegment segment, const char* defaultSegment)
{
	switch(segment) {
		default: case WsSegment::Default: str.WriteAll(defaultSegment, ':'); break;
		case WsSegment::ES: str.Write("ES:"); break;
		case WsSegment::SS: str.Write("SS:"); break;
		case WsSegment::CS: str.Write("CS:"); break;
		case WsSegment::DS: str.Write("DS:"); break;
	}
}

void WsDisUtils::GetModRegParam(FastString& str, uint8_t* byteCode, bool word)
{
	uint8_t modRm = byteCode[1];
	uint8_t reg = (modRm >> 3) & 0x07;
	str.Write(word ? _modRegLut16[reg] : _modRegLut8[reg]);
}

void WsDisUtils::GetModSegRegParam(FastString& str, uint8_t* byteCode)
{
	uint8_t modRm = byteCode[1];
	uint8_t reg = (modRm >> 3) & 0x03;
	str.Write(_modSegRegLut[reg]);
}

int WsDisUtils::GetModRmParam(FastString& str, uint8_t* byteCode, WsSegment segment, bool word, bool forLeaLdsLes)
{
	uint8_t modRm = byteCode[1];
	uint8_t mode = (modRm >> 6) & 0x03;
	uint8_t rm = modRm & 0x07;
	int len = 1;

	string param;
	string param2;

	if(mode == 3) {
		if(forLeaLdsLes) {
			switch(rm) {
				case 0x00: GetSegment(str, segment, "DS"); str.Write("[BX+AX]"); break;
				case 0x01: GetSegment(str, segment, "DS"); str.Write("[BX+CX]"); break;
				case 0x02: GetSegment(str, segment, "SS"); str.Write("[BP+DX]"); break;
				case 0x03: GetSegment(str, segment, "SS"); str.Write("[BP+BX]"); break;
				case 0x04: GetSegment(str, segment, "DS"); str.Write("[SP+SI]"); break;
				case 0x05: GetSegment(str, segment, "DS"); str.Write("[BP+DI]"); break;
				case 0x06: GetSegment(str, segment, "SS"); str.Write("[BP+SI]"); break;
				case 0x07: GetSegment(str, segment, "DS"); str.Write("[BX+DI]"); break;
			}
		} else {
			str.Write(word ? _modRegLut16[rm] : _modRegLut8[rm]);
		}
	} else if(mode == 0 && rm == 0x06) {
		GetSegment(str, segment, "DS");
		str.WriteAll("[$", HexUtilities::ToHex((uint16_t)(byteCode[2] | (byteCode[3] << 8))), ']');
		len += 2;
	} else {
		switch(rm) {
			case 0x00: GetSegment(str, segment, "DS"); str.Write("[BX+SI"); break;
			case 0x01: GetSegment(str, segment, "DS"); str.Write("[BX+DI"); break;
			case 0x02: GetSegment(str, segment, "SS"); str.Write("[BP+SI"); break;
			case 0x03: GetSegment(str, segment, "SS"); str.Write("[BP+DI"); break;
			case 0x04: GetSegment(str, segment, "DS"); str.Write("[SI"); break;
			case 0x05: GetSegment(str, segment, "DS"); str.Write("[DI"); break;
			case 0x06: GetSegment(str, segment, "SS"); str.Write("[BP"); break;
			case 0x07: GetSegment(str, segment, "DS"); str.Write("[BX"); break;
		}

		if(mode == 1) {
			str.WriteAll("+$", HexUtilities::ToHex((uint8_t)byteCode[2]), ']');
			len++;
		} else if(mode == 2) {
			str.WriteAll("+$", HexUtilities::ToHex((uint16_t)(byteCode[2] | (byteCode[3] << 8))), ']');
			len += 2;
		} else {
			str.Write(']');
		}
	}

	return len;
}

EffectiveAddressInfo WsDisUtils::GetEffectiveAddress(DisassemblyInfo& info, WsConsole* console, WsCpuState& state)
{
	DummyWsCpu dummyCpu(nullptr, console->GetMemoryManager());
	dummyCpu.SetDummyState(state);
	dummyCpu.Exec();

	uint16_t opCode = WsDisUtils::GetFullOpCode(info);
	if(WsDisUtils::IsReturnInstruction(opCode) || WsDisUtils::IsPushPopInstruction(opCode)) {
		return {};
	}

	if((uint8_t)opCode == 0xC8 || (uint8_t)opCode == 0xC9) {
		//Show nothing for Enter/Leave
		return {};
	}

	if(WsDisUtils::IsConditionalJump(opCode) || WsDisUtils::IsUnconditionalJump(opCode)) {
		EffectiveAddressInfo result;
		result.ShowAddress = true;
		result.Address = dummyCpu.GetProgramCounter();
		result.Type = MemoryType::WsMemory;
		return result;
	}

	//For everything else, return the last read/write address
	uint32_t count = dummyCpu.GetOperationCount();
	for(int i = count - 1; i > 0; i--) {
		MemoryOperationInfo opInfo = dummyCpu.GetOperationInfo(i);

		if(opInfo.Type != MemoryOperationType::ExecOperand) {
			MemoryOperationInfo prevOpInfo = dummyCpu.GetOperationInfo(i - 1);
			EffectiveAddressInfo result;
			result.ShowAddress = true;
			
			if(dummyCpu.IsWordAccess(i)) {
				result.ValueSize = 2;
				if(dummyCpu.IsWordAccess(i - 1) && prevOpInfo.Type == opInfo.Type && prevOpInfo.Address == opInfo.Address - 2) {
					result.Address = prevOpInfo.Address;
					result.Type = prevOpInfo.MemType;
				} else {
					result.Address = opInfo.Address;
					result.Type = opInfo.MemType;
				}
			} else {
				if(prevOpInfo.Type == opInfo.Type && prevOpInfo.Address == opInfo.Address - 1) {
					//For 16-bit read/writes that were split into 2 8-bit accesses, return the first address
					result.Address = prevOpInfo.Address;
					result.Type = prevOpInfo.MemType;
					result.ValueSize = 2;
				} else {
					result.Address = opInfo.Address;
					result.Type = opInfo.MemType;
					result.ValueSize = 1;
				}
			}
			return result;
		}
	}

	return {};
}

int WsDisUtils::GetModRmSize(uint8_t modRm)
{
	uint8_t mode = (modRm >> 6) & 0x03;
	uint8_t rm = modRm & 0x07;
	if(mode == 2 || (mode == 0 && rm == 0x06)) {
		return 3;
	} else if(mode == 1) {
		return 2;
	}
	return 1;
}

bool WsDisUtils::IsPrefix(uint8_t opCode)
{
	return opCode == 0xF0 || opCode == 0xF2 || opCode == 0xF3 || opCode == 0x26 || opCode == 0x36 || opCode == 0x2E || opCode == 0x3F;
}

uint8_t WsDisUtils::GetOpSize(uint32_t cpuAddress, MemoryType memType, MemoryDumper* memoryDumper)
{
	uint8_t size = 1;
	uint8_t opCode = memoryDumper->GetMemoryValue(memType, cpuAddress);
	while(IsPrefix(opCode)) {
		cpuAddress++;
		opCode = memoryDumper->GetMemoryValue(memType, cpuAddress);
		size++;
	}

	const char* op = _opTemplate[opCode];

	int i = 0;

	while(op[i]) {
		switch(op[i]) {
			//Absolute address
			case 'a': size+=2; break;
			
			//Immediate values
			case 'i': size++; break;
			case 'j': size += 2; break;

			//ModRM
			case 'm':
			case 'n':
			case 'o':
			case 'p':
			case 'q':
				size += GetModRmSize(memoryDumper->GetMemoryValue(memType, cpuAddress + 1));
				break;

			//Relative jumps
			case 'r': size++; break;
			case 's': size += 2; break;
			case 't': size += 4; break;

			case 'v': //grp1
				size += GetModRmSize(memoryDumper->GetMemoryValue(memType, cpuAddress + 1));
				size++;
				if((opCode & 0x3) == 0x01) {
					size++;
				}
				break;

			case 'x': { //grp3
				uint8_t modRm = memoryDumper->GetMemoryValue(memType, cpuAddress + 1);
				size += GetModRmSize(modRm);
				if(((modRm >> 3) & 0x07) == 0) {
					size += (opCode & 0x01) ? 2 : 1;
				}
				break;
			}

			case 'w': //grp2
			case 'y': //grp4
			case 'z': //grp5
				size += GetModRmSize(memoryDumper->GetMemoryValue(memType, cpuAddress + 1));
				break;

			default: break;
		}
		i++;
	}

	return size;
}

uint16_t WsDisUtils::GetFullOpCode(uint16_t cs, uint16_t ip, WsMemoryManager* memoryManager)
{
	int i = 0;
	while(IsPrefix(memoryManager->DebugRead((cs << 4) + ip)) && i < 6) {
		ip++;
		i++;
	}

	return (
		memoryManager->DebugRead((cs << 4) + ip) |
		(memoryManager->DebugRead((cs << 4) + (uint16_t)(ip + 1)) << 8)
	);
}


uint16_t WsDisUtils::GetFullOpCode(DisassemblyInfo& disInfo)
{
	uint8_t* byteCode = disInfo.GetByteCode();

	int i = 0;
	while(IsPrefix(byteCode[i]) && i < 6) {
		i++;
	}

	return byteCode[i] | (byteCode[i+1] << 8);
}

bool WsDisUtils::IsJumpToSub(uint16_t opCode)
{
	switch((uint8_t)opCode) {
		case 0x9A:
		case 0xE8:
			return true;

		case 0xFE:
		case 0xFF: {
			uint8_t mode = (opCode >> 11) & 0x07;
			switch(mode) {
				case 2: //CALL
				case 3: //CALL FAR
					return true;

				default:
					return false;
			}
		}
	}
	return false;
}

bool WsDisUtils::IsReturnInstruction(uint16_t opCode)
{
	switch((uint8_t)opCode) {
		case 0xC2: //RET
		case 0xC3: //RET
		case 0xCA: //RETF
		case 0xCB: //RETF
		case 0xCF: //IRET
			return true;
	}
	return false;
}

bool WsDisUtils::IsUnconditionalJump(uint16_t opCode)
{
	switch((uint8_t)opCode) {
		case 0x9A: //CALL
		case 0xC2: //RET
		case 0xC3: //RET
		case 0xCA: //RETF
		case 0xCB: //RETF
		case 0xCF: //IRET
		case 0xE8: //CALL relative
		case 0xE9: //JMP
		case 0xEA: //JMP
		case 0xEB: //JMP
			return true;

		case 0xFE:
		case 0xFF: {
			uint8_t mode = (opCode >> 11) & 0x07;
			switch(mode) {
				case 2: //CALL
				case 3: //CALL FAR
				case 4: //JMP
				case 5: //JMP FAR
					return true;

				default:
					return false;
			}
		}
	}

	return false;
}

bool WsDisUtils::IsConditionalJump(uint16_t opCode)
{
	switch((uint8_t)opCode) {
		case 0x70: //JO
		case 0x71: //JNO
		case 0x72: //JB
		case 0x73: //JNB
		case 0x74: //JZ
		case 0x75: //JNZ
		case 0x76: //JBE
		case 0x77: //JA
		case 0x78: //JS
		case 0x79: //JNS
		case 0x7A: //JPE
		case 0x7B: //JPO
		case 0x7C: //JL
		case 0x7D: //JGE
		case 0x7E: //JLE
		case 0x7F: //JG
		case 0xE0: //LOOPNZ
		case 0xE1: //LOOPZ
		case 0xE2: //LOOP
		case 0xE3: //JCXZ
			return true;
	}

	return false;
}

bool WsDisUtils::IsUndefinedOpCode(uint16_t opCode)
{
	switch((uint8_t)opCode) {
		case 0x0F:
		case 0x63:
		case 0x64:
		case 0x65:
		case 0x66:
		case 0x67:
		case 0xD8:
		case 0xD9:
		case 0xDA:
		case 0xDB:
		case 0xDC:
		case 0xDD:
		case 0xDE:
		case 0xDF:
		case 0xF1:
			return true;

		case 0xC0:
		case 0xC1:
		case 0xD0:
		case 0xD1:
		case 0xD2:
		case 0xD3:
			return ((opCode >> 11) & 0x07) == 0x06;

		case 0xF6:
		case 0xF7:
			return ((opCode >> 11) & 0x07) == 0x01;

		case 0xFE:
		case 0xFF:
			return ((opCode >> 11) & 0x07) == 0x07;

		default:
			return false;
	}
}

bool WsDisUtils::IsPushPopInstruction(uint16_t opCode)
{
	switch((uint8_t)opCode) {
		//Push/pop segments
		case 0x06: case 0x07: case 0x0E: case 0x16:
		case 0x17: case 0x1E: case 0x1F:

		//Push
		case 0x50: case 0x51: case 0x52: case 0x53:
		case 0x54: case 0x55: case 0x56: case 0x57:

		//Pop
		case 0x58: case 0x59: case 0x5A: case 0x5B:
		case 0x5C: case 0x5D: case 0x5E: case 0x5F:

		//Push/pop all
		case 0x60: case 0x61:

		//Push immediate
		case 0x68: case 0x6A:

		//Push/pop flags
		case 0x9C: case 0x9D:
			return true;
	}

	return false;
}

CdlFlags::CdlFlags WsDisUtils::GetOpFlags(uint16_t opCode, uint32_t pc, uint32_t prevPc, uint8_t opSize)
{
	if(WsDisUtils::IsJumpToSub(opCode)) {
		return CdlFlags::SubEntryPoint;
	} else if(WsDisUtils::IsUnconditionalJump(opCode) && !WsDisUtils::IsReturnInstruction(opCode)) {
		return CdlFlags::JumpTarget;
	} else if(WsDisUtils::IsConditionalJump(opCode) && (pc != prevPc + opSize)) {
		return CdlFlags::JumpTarget;
	}
	
	return CdlFlags::None;
}

void WsDisUtils::UpdateAddressCsIp(uint32_t addr, WsCpuState& state)
{
	uint32_t baseAddr = (state.CS << 4);
	if(addr >= baseAddr && addr <= baseAddr + 0xFFFF) {
		//address fits within the current code segment, only change the instruction pointer
		state.IP = addr - baseAddr;
	} else {
		//address is outside the range of the current CS, change both values
		state.CS = (addr & 0xF0000) >> 4;
		state.IP = (addr & 0xFFFF);
	}
}

#include "pch.h"
#include "SNES/Debugger/GsuDisUtils.h"
#include "SNES/Coprocessors/GSU/GsuTypes.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/LabelManager.h"
#include "Shared/EmuSettings.h"
#include "Utilities/FastString.h"
#include "Utilities/HexUtilities.h"
#include "Shared/MemoryType.h"

void GsuDisUtils::GetDisassembly(DisassemblyInfo &info, string &out, uint32_t memoryAddr, LabelManager *labelManager, EmuSettings *settings)
{
	constexpr const char* registerNames[16] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15" };
	bool alt1 = (info.GetFlags() & 0x01) != 0;
	bool alt2 = (info.GetFlags() & 0x02) != 0;
	bool prefix = (info.GetFlags() & 0x10) != 0;

	uint8_t opCode = info.GetOpCode();

	FastString str(settings->GetDebugConfig().UseLowerCaseDisassembly);

	auto getJumpTarget = [&str, labelManager, memoryAddr, &info]() {
		uint32_t jmpTarget = (memoryAddr + (int8_t)info.GetByteCode()[1] + 2) & 0xFFFFFF;
		AddressInfo address = { (int32_t)jmpTarget, MemoryType::GsuMemory };
		string label = labelManager ? labelManager->GetLabel(address) : "";
		if(label.empty()) {
			str.WriteAll('$', HexUtilities::ToHex24(jmpTarget));
		} else {
			str.Write(label, true);
		}
	};

	const char* reg = registerNames[opCode & 0x0F];

	switch(opCode) {
		case 0x00: str.Write("STOP"); break;
		case 0x01: str.Write("NOP"); break;
		case 0x02: str.Write("CACHE"); break;
		case 0x03: str.Write("LSR"); break;
		case 0x04: str.Write("ROL"); break;
		
		case 0x05: str.WriteAll("BRA "); getJumpTarget(); break;
		case 0x06: str.WriteAll("BGE "); getJumpTarget(); break;
		case 0x07: str.WriteAll("BLT "); getJumpTarget(); break;
		case 0x08: str.WriteAll("BNE "); getJumpTarget(); break;
		case 0x09: str.WriteAll("BEQ "); getJumpTarget(); break;
		case 0x0A: str.WriteAll("BPL "); getJumpTarget(); break;
		case 0x0B: str.WriteAll("BMI "); getJumpTarget(); break;
		case 0x0C: str.WriteAll("BCC "); getJumpTarget(); break;
		case 0x0D: str.WriteAll("BCS "); getJumpTarget(); break;
		case 0x0E: str.WriteAll("BVC "); getJumpTarget(); break;
		case 0x0F: str.WriteAll("BVS "); getJumpTarget(); break;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E: case 0x1F:
			str.WriteAll(prefix ? "MOVE R" : "TO R", reg);
			break;

		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2A: case 0x2B: case 0x2C: case 0x2D: case 0x2E: case 0x2F:
			str.WriteAll("WITH R", reg);
			break;

		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3A: case 0x3B:
			str.WriteAll(alt1 ? "STB (R" : "STW (R", reg, ')');
			break;

		case 0x3C: str.Write("LOOP"); break;
		case 0x3D: str.Write("ALT1"); break;
		case 0x3E: str.Write("ALT2"); break;
		case 0x3F: str.Write("ALT3"); break;

		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
		case 0x48: case 0x49: case 0x4A: case 0x4B:
			str.WriteAll(alt1 ? "LDB (R" : "LDW (R", reg, ')');
			break;

		case 0x4C: str.Write(alt1 ? "RPIX" : "PLOT"); break;

		case 0x4D: str.Write("SWAP"); break;
		case 0x4E: str.Write(alt1 ? "CMODE" : "COLOR"); break;

		case 0x4F: str.Write("NOT"); break;

		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5E: case 0x5F:
			str.Write(alt1 ? "ADC " : "ADD ");
			str.WriteAll(alt2 ? '#' : 'R', reg);
			break;

		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6E: case 0x6F:
			if(alt2 && alt1) {
				str.WriteAll("CMP R", reg);
			} else {
				if(!alt2 && alt1) {
					str.Write("SBC ");
				} else {
					str.Write("SUB ");
				}
				if(alt2 && !alt1) {
					str.WriteAll('#', reg);
				} else {
					str.WriteAll('R', reg);
				}
			}
			break;

		case 0x70: str.Write("MERGE"); break;

		case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F:
			str.Write(alt1 ? "BIC " : "AND ");
			str.WriteAll(alt2 ? '#' : 'R', reg);
			break;

		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
		case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D: case 0x8E: case 0x8F:
			str.Write(alt1 ? "UMULT " : "MULT ");
			str.WriteAll(alt2 ? '#' : 'R', reg);
			break;

		case 0x90: str.Write("SBK"); break;

		case 0x91: case 0x92: case 0x93: case 0x94:
			str.WriteAll("LINK #", reg); 
			break;

		case 0x95: str.Write("SEX"); break;

		case 0x96: str.Write(alt1 ? "DIV2" : "ASR"); break;
		case 0x97: str.Write("ROR"); break;

		case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D:
			str.WriteAll(alt1 ? "LJMP R" : "JMP R", reg);
			break;

		case 0x9E: str.Write("LOB"); break;
		case 0x9F: str.Write(alt1 ? "LMULT" : "FMULT"); break;

		case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5: case 0xA6: case 0xA7:
		case 0xA8: case 0xA9: case 0xAA: case 0xAB: case 0xAC: case 0xAD: case 0xAE: case 0xAF:
			if(alt1) {
				str.WriteAll("LMS R", reg, ",($", HexUtilities::ToHex(info.GetByteCode()[1] << 1), ')');
			} else if(alt2) {
				str.WriteAll("SMS R", reg, ",($", HexUtilities::ToHex(info.GetByteCode()[1] << 1), ')');
			} else {
				str.WriteAll("IBT R", reg, ",#$", HexUtilities::ToHex(info.GetByteCode()[1]));
			}
			break;

		case 0xB0: case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6: case 0xB7:
		case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBE: case 0xBF:
			str.WriteAll(prefix ? "MOVES R" : "FROM R", reg);
			break;

		case 0xC0: str.Write("HIB"); break;

		case 0xC1: case 0xC2: case 0xC3: case 0xC4: case 0xC5: case 0xC6: case 0xC7:
		case 0xC8: case 0xC9: case 0xCA: case 0xCB: case 0xCC: case 0xCD: case 0xCE: case 0xCF:
			str.Write(alt1 ? "XOR " : "OR ");
			str.WriteAll(alt2 ? '#' : 'R', reg);
			break;

		case 0xD0: case 0xD1: case 0xD2: case 0xD3: case 0xD4: case 0xD5: case 0xD6: case 0xD7:
		case 0xD8: case 0xD9: case 0xDA: case 0xDB: case 0xDC: case 0xDD: case 0xDE:
			str.WriteAll("INC R", reg);
			break;

		case 0xDF: 
			if(!alt2) {
				//GETC - "Get byte from ROM to color register"
				str.Write("GETC");
			} else if(!alt1) {
				//RAMB - "Set RAM data bank"
				str.Write("RAMB");
			} else {
				str.Write("ROMB");
			}
			break;

		case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5: case 0xE6: case 0xE7:
		case 0xE8: case 0xE9: case 0xEA: case 0xEB: case 0xEC: case 0xED: case 0xEE:
			str.WriteAll("DEC R", reg);
			break;

		case 0xEF:
			if(alt2 && alt1) {
				str.Write("GETBS");
			} else if(alt2) {
				str.Write("GETBL");
			} else if(alt1) {
				str.Write("GETBH");
			} else {
				str.Write("GETB");
			}
			break;

		case 0xF0: case 0xF1: case 0xF2: case 0xF3: case 0xF4: case 0xF5: case 0xF6: case 0xF7:
		case 0xF8: case 0xF9: case 0xFA: case 0xFB: case 0xFC: case 0xFD: case 0xFE: case 0xFF:
			if(alt1) {
				str.WriteAll("LM R", reg, ",($", HexUtilities::ToHex(info.GetByteCode()[2]), HexUtilities::ToHex(info.GetByteCode()[1]), ')');
			} else if(alt2) {
				str.WriteAll("SM R", reg, ",($", HexUtilities::ToHex(info.GetByteCode()[2]), HexUtilities::ToHex(info.GetByteCode()[1]), ')');
			} else {
				str.WriteAll("IWT R", reg, ",#$", HexUtilities::ToHex(info.GetByteCode()[2]), HexUtilities::ToHex(info.GetByteCode()[1]));
			}
			break;
	}

	out += str.ToString();
}

EffectiveAddressInfo GsuDisUtils::GetEffectiveAddress(DisassemblyInfo& info, SnesConsole* console, GsuState& state)
{
	uint8_t opCode = info.GetOpCode();
	bool alt1 = (info.GetFlags() & 0x01) != 0;
	bool alt2 = (info.GetFlags() & 0x02) != 0;

	switch(opCode) {
		case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5: case 0xA6: case 0xA7:
		case 0xA8: case 0xA9: case 0xAA: case 0xAB: case 0xAC: case 0xAD: case 0xAE: case 0xAF:
			if(alt1 || alt2) {
				return { 0x700000 | (info.GetByteCode()[1] << 1) | (state.RamBank << 16), 2, true };
			}
			break;

		case 0xF0: case 0xF1: case 0xF2: case 0xF3: case 0xF4: case 0xF5: case 0xF6: case 0xF7:
		case 0xF8: case 0xF9: case 0xFA: case 0xFB: case 0xFC: case 0xFD: case 0xFE: case 0xFF:
			if(alt1 || alt2) {
				return { 0x700000 | info.GetByteCode()[1] | (info.GetByteCode()[2] << 8) | (state.RamBank << 16), 2, true };
			}
			break;
	}

	return {};
}

bool GsuDisUtils::IsUnconditionalJump(uint8_t opCode)
{
	switch(opCode) {
		case 0x05: //BRA
		case 0x98: //JMP
		case 0x99: //JMP
		case 0x9A: //JMP
		case 0x9B: //JMP
		case 0x9C: //JMP
		case 0x9D: //JMP
			return true;

		default:
			return false;
	}
}

bool GsuDisUtils::IsConditionalJump(uint8_t opCode)
{
	switch(opCode) {
		case 0x06: //BLT
		case 0x07: //BGE
		case 0x08: //BNE
		case 0x09: //BEQ
		case 0x0A: //BPL
		case 0x0B: //BMI
		case 0x0C: //BCC
		case 0x0D: //BCS
		case 0x0E: //BCV
		case 0x0F: //BVS
		case 0x3C: //LOOP
			return true;

		default:
			return false;
	}
}

uint8_t GsuDisUtils::GetOpSize(uint8_t opCode)
{
	if(opCode >= 0x05 && opCode <= 0x0F) {
		return 2;
	} else if(opCode >= 0xA0 && opCode <= 0xAF) {
		return 2;
	} else if(opCode >= 0xF0 && opCode <= 0xFF) {
		return 3;
	}
	return 1;
}

bool GsuDisUtils::CanDisassembleNextOp(uint8_t opCode)
{
	//STOP instruction (Op code $00)
	return opCode != 0;
}

void GsuDisUtils::UpdateCpuFlags(uint8_t opCode, uint8_t& cpuFlags)
{
	switch(opCode) {
		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2A: case 0x2B: case 0x2C: case 0x2D: case 0x2E: case 0x2F:
			//WITH - Prefix
			cpuFlags |= 0x10;
			break;

		case 0x3D:
			//ALT1
			cpuFlags &= 0x03;
			cpuFlags |= 0x01;
			break;

		case 0x3E:
			//ALT2
			cpuFlags &= 0x03;
			cpuFlags |= 0x02;
			break;

		case 0x3F:
			//ALT3
			cpuFlags &= 0x03;
			cpuFlags |= 0x03;
			break;

		default:
			cpuFlags = 0;
			break;
	}
}

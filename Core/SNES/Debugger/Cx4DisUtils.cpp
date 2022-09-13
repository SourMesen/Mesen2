#include "pch.h"
#include "SNES/Debugger/Cx4DisUtils.h"
#include "SNES/Coprocessors/CX4/Cx4Types.h"
#include "Debugger/DisassemblyInfo.h"
#include "Shared/EmuSettings.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/FastString.h"

void Cx4DisUtils::GetDisassembly(DisassemblyInfo &info, string &out, uint32_t memoryAddr, LabelManager *labelManager, EmuSettings *settings)
{
	FastString str(settings->GetDebugConfig().UseLowerCaseDisassembly);

	uint8_t op = info.GetByteCode()[1] & 0xFC;
	uint8_t param1 = info.GetByteCode()[1] & 0x03;
	uint8_t param2 = info.GetByteCode()[0] & 0xFF;

	auto writeSrc = [&str, param2]() -> void {
		switch(param2 & 0x7F) {
			case 0x00: str.Write("A"); break;
			case 0x01: str.Write("MULTH"); break;
			case 0x02: str.Write("MULTL"); break;
			case 0x03: str.Write("MDR"); break;
			case 0x08: str.Write("ROM"); break;
			case 0x0C: str.Write("RAM"); break;
			case 0x13: str.Write("MAR"); break;
			case 0x1C: str.Write("DPR"); break;
			case 0x20: str.Write("PC"); break;
			case 0x28: str.Write("P"); break;

			case 0x2E: str.Write("RDROM"); break;

			case 0x2F: str.Write("RDRAM"); break;

			case 0x50: str.Write("#$000000"); break;
			case 0x51: str.Write("#$FFFFFF"); break;
			case 0x52: str.Write("#$00FF00"); break;
			case 0x53: str.Write("#$FF0000"); break;
			case 0x54: str.Write("#$00FFFF"); break;
			case 0x55: str.Write("#$FFFF00"); break;
			case 0x56: str.Write("#$800000"); break;
			case 0x57: str.Write("#$7FFFFF"); break;
			case 0x58: str.Write("#$008000"); break;
			case 0x59: str.Write("#$007FFF"); break;
			case 0x5A: str.Write("#$FF7FFF"); break;
			case 0x5B: str.Write("#$FFFF7F"); break;
			case 0x5C: str.Write("#$010000"); break;
			case 0x5D: str.Write("#$FEFFFF"); break;
			case 0x5E: str.Write("#$000100"); break;
			case 0x5F: str.Write("#$00FEFF"); break;

			case 0x60: case 0x70: str.Write("R0"); break;
			case 0x61: case 0x71: str.Write("R1"); break;
			case 0x62: case 0x72: str.Write("R2"); break;
			case 0x63: case 0x73: str.Write("R3"); break;
			case 0x64: case 0x74: str.Write("R4"); break;
			case 0x65: case 0x75: str.Write("R5"); break;
			case 0x66: case 0x76: str.Write("R6"); break;
			case 0x67: case 0x77: str.Write("R7"); break;
			case 0x68: case 0x78: str.Write("R8"); break;
			case 0x69: case 0x79: str.Write("R9"); break;
			case 0x6A: case 0x7A: str.Write("R10"); break;
			case 0x6B: case 0x7B: str.Write("R11"); break;
			case 0x6C: case 0x7C: str.Write("R12"); break;
			case 0x6D: case 0x7D: str.Write("R13"); break;
			case 0x6E: case 0x7E: str.Write("R14"); break;
			case 0x6F: case 0x7F: str.Write("R15"); break;
		}
	};

	auto writeDest = [&str, param1]() -> void {
		switch(param1) {
			case 0: str.Write("A"); break;
			case 1: str.Write("MDR"); break;
			case 2: str.Write("MAR"); break;
			case 3: str.Write("P"); break;
		}
	};

	auto writeShiftedA = [&str, param1]() -> void {
		switch(param1) {
			case 0: str.Write("A"); break;
			case 1: str.Write("(A << 1)"); break;
			case 2: str.Write("(A << 8)"); break;
			case 3: str.Write("(A << 16)"); break;
		}
	};

	auto writeBranchTarget = [&str, param1, param2]() -> void {
		if(param1) {
			//Far jump
			str.Write("P:");
		}
		str.WriteAll('$', HexUtilities::ToHex(param2));
	};
	
	switch(op) {
		case 0x00: str.Write("NOP"); break;
		case 0x04: str.Write("???"); break;
		case 0x08: str.Write("BRA "); writeBranchTarget(); break;
		case 0x0C: str.Write("BEQ "); writeBranchTarget(); break;

		case 0x10: str.Write("BCS "); writeBranchTarget(); break;
		case 0x14: str.Write("BMI "); writeBranchTarget(); break;
		case 0x18: str.Write("BVS "); writeBranchTarget(); break;
		case 0x1C: str.Write("WAIT"); break;

		case 0x20: str.Write("???"); break;
		case 0x24:
			str.Write("SKIP"); 
			switch(param1) {
				case 0: str.Write('V'); break;
				case 1: str.Write('C'); break;
				case 2: str.Write('Z'); break;
				case 3: str.Write('N'); break;
			}
			str.Write((param2 & 0x01) ? 'S' : 'C');
			break;

		case 0x28: str.Write("JSR "); writeBranchTarget(); break;
		case 0x2C: str.Write("JEQ "); writeBranchTarget(); break;

		case 0x30: str.Write("JCS "); writeBranchTarget(); break;
		case 0x34: str.Write("JMI "); writeBranchTarget(); break;
		case 0x38: str.Write("JVS "); writeBranchTarget(); break;
		case 0x3C: str.Write("RTS"); break;

		case 0x40: str.Write("INC MAR"); break;
		case 0x44: str.Write("???"); break;
		case 0x48: str.Write("CMPR "); writeSrc(); str.Write(","); writeShiftedA(); break;
		case 0x4C: str.WriteAll("CMPR #$", HexUtilities::ToHex(param2)); str.Write(","); writeShiftedA(); break;

		case 0x50: str.Write("CMP "); writeShiftedA(); str.Write(","); writeSrc(); break;
		case 0x54: str.WriteAll("CMP "); writeShiftedA(); str.WriteAll(",#$", HexUtilities::ToHex(param2)); break;
		case 0x58:
			if(param1 == 1) {
				str.Write("SXB");
			} else if(param1 == 2) {
				str.Write("SXW");
			} else {
				str.Write("???");
			}
			break;
		case 0x5C: str.Write("???"); break;

		case 0x60: str.Write("LD "); writeDest(); str.Write(","); writeSrc(); break;
		case 0x64: str.Write("LD "); writeDest(); str.WriteAll(", #$", HexUtilities::ToHex(param2)); break;

		case 0x68: str.WriteAll("RDRAM RAM:", '0' + param1, ",A"); break;
		case 0x6C: str.WriteAll("RDRAM RAM:", '0' + param1, ",DPR+#$", HexUtilities::ToHex(param2)); break;

		case 0x70: str.Write("RDROM (a)"); break;
		case 0x74: str.WriteAll("RDROM (#$", HexUtilities::ToHex((param1 << 8) | param2), ")"); break;
		case 0x78: str.Write("???"); break;

		case 0x7C: 
			if(param1 <= 1) {
				str.WriteAll("LD P", param1 ? "H" : "L", ",#$", HexUtilities::ToHex(param2));
			} else {
				str.Write("???");
			}
			break;

		case 0x80: str.Write("ADD "); writeShiftedA(); str.Write(","); writeSrc(); break;
		case 0x84: str.WriteAll("ADD "); writeShiftedA(); str.WriteAll(",#$", HexUtilities::ToHex(param2)); break;
		case 0x88: str.Write("SUBR "); writeSrc(); str.Write(","); writeShiftedA(); break;
		case 0x8C: str.WriteAll("SUBR #$", HexUtilities::ToHex(param2)); str.Write(","); writeShiftedA(); break;

		case 0x90: str.Write("SUB "); writeShiftedA(); str.Write(","); writeSrc(); break;
		case 0x94: str.WriteAll("SUB "); writeShiftedA(); str.WriteAll(",#$", HexUtilities::ToHex(param2)); break;
		case 0x98: str.Write("SMUL A, "); writeSrc(); break;
		case 0x9C: str.WriteAll("SMUL A,#$", HexUtilities::ToHex(param2)); break;

		case 0xA0: str.Write("XNOR "); writeShiftedA(); str.Write(","); writeSrc(); break;
		case 0xA4: str.WriteAll("XNOR "); writeShiftedA(); str.WriteAll(",#$", HexUtilities::ToHex(param2)); break;
		case 0xA8: str.Write("XOR "); writeShiftedA(); str.Write(","); writeSrc(); break;
		case 0xAC: str.WriteAll("XOR "); writeShiftedA(); str.WriteAll(",#$", HexUtilities::ToHex(param2)); break;

		case 0xB0: str.Write("AND "); writeShiftedA(); str.Write(","); writeSrc(); break;
		case 0xB4: str.WriteAll("AND "); writeShiftedA(); str.WriteAll(",#$", HexUtilities::ToHex(param2)); break;
		case 0xB8: str.Write("OR "); writeShiftedA(); str.Write(","); writeSrc(); break;
		case 0xBC: str.WriteAll("OR "); writeShiftedA(); str.WriteAll(",#$", HexUtilities::ToHex(param2)); break;

		case 0xC0: str.Write("SHR A,"); writeSrc(); break;
		case 0xC4: str.WriteAll("SHR A,#$", HexUtilities::ToHex(param2 & 0x1F)); break;
		case 0xC8: str.Write("ASR A,"); writeSrc(); break;
		case 0xCC: str.WriteAll("ASR A,#$", HexUtilities::ToHex(param2 & 0x1F)); break;

		case 0xD0: str.Write("ROR A,"); writeSrc(); break;
		case 0xD4: str.WriteAll("ROR A,#$", HexUtilities::ToHex(param2 & 0x1F)); break;
		case 0xD8: str.Write("SHL A,"); writeSrc(); break;
		case 0xDC: str.WriteAll("SHL A,#$", HexUtilities::ToHex(param2 & 0x1F)); break;

		case 0xE0: 
			if(param1 <= 1) {
				str.Write("ST "); writeSrc(); str.WriteAll(",", param1 ? "MDR" : "A");
			} else {
				str.Write("???");
			}
			break;
		case 0xE4: str.Write("???"); break;
		case 0xE8: str.WriteAll("WRRAM A,RAM:", '0' + param1); break;
		case 0xEC: str.WriteAll("WRRAM DPR+#$", HexUtilities::ToHex(param2), ",RAM:", '0' + param1); break;

		case 0xF0: str.WriteAll("SWAP A,R", std::to_string(param2 & 0x0F)); break;
		case 0xF4: str.Write("???"); break;
		case 0xF8: str.Write("???"); break;
		case 0xFC: str.Write("STOP"); break;
	}

	out += str.ToString();
}

EffectiveAddressInfo Cx4DisUtils::GetEffectiveAddress(DisassemblyInfo& info, Cx4State& state, MemoryDumper* memoryDumper)
{
	uint8_t op = info.GetByteCode()[1] & 0xFC;
	uint8_t param1 = info.GetByteCode()[1] & 0x03;
	uint8_t param2 = info.GetByteCode()[0] & 0xFF;

	switch(op) {
		default: return {};

		case 0x08:
		case 0x0C:
		case 0x10:
		case 0x14:
		case 0x18:
		case 0x28:
		case 0x2C:
		case 0x30:
		case 0x34:
		case 0x38:
			//Show destination address for branches & JSR
			if(param1) {
				return { (state.P << 9) | (param2 * 2), 0, true };
			} else {
				return { (int32_t)(state.Cache.Address[state.Cache.Page] | (param2 * 2)), 0, true };
			}
	}
}

bool Cx4DisUtils::IsConditionalJump(uint8_t opCode, uint8_t param)
{
	switch(opCode) {
		case 0x0C: //BEQ
		case 0x10: //BCS
		case 0x14: //BMI
		case 0x18: //BVS
		case 0x24: //SKIP
		case 0x2C: //JEQ
		case 0x30: //JCS
		case 0x34: //JMI
		case 0x38: //JVS
			return true;

		case 0xE0:
			return param == 0x20;

		default:
			return false;
	}
}

bool Cx4DisUtils::IsUnconditionalJump(uint8_t opCode)
{
	switch(opCode) {
		case 0x08: //BRA
		case 0x28: //JSR
		case 0x3C: //RTS
			return true;

		default:
			return false;
	}
}

bool Cx4DisUtils::IsJumpToSub(uint8_t opCode)
{
	switch(opCode) {
		case 0x28: //JSR
		case 0x2C: //JEQ
		case 0x30: //JCS
		case 0x34: //JMI
		case 0x38: //JVS
			return true;
	}

	return false;
}

bool Cx4DisUtils::IsReturnInstruction(uint8_t opCode)
{
	return opCode == 0x3C; //RTS
}

bool Cx4DisUtils::CanDisassembleNextOp(uint8_t opCode)
{
	//STOP instruction (op code $FC)
	return (opCode & 0xFC) != 0xFC;
}

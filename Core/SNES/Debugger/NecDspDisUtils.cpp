#include "pch.h"
#include "SNES/Debugger/NecDspDisUtils.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/LabelManager.h"
#include "Debugger/DebugTypes.h"
#include "Shared/EmuSettings.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/FastString.h"
#include "Shared/MemoryType.h"

void NecDspDisUtils::GetDisassembly(DisassemblyInfo &info, string &out, uint32_t memoryAddr, LabelManager *labelManager, EmuSettings* settings)
{
	constexpr const char* aluOperations[16] = { "NOP", "OR", "AND", "XOR", "SUB", "ADD", "SBC", "ADC", "DEC" , "INC", "CMP", "SHR1", "SHL1", "SHL2", "SHL4", "XCHG" };
	constexpr const char* sourceNames[16] = { "TRB", "A", "B", "TR", "DP", "RP", "ROM", "SGN", "DR", "DRNF", "SR", "SIM", "SIL" ,"K", "L", "MEM" };
	constexpr const char* destNames[16] = { "NON", "A", "B", "TR", "DP", "RP", "DR", "SR", "SOL", "SOM", "K", "KLR", "KLM", "L", "TRB", "MEM" };
	constexpr const char* dataPointerOp[4] = { "DPL:NOP", "DPL:INC", "DPL:DEC", "DPL:CLR" };

	FastString str(settings->GetDebugConfig().UseLowerCaseDisassembly);
	uint32_t opCode = info.GetByteCode()[0] | (info.GetByteCode()[1] << 8) | (info.GetByteCode()[2] << 16);
	uint8_t operationType = (opCode >> 22) & 0x03;

	if(operationType <= 1) {
		//OP or RT
		uint8_t aluOperation = (opCode >> 16) & 0x0F;
		uint8_t source = (opCode >> 4) & 0x0F;
		uint8_t accSelect = (opCode >> 15) & 0x01;
		if(aluOperation) {
			str.WriteAll(aluOperations[aluOperation], " ");

			if(aluOperation <= 7) {
				uint8_t pSelect = (opCode >> 20) & 0x03;

				switch(pSelect) {
					case 0: str.Write("RAM,"); break;
					case 1: str.WriteAll(sourceNames[source], ","); break;
					case 2: str.Write("M,"); break;
					case 3: str.Write("N,"); break;
				}
			}

			str.Write(accSelect ? "B" : "A");
		}
		
		uint8_t dest = opCode & 0x0F;
		if(dest) {
			str.Delimiter(" | ");
			str.Write("MOV ");
			str.WriteAll(sourceNames[source], ",");
			str.Write(destNames[dest]);
		}
		
		uint8_t dpLow = (opCode >> 13) & 0x03;
		if(dpLow) {
			str.Delimiter(" | ");
			str.Write(dataPointerOp[dpLow]);
		}

		uint8_t dpHighModify = (opCode >> 9) & 0x0F;
		if(dpHighModify) {
			str.Delimiter(" | ");
			str.WriteAll("DPH:$", HexUtilities::ToHex(dpHighModify));
		}

		uint8_t rpDecrement = (opCode >> 8) & 0x01;
		if(rpDecrement) {
			str.Delimiter(" | ");
			str.Write("RP:DEC");
		}

		if(operationType == 1) {
			str.Delimiter(" | ");
			str.Write("RET");
		} else if(opCode == 0) {
			str.Write("NOP");
		}
	} else if(operationType == 2) {
		//Jump
		uint8_t bank = opCode & 0x03;
		uint16_t address = (opCode >> 2) & 0x7FF;
		uint16_t target = (memoryAddr & 0x2000) | (bank << 11) | address;

		switch((opCode >> 13) & 0x1FF) {
			case 0x00: str.Write("JMPSO"); target = 0; break;
			case 0x80: str.Write("JNCA"); break;
			case 0x82: str.Write("JCA"); break;
			case 0x84: str.Write("JNCB"); break;
			case 0x86: str.Write("JCB"); break;
			case 0x88: str.Write("JNZA"); break;
			case 0x8A: str.Write("JZA"); break;
			case 0x8C: str.Write("JNZB"); break;
			case 0x8E: str.Write("JZB"); break;
			case 0x90: str.Write("JNOVA0"); break;
			case 0x92: str.Write("JOVA0"); break;
			case 0x94: str.Write("JNOVB0"); break;
			case 0x96: str.Write("JOVB0"); break;
			case 0x98: str.Write("JNOVA1"); break;
			case 0x9A: str.Write("JOVA1"); break;
			case 0x9C: str.Write("JNOVB1"); break;
			case 0x9E: str.Write("JOVB1"); break;
			case 0xA0: str.Write("JNSA0"); break;
			case 0xA2: str.Write("JSA0"); break;
			case 0xA4: str.Write("JNSB0"); break;
			case 0xA6: str.Write("JSB0"); break;
			case 0xA8: str.Write("JNSA1"); break;
			case 0xAA: str.Write("JSA1"); break;
			case 0xAC: str.Write("JNSB1"); break;
			case 0xAE: str.Write("JSB1"); break;
			case 0xB0: str.Write("JDPL0"); break;
			case 0xB1: str.Write("JDPLN0"); break;
			case 0xB2: str.Write("JDPLF"); break;
			case 0xB3: str.Write("JDPLNF"); break;
			case 0xB4: str.Write("JNSIAK"); break;
			case 0xB6: str.Write("JSIAK"); break;
			case 0xB8: str.Write("JNSOAK"); break;
			case 0xBA: str.Write("JSOAK"); break;
			case 0xBC: str.Write("JNRQM"); break;
			case 0xBE: str.Write("JRQM"); break;
			case 0x100: str.Write("LJMP"); target &= ~0x2000; break;
			case 0x101: str.Write("HJMP"); target |= 0x2000; break;
			case 0x140: str.Write("LCALL"); target &= ~0x2000; break;
			case 0x141: str.Write("HCALL"); target |= 0x2000; break;
			default: str.Write("<unknown jump>"); break;
		}
		str.Write(' ');

		AddressInfo absAddress = { (int32_t)target*3, MemoryType::DspProgramRom };
		string label = labelManager ? labelManager->GetLabel(absAddress) : "";
		if(label.empty()) {
			str.WriteAll('$', HexUtilities::ToHex(target * 3));
		} else {
			str.Write(label, true);
		}
	} else if(operationType == 3) {
		//Load
		uint16_t value = opCode >> 6;
		uint8_t dest = opCode & 0x0F;

		str.Write("LD ");
		str.WriteAll("$", HexUtilities::ToHex(value), ", ");
		str.Write(destNames[dest]);
	}

	out += str.ToString();
}

bool NecDspDisUtils::IsUnconditionalJump(uint32_t opCode)
{
	switch(opCode & 0xC00000) {
		case 0x400000: //RET
			return true;

		case 0x800000: {
			//JMP
			uint16_t jmpType = (opCode >> 13) & 0x1FF;
			switch(jmpType) {
				case 0x00:
				case 0x100:
				case 0x101:
				case 0x140:
				case 0x141:
					return true;

				default:
					return false;
			}
		}

		default:
			return false;
	}
}

bool NecDspDisUtils::IsConditionalJump(uint32_t opCode)
{
	switch(opCode & 0xC00000) {
		case 0x800000: {
			//JMP
			uint16_t jmpType = (opCode >> 13) & 0x1FF;
			switch(jmpType) {
				case 0x00:
				case 0x100:
				case 0x101:
				case 0x140:
				case 0x141:
					return false;

				default:
					return true;
			}
		}

		default:
			return false;
	}
}

bool NecDspDisUtils::IsJumpToSub(uint32_t opCode)
{
	switch(opCode & 0xC00000) {
		case 0x800000: {
			//JMP
			uint16_t jmpType = (opCode >> 13) & 0x1FF;
			switch(jmpType) {
				case 0x140:
				case 0x141:
					return true;

				default:
					return false;
			}
		}

		default:
			return false;
	}
}

bool NecDspDisUtils::IsReturnInstruction(uint32_t opCode)
{
	switch(opCode & 0xC00000) {
		case 0x400000: //RET
			return true;

		default:
			return false;
	}
}
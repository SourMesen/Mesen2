#include "pch.h"
#include "GBA/Debugger/GbaDisUtils.h"
#include "GBA/GbaTypes.h"
#include "GBA/GbaCpu.h"
#include "GBA/Debugger/DummyGbaCpu.h"
#include "GBA/Debugger/GbaCodeDataLogger.h"
#include "GBA/GbaConsole.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/LabelManager.h"
#include "Shared/EmuSettings.h"
#include "Utilities/FastString.h"
#include "Utilities/HexUtilities.h"

void GbaDisUtils::GetDisassembly(DisassemblyInfo& info, string& out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings)
{
	if(info.GetFlags() & GbaCdlFlags::Thumb) {
		ThumbDisassemble(info, out, memoryAddr, labelManager, settings);
	} else {
		ArmDisassemble(info, out, memoryAddr, labelManager, settings);
	}
}

void GbaDisUtils::ThumbDisassemble(DisassemblyInfo& info, string& out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings)
{
	FastString str(settings->GetDebugConfig().UseLowerCaseDisassembly);

	uint16_t opCode = info.GetByteCode()[0] | (info.GetByteCode()[1] << 8);
	GbaThumbOpCategory category = GbaCpu::GetThumbOpCategory(opCode);

	auto writeBranchTarget = [&str, labelManager](uint32_t addr) {
		AddressInfo relAddr = { (int32_t)addr, MemoryType::GbaMemory };
		string label = labelManager ? labelManager->GetLabel(relAddr) : "";
		if(label.empty()) {
			str.WriteAll('$', HexUtilities::ToHex(addr));
		} else {
			str.Write(label, true);
		}
	};

	switch(category) {
		case GbaThumbOpCategory::MoveShiftedRegister:
		{
			uint8_t op = (opCode >> 11) & 0x03;
			uint8_t offset = (opCode >> 6) & 0x1F;
			uint8_t rs = (opCode >> 3) & 0x07;
			uint8_t rd = opCode & 0x07;

			switch(op) {
				case 0: str.Write("LSL"); break;
				case 1: str.Write("LSR"); break;
				case 2: str.Write("ASR"); break;
				default: str.Write("invalid"); break;
			}

			str.Write(' ');
			WriteReg(str, rd);
			str.Write(", ");
			WriteReg(str, rs);
			str.WriteAll(", #$", HexUtilities::ToHex(offset));
			break;
		}

		case GbaThumbOpCategory::AddSubtract:
		{
			bool sub = opCode & (1 << 9);
			bool immediate = opCode &  (1 << 10);
			uint8_t rnOffset = (opCode >> 6) & 0x7;
			uint8_t rs = (opCode >> 3) & 0x07;
			uint8_t rd = opCode & 0x07;
			
			str.Write(sub ? "SUB" : "ADD");
			str.Write(' ');
			WriteReg(str, rd);
			str.Write(", ");
			WriteReg(str, rs);
			if(immediate) {
				str.WriteAll(", #$", HexUtilities::ToHex(rnOffset));
			} else {
				str.Write(", ");
				WriteReg(str, rnOffset);
			}
			break;
		}

		case GbaThumbOpCategory::MoveCmpAddSub:
		{
			uint8_t op = (opCode >> 11) & 0x03;
			uint8_t rd = (opCode >> 8) & 0x07;
			uint8_t offset = opCode & 0xFF;

			switch(op) {
				case 0: str.Write("MOV"); break;
				case 1: str.Write("CMP"); break;
				case 2: str.Write("ADD"); break;
				case 3: str.Write("SUB"); break;
			}
			str.Write(' ');
			WriteReg(str, rd);
			str.WriteAll(", #$", HexUtilities::ToHex(offset));
			break;
		}

		case GbaThumbOpCategory::AluOperation:
		{
			uint8_t op = (opCode >> 6) & 0x0F;
			uint8_t rs = (opCode >> 3) & 0x07;
			uint8_t rd = opCode & 0x07;

			switch(op) {
				case 0: str.Write("AND"); break;
				case 1: str.Write("EOR"); break;
				case 2: str.Write("LSL"); break;
				case 3: str.Write("LSR"); break;
				case 4: str.Write("ASR"); break;
				case 5: str.Write("ADC"); break;
				case 6: str.Write("SBC"); break;
				case 7: str.Write("ROR"); break;
				case 8: str.Write("TST"); break;
				case 9: str.Write("NEG"); break;
				case 10: str.Write("CMP"); break;
				case 11: str.Write("CMN"); break;
				case 12: str.Write("ORR"); break;
				case 13: str.Write("MUL"); break;
				case 14: str.Write("BIC"); break;
				case 15: str.Write("MVN"); break;
			}
			str.Write(' ');
			WriteReg(str, rd);
			str.Write(", ");
			WriteReg(str, rs);
			break;
		}

		case GbaThumbOpCategory::HiRegBranchExch:
		{
			uint8_t op = (opCode >> 8) & 0x03;
			uint8_t rs = ((opCode >> 3) & 0x07) | ((opCode & 0x40) >> 3);
			uint8_t rd = (opCode & 0x07) | ((opCode & 0x80) >> 4);

			switch(op) {
				case 0: str.Write("ADD"); break;
				case 1: str.Write("CMP"); break;
				case 2: str.Write("MOV"); break;
				case 3: str.Write("BX"); break;
			}
			
			str.Write(' ');

			if(op < 3) {
				WriteReg(str, rd);
				str.Write(", ");
				WriteReg(str, rs);
			} else {
				WriteReg(str, rs);
			}
			break;
		}

		case GbaThumbOpCategory::PcRelLoad:
		{
			uint8_t rd = (opCode >> 8) & 0x07;
			uint8_t immValue = opCode & 0xFF;
			
			str.Write("LDR");
			str.Write(' ');
			WriteReg(str, rd);
			str.WriteAll(", [PC, #$", HexUtilities::ToHex(immValue), ']');
			break;
		}

		case GbaThumbOpCategory::LoadStoreRegOffset:
		{
			uint8_t ro = (opCode >> 6) & 0x07;
			uint8_t rb = (opCode >> 3) & 0x07;
			uint8_t rd = opCode & 0x07;
			bool byte = opCode & (1 << 10);
			bool load = opCode & (1 << 11);

			str.Write(load ? "LDR" : "STR");
			if(byte) {
				str.Write('B');
			}

			str.Write(' ');
			WriteReg(str, rd);
			str.Write(", [");
			WriteReg(str, rb);
			str.Write(", ");
			WriteReg(str, ro);
			str.Write(']');
			break;
		}

		case GbaThumbOpCategory::LoadStoreSignExtended:
		{
			uint8_t ro = (opCode >> 6) & 0x07;
			uint8_t rb = (opCode >> 3) & 0x07;
			uint8_t rd = opCode & 0x07;
			uint8_t op = (opCode >> 10) & 0x03;

			switch(op) {
				case 0: str.Write("STRH"); break;
				case 1: str.Write("LDRH"); break;
				case 2: str.Write("LDSB"); break;
				case 3: str.Write("LDSH"); break;
			}
			
			str.Write(' ');
			WriteReg(str, rd);
			str.Write(", [");
			WriteReg(str, rb);
			str.Write(", ");
			WriteReg(str, ro);
			str.Write(']');
			break;
		}

		case GbaThumbOpCategory::LoadStoreImmOffset:
		{
			uint8_t rb = (opCode >> 3) & 0x07;
			uint8_t rd = opCode & 0x07;
			uint8_t offset = (opCode >> 6) & 0x1F;
			bool load = opCode & (1 << 11);
			bool byte = opCode & (1 << 12);

			str.Write(load ? "LDR" : "STR");
			if(byte) {
				str.Write('B');
			}

			str.Write(' ');
			WriteReg(str, rd);
			str.Write(", [");
			WriteReg(str, rb);
			if(byte) {
				str.WriteAll(", #$", HexUtilities::ToHex(offset));
			} else {
				str.WriteAll(", #$", HexUtilities::ToHex(offset << 2));
			}
			str.Write(']');
			break;
		}

		case GbaThumbOpCategory::LoadStoreHalfWord:
		{
			uint8_t rb = (opCode >> 3) & 0x07;
			uint8_t rd = opCode & 0x07;
			uint8_t offset = (opCode >> 6) & 0x1F;
			bool load = opCode & (1 << 11);

			str.Write(load ? "LDRH" : "STRH");

			str.Write(' ');
			WriteReg(str, rd);
			str.Write(", [");
			WriteReg(str, rb);
			str.WriteAll(", #$", HexUtilities::ToHex(offset << 1));
			str.Write(']');
			break;
		}

		case GbaThumbOpCategory::SpRelLoadStore:
		{
			uint8_t rd = (opCode >> 8) & 0x07;
			uint8_t immValue = opCode & 0xFF;
			bool load = opCode & (1 << 11);

			str.Write(load ? "LDR" : "STR");

			str.Write(' ');
			WriteReg(str, rd);
			str.WriteAll(", [SP, #$", HexUtilities::ToHex(immValue << 2), ']');
			break;
		}

		case GbaThumbOpCategory::LoadAddress:
		{
			uint8_t rd = (opCode >> 8) & 0x07;
			uint8_t immValue = opCode & 0xFF;
			bool useSp = opCode & (1 << 11);

			str.Write("ADD");

			str.Write(' ');
			WriteReg(str, rd);
			str.Write(", ");
			str.Write(useSp ? "SP" : "PC");
			str.WriteAll(", #$", HexUtilities::ToHex(immValue << 2));
			break;
		}

		case GbaThumbOpCategory::AddOffsetToSp:
		{
			uint16_t immValue = (opCode & 0x7F) << 2;
			bool sign = opCode & (1 << 7);

			str.Write("ADD SP, #");
			str.WriteAll(sign ? "-" : "", "$", HexUtilities::ToHex(immValue));
			break;
		}

		case GbaThumbOpCategory::PushPopReg:
		{
			uint8_t regMask = opCode & 0xFF;
			bool storeLrLoadPc = opCode & (1 << 8);
			bool load = opCode & (1 << 11);

			str.Write(load ? "POP" : "PUSH");
			str.Write(" {");

			WriteRegList(str, regMask, 8);

			if(storeLrLoadPc) {
				if(regMask) {
					str.Write(',');
				}
				str.Write(load ? "PC" : "LR");
			}
			str.Write("}");
			break;
		}

		case GbaThumbOpCategory::MultipleLoadStore:
		{
			uint8_t regMask = opCode & 0xFF;
			uint8_t rb = (opCode >> 8) & 0x07;
			bool load = opCode & (1 << 11);

			str.Write(load ? "LDMIA" : "STMIA");
			str.Write(' ');
			WriteReg(str, rb);
			str.Write("!, {");
			WriteRegList(str, regMask, 8);
			str.Write('}');
			break;
		}

		case GbaThumbOpCategory::ConditionalBranch:
		{
			int16_t offset = (((int16_t)(int8_t)(opCode & 0xFF)) << 1) + 4;
			uint8_t cond = (opCode >> 8) & 0x0F;

			switch(cond) {
				case 0: str.Write("BEQ"); break;
				case 1: str.Write("BNE"); break;
				case 2: str.Write("BCS"); break;
				case 3: str.Write("BCC"); break;
				case 4: str.Write("BMI"); break;
				case 5: str.Write("BPL"); break;
				case 6: str.Write("BVS"); break;
				case 7: str.Write("BVC"); break;
				case 8: str.Write("BHI"); break;
				case 9: str.Write("BLS"); break;
				case 10: str.Write("BGE"); break;
				case 11: str.Write("BLT"); break;
				case 12: str.Write("BGT"); break;
				case 13: str.Write("BLE"); break;
				default: str.Write("invalid"); break;
			}

			str.Write(' ');
			writeBranchTarget(memoryAddr + offset);
			break;
		}

		case GbaThumbOpCategory::SoftwareInterrupt:
		{
			uint8_t value = opCode & 0xFF;

			str.Write("SWI");
			str.WriteAll(" #$", HexUtilities::ToHex(value));
			break;
		}

		case GbaThumbOpCategory::UnconditionalBranch:
		{
			int16_t offset = ((int16_t)((opCode & 0x7FF) << 5)) >> 4;
			str.Write("BAL ");
			writeBranchTarget(memoryAddr + offset + 4);
			break;
		}

		case GbaThumbOpCategory::LongBranchLink:
		{
			uint16_t offset = opCode & 0x7FF;
			bool high = opCode & (1 << 11);
			str.WriteAll("BL", (high ? "H" : "L"), " #$", HexUtilities::ToHex(offset));
			break;
		}

		case GbaThumbOpCategory::InvalidOp:
		{
			str.Write("INVALID");
			break;
		}
	}

	out += str.ToString();
}

void GbaDisUtils::ArmDisassemble(DisassemblyInfo& info, string& out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings)
{
	FastString str(settings->GetDebugConfig().UseLowerCaseDisassembly);
	uint32_t opCode = info.GetByteCode()[0] | (info.GetByteCode()[1] << 8) | (info.GetByteCode()[2] << 16) | (info.GetByteCode()[3] << 24);

	ArmOpCategory category = GbaCpu::GetArmOpCategory(opCode);

	auto writeBranchTarget = [&str, labelManager](uint32_t addr) {
		AddressInfo relAddr = { (int32_t)addr, MemoryType::GbaMemory };
		string label = labelManager ? labelManager->GetLabel(relAddr) : "";
		if(label.empty()) {
			str.WriteAll('$', HexUtilities::ToHex(addr));
		} else {
			str.Write(label, true);
		}
	};

	switch(category) {
		case ArmOpCategory::Branch:
		{
			str.Write('B');
			if(opCode & (1 << 24)) {
				str.Write('L');
			}
			WriteCond(str, opCode);
			str.Write(' ');
			
			int32_t offset = (((int32_t)opCode << 8) >> 6);
			writeBranchTarget(memoryAddr + offset + 8);
			break;
		}

		case ArmOpCategory::DataProcessing:
		{
			bool immediate = (opCode & (1 << 25)) != 0;
			uint8_t rn = (opCode >> 16) & 0x0F;
			uint8_t rd = (opCode >> 12) & 0x0F;
			bool updateFlags = (opCode & (1 << 20)) != 0;

			ArmAluOperation aluOp = (ArmAluOperation)((opCode >> 21) & 0x0F);
			switch(aluOp) {
				case ArmAluOperation::And: str.Write("AND"); break;
				case ArmAluOperation::Eor: str.Write("EOR"); break;
				case ArmAluOperation::Sub: str.Write("SUB"); break;
				case ArmAluOperation::Rsb: str.Write("RSB"); break;
				case ArmAluOperation::Add: str.Write("ADD"); break;
				case ArmAluOperation::Adc: str.Write("ADC"); break;
				case ArmAluOperation::Sbc: str.Write("SBC"); break;
				case ArmAluOperation::Rsc: str.Write("RSC"); break;
				case ArmAluOperation::Tst: str.Write("TST"); break;
				case ArmAluOperation::Teq: str.Write("TEQ"); break;
				case ArmAluOperation::Cmp: str.Write("CMP"); break;
				case ArmAluOperation::Cmn: str.Write("CMN"); break;
				case ArmAluOperation::Orr: str.Write("ORR"); break;
				case ArmAluOperation::Mov: str.Write("MOV"); break;
				case ArmAluOperation::Bic: str.Write("BIC"); break;
				case ArmAluOperation::Mvn: str.Write("MVN"); break;
			}

			WriteCond(str, opCode);
			bool isCmp = aluOp >= ArmAluOperation::Tst && aluOp <= ArmAluOperation::Cmn;
			if(updateFlags && !isCmp) {
				str.Write('S');
			}

			str.Write(' ');

			if(aluOp == ArmAluOperation::Mov || aluOp == ArmAluOperation::Mvn) {
				WriteReg(str, rd);
			} else if(isCmp) {
				WriteReg(str, rn);
			} else {
				WriteReg(str, rd);
				str.Write(", ");
				WriteReg(str, rn);
			}

			if(immediate) {
				uint8_t shift = ((opCode >> 8) & 0x0F) * 2;
				uint32_t val = (opCode & 0xFF);
				if(shift) {
					val = (val >> shift) | (val << (32 - shift));
				}
				str.WriteAll(", #$", HexUtilities::ToHex(val));
			} else {
				str.Write(", ");
				uint8_t rm = opCode & 0x0F;
				WriteReg(str, rm);
				uint8_t shiftType = (opCode >> 5) & 0x03;

				bool useReg = opCode & (1 << 4);
				uint8_t shift = ((opCode >> 7) & 0x1F);

				if(useReg || shift || shiftType != 0) {
					switch(shiftType) {
						case 0: str.Write(", LSL"); break;
						case 1: str.Write(", LSR"); break;
						case 2: str.Write(", ASR"); break;
						case 3: str.Write(shift == 0 ? ", RRX" : ", ROR"); break;
					}
				}

				if(useReg) {
					//Shift amount in register
					uint8_t rs = (opCode >> 8) & 0x0F;
					str.Write(' ');
					WriteReg(str, rs);
				} else {
					if(shift == 0 && shiftType != 0) {
						if(shiftType != 3) {
							str.WriteAll(" #", std::to_string(32));
						}
					} else {
						if(shift) {
							str.WriteAll(" #", std::to_string(shift));
						}
					}
				}
			}

			break;
		}

		case ArmOpCategory::SingleDataTransfer:
		{
			bool immediate = (opCode & (1 << 25)) == 0;
			bool pre = (opCode & (1 << 24)) != 0;
			bool up = (opCode & (1 << 23)) != 0;
			bool byte = (opCode & (1 << 22)) != 0;
			bool writeBack = (opCode & (1 << 21)) != 0;
			bool load = (opCode & (1 << 20)) != 0;
			uint8_t rn = (opCode >> 16) & 0x0F;
			uint8_t rd = (opCode >> 12) & 0x0F;

			str.Write(load ? "LDR" : "STR");
			WriteCond(str, opCode);
			if(byte) {
				str.Write('B');
			}

			str.Write(' ');
			WriteReg(str, rd);
			str.Write(", ");

			if(immediate) {
				uint16_t offset = opCode & 0xFFF;
				str.Write('[');
				if(offset == 0) {
					WriteReg(str, rn);
					str.Write(']');
				} else if(pre) {
					WriteReg(str, rn);
					str.WriteAll(", ", up ? '+' : '-', "#$", HexUtilities::ToHex(offset), ']');
					if(writeBack) {
						str.Write('!');
					}
				} else {
					WriteReg(str, rn);
					str.WriteAll("], ", up ? '+' : '-', "#$", HexUtilities::ToHex(offset));
				}
			} else {
				str.Write('[');
				WriteReg(str, rn);

				if(!pre) {
					str.Write(']');
				}

				uint8_t rm = opCode & 0x0F;
				str.WriteAll(", ", up ? '+' : '-', 'R', std::to_string(rm));

				uint8_t shiftType = (opCode >> 5) & 0x03;
				uint8_t shift = (opCode >> 7) & 0x1F;

				if(shift) {
					switch(shiftType) {
						case 0: str.Write(", LSL"); break;
						case 1: str.Write(", LSR"); break;
						case 2: str.Write(", ASR"); break;
						case 3: str.Write(", ROR"); break;
					}

					str.WriteAll(" #", std::to_string(shift));
				}

				if(pre) {
					str.Write(']');
					if(writeBack) {
						str.Write('!');
					}
				}
			}
			break;
		}

		case ArmOpCategory::Mrs:
		{
			uint8_t rd = (opCode >> 12) & 0x0F;
			bool psrSrc = opCode & (1 << 22);

			str.Write("MRS");
			WriteCond(str, opCode);
			str.Write(' ');
			WriteReg(str, rd);
			str.WriteAll(", ", psrSrc ? "SPSR" : "CPSR");
			break;
		}

		case ArmOpCategory::Msr:
		{
			bool immediate = opCode & (1 << 25);
			bool psrDst = opCode & (1 << 22);
			uint8_t affectedBytes = (opCode >> 16) & 0x0F;

			str.Write("MSR");
			WriteCond(str, opCode);
			str.Write(' ');

			str.WriteAll(psrDst ? "SPSR_" : "CPSR_");
			if(affectedBytes & 0x08) {
				str.Write('F');
			}
			if(affectedBytes & 0x04) {
				str.Write('S');
			}
			if(affectedBytes & 0x02) {
				str.Write('X');
			}
			if(affectedBytes & 0x01) {
				str.Write('C');
			}

			if(immediate) {
				uint32_t val = (opCode & 0xFF);
				uint8_t shift = ((opCode >> 8) & 0x0F) * 2;
				if(shift) {
					val = (val >> shift) | (val << (32 - shift));
				}
				str.WriteAll(", #$", HexUtilities::ToHex(val));
			} else {
				uint8_t rm = opCode & 0x0F;
				str.Write(", ");
				WriteReg(str, rm);
			}

			break;
		}

		case ArmOpCategory::SingleDataSwap:
		{
			str.Write("SWP");
			WriteCond(str, opCode);

			if(opCode & (1 << 22)) {
				str.Write('B');
			}

			uint8_t rm = opCode & 0x0F;
			uint8_t rd = (opCode >> 12) & 0x0F;
			uint8_t rn = (opCode >> 16) & 0x0F;

			str.Write(' ');
			WriteReg(str, rd);
			str.Write(", ");
			WriteReg(str, rm);
			str.Write(", [");
			WriteReg(str, rn);
			str.Write(']');
			break;
		}

		case ArmOpCategory::BranchExchangeRegister:
		{
			str.Write("BX");
			WriteCond(str, opCode);
			str.Write(' ');
			uint8_t rn = opCode & 0x0F;
			WriteReg(str, rn);
			break;
		}

		case ArmOpCategory::BlockDataTransfer:
		{
			bool pre = (opCode & (1 << 24)) != 0;
			bool up = (opCode & (1 << 23)) != 0;
			bool psrForceUser = (opCode & (1 << 22)) != 0;
			bool writeBack = (opCode & (1 << 21)) != 0;
			bool load = (opCode & (1 << 20)) != 0;
			uint8_t rn = (opCode >> 16) & 0x0F;
			uint16_t regMask = (uint16_t)opCode;

			if(load) {
				str.Write("LDM");
				if(up) {
					str.Write(pre ? "IB" : "IA");
				} else {
					str.Write(pre ? "DB" : "DA");
				}
			} else {
				str.Write("STM");
				if(up) {
					str.Write(pre ? "IB" : "IA");
				} else {
					str.Write(pre ? "DB" : "DA");
				}
			}

			str.Write(' ');
			WriteReg(str, rn);
			if(writeBack) {
				str.Write('!');
			}
			str.Write(", {");
			WriteRegList(str, regMask, 16);
			str.Write('}');

			if(psrForceUser) {
				str.Write('^');
			}
			break;
		}

		case ArmOpCategory::SignedHalfDataTransfer:
		{
			bool pre = (opCode & (1 << 24)) != 0;
			bool up = (opCode & (1 << 23)) != 0;
			bool immediate = (opCode & (1 << 22)) != 0;
			bool writeBack = (opCode & (1 << 21)) != 0;
			bool load = (opCode & (1 << 20)) != 0;
			uint8_t rn = (opCode >> 16) & 0x0F;
			uint8_t rd = (opCode >> 12) & 0x0F;

			bool sign = (opCode & (1 << 6)) != 0;
			bool half = (opCode & (1 << 5)) != 0;

			str.Write(load ? "LDR" : "STR");
			WriteCond(str, opCode);

			if(sign) {
				str.Write('S');
			}
			if(half) {
				str.Write('H');
			} else {
				str.Write('B');
			}

			str.Write(' ');
			WriteReg(str, rd);
			str.Write(", ");

			if(immediate) {
				uint8_t offset = ((opCode >> 4) & 0xF0) | (opCode & 0x0F);
				str.Write('[');
				if(offset == 0) {
					WriteReg(str, rn);
					str.Write(']');
				} else if(pre) {
					WriteReg(str, rn);
					str.WriteAll(", ", up ? '+' : '-', "#$", HexUtilities::ToHex(offset), ']');
					if(writeBack) {
						str.Write('!');
					}
				} else {
					WriteReg(str, rn);
					str.WriteAll("], ", up ? '+' : '-', "#$", HexUtilities::ToHex(offset));
				}
			} else {
				str.Write('[');
				WriteReg(str, rn);

				if(!pre) {
					str.Write(']');
				}

				uint8_t rm = opCode & 0x0F;
				str.WriteAll(", ", up ? '+' : '-', 'R', std::to_string(rm));

				if(pre) {
					str.Write(']');
					if(writeBack) {
						str.Write('!');
					}
				}
			}
			break;
		}

		case ArmOpCategory::Multiply:
		{
			uint8_t rd = (opCode >> 16) & 0x0F;
			uint8_t rn = (opCode >> 12) & 0x0F;
			uint8_t rs = (opCode >> 8) & 0x0F;
			uint8_t rm = opCode & 0x0F;
			bool updateFlags = (opCode & (1 << 20)) != 0;
			bool multAndAcc = (opCode & (1 << 21)) != 0;

			str.Write(multAndAcc ? "MLA" : "MUL");
			WriteCond(str, opCode);
			if(updateFlags) {
				str.Write('S');
			}

			str.Write(' ');
			WriteReg(str, rd);
			str.Write(", ");
			WriteReg(str, rm);
			str.Write(", ");
			WriteReg(str, rs);

			if(multAndAcc) {
				str.Write(", ");
				WriteReg(str, rn);
			}
			break;
		}

		case ArmOpCategory::MultiplyLong:
		{
			uint8_t rh = (opCode >> 16) & 0x0F;
			uint8_t rl = (opCode >> 12) & 0x0F;
			uint8_t rs = (opCode >> 8) & 0x0F;
			uint8_t rm = opCode & 0x0F;
			bool updateFlags = (opCode & (1 << 20)) != 0;
			bool multAndAcc = (opCode & (1 << 21)) != 0;
			bool sign = (opCode & (1 << 22)) != 0;

			str.Write(sign ? 'S' : 'U');
			str.Write(multAndAcc ? "MLAL" : "MULL");
			WriteCond(str, opCode);
			if(updateFlags) {
				str.Write('S');
			}

			str.Write(' ');
			WriteReg(str, rl);
			str.Write(", ");
			WriteReg(str, rh);
			str.Write(", ");
			WriteReg(str, rm);
			str.Write(", ");
			WriteReg(str, rs);
			break;
		}

		case ArmOpCategory::SoftwareInterrupt:
		{
			uint32_t value = opCode & 0xFFFFFF;
			str.Write("SWI");
			str.WriteAll(" #$", HexUtilities::ToHex24(value));
			break;
		}

		default:
			str.Write("INVALID");
			break;
	}

	out += str.ToString();
}

void GbaDisUtils::WriteRegList(FastString& str, uint16_t regMask, uint8_t size)
{
	bool firstReg = true;
	int8_t rangeStart = -1;
	for(int i = 0; i < size + 1; i++) {
		if(regMask & (1 << i)) {
			if(rangeStart == -1) {
				if(!firstReg) {
					str.Write(',');
				}
				firstReg = false;
				WriteReg(str, i);
				rangeStart = i;
			}
		} else {
			if(rangeStart >= 0) {
				if(i - rangeStart > 1) {
					str.Write('-');
					WriteReg(str, i - 1);
				}
			}
			rangeStart = -1;
		}
	}
}

void GbaDisUtils::WriteReg(FastString& str, uint8_t reg)
{
	switch(reg) {
		case 13: str.Write("SP"); break;
		case 14: str.Write("LR"); break;
		case 15: str.Write("PC"); break;
		default:	str.WriteAll('R', std::to_string(reg)); break;
	}
}

void GbaDisUtils::WriteCond(FastString& str, uint32_t opCode)
{
	switch(opCode >> 28) {
		default: case 0: str.Write("EQ"); break;
		case 1: str.Write("NE"); break;
		case 2: str.Write("CS"); break;
		case 3: str.Write("CC"); break;
		case 4: str.Write("MI"); break;
		case 5: str.Write("PL"); break;
		case 6: str.Write("VS"); break;
		case 7: str.Write("VC"); break;
		case 8: str.Write("HI"); break;
		case 9: str.Write("LS"); break;
		case 10: str.Write("GE"); break;
		case 11: str.Write("LT"); break;
		case 12: str.Write("GT"); break;
		case 13: str.Write("LE"); break;
		case 14: str.Write(""); break;
		case 15: str.Write("--"); break;
	}
}

EffectiveAddressInfo GbaDisUtils::GetEffectiveAddress(DisassemblyInfo& info, GbaConsole* console, GbaCpuState& state)
{
	bool thumb = state.CPSR.Thumb;
	uint32_t opCode = console->GetMemoryManager()->DebugCpuRead(thumb ? GbaAccessMode::HalfWord : GbaAccessMode::Word, state.Pipeline.Execute.Address);
	state.Pipeline.Execute.OpCode = opCode;

	if(thumb) {
		GbaThumbOpCategory category = GbaCpu::GetThumbOpCategory(opCode);
		switch(category) {
			case GbaThumbOpCategory::HiRegBranchExch: {
				uint8_t op = (opCode >> 8) & 0x03;
				if(op == 3) {
					uint8_t rs = ((opCode >> 3) & 0x07) | ((opCode & 0x40) >> 3);
					return { (int)state.R[rs], 0, true, MemoryType::GbaMemory };
				}
				break;
			}

			case GbaThumbOpCategory::LongBranchLink: {
				bool high = opCode & (1 << 11);
				if(high) {
					uint32_t opCodeLow = console->GetMemoryManager()->DebugCpuRead(GbaAccessMode::HalfWord, state.Pipeline.Execute.Address - 2);
					uint16_t offsetLow = opCodeLow & 0x7FF;
					uint16_t offsetHigh = opCode & 0x7FF;
					int32_t relOffset = ((int32_t)offsetLow << 21) >> 9;
					uint32_t r14 = state.R[15] - 2 + relOffset;
					uint32_t addr = r14 + (offsetHigh << 1);
					return { (int)addr, 0, true, MemoryType::GbaMemory };
				}
				return {};
			}

			case GbaThumbOpCategory::InvalidOp:
			case GbaThumbOpCategory::MultipleLoadStore:
			case GbaThumbOpCategory::PushPopReg:
				return {};
		}
	} else {
		ArmOpCategory category = GbaCpu::GetArmOpCategory(opCode);
		switch(category) {
			case ArmOpCategory::BranchExchangeRegister: {
				uint8_t rs = opCode & 0x0F;
				return { (int)state.R[rs], 0, true, MemoryType::GbaMemory };
			}

			case ArmOpCategory::InvalidOp:
			case ArmOpCategory::BlockDataTransfer:
				return {};
		}
	}

	DummyGbaCpu dummyCpu;
	dummyCpu.Init(console->GetEmulator(), console->GetMemoryManager(), nullptr);
	dummyCpu.SetDummyState(state);
	dummyCpu.Exec<false, false>();

	uint32_t count = dummyCpu.GetOperationCount();
	for(int i = count - 1; i >= 0; i--) {
		MemoryOperationInfo opInfo = dummyCpu.GetOperationInfo(i);
		if(opInfo.Type != MemoryOperationType::ExecOperand) {
			EffectiveAddressInfo result;
			result.Address = opInfo.Address;
			result.Type = opInfo.MemType;
			result.ValueSize = 1;

			switch(dummyCpu.GetOperationMode(i) & (GbaAccessMode::Byte | GbaAccessMode::HalfWord | GbaAccessMode::Word)) {
				case GbaAccessMode::Byte: result.ValueSize = 1; break;
				case GbaAccessMode::HalfWord: result.ValueSize = 2; break;
				case GbaAccessMode::Word: result.ValueSize = 4; break;
			}

			result.ShowAddress = true;
			return result;
		}
	}

	return {};
}

uint8_t GbaDisUtils::GetOpSize(uint32_t opCode, uint8_t flags)
{
	return IsThumbMode(flags) ? 2 : 4;
}

bool GbaDisUtils::IsJumpToSub(uint32_t opCode, uint8_t flags)
{
	if(IsThumbMode(flags)) {
		GbaThumbOpCategory category = GbaCpu::GetThumbOpCategory(opCode);
		if(category == GbaThumbOpCategory::LongBranchLink) {
			bool high = opCode & (1 << 11);
			return high;
		}
		return false;
	} else {
		ArmOpCategory category = GbaCpu::GetArmOpCategory(opCode);
		if(category == ArmOpCategory::Branch) {
			bool withLink = opCode & (1 << 24);
			return withLink;
		}
		return false;
	}
}

bool GbaDisUtils::IsReturnInstruction(uint32_t opCode, uint8_t flags)
{
	//For GBA, this is only used to show markers at the end of functions 
	//Callstack uses a different logic in GbaDebugger itself
	if(IsThumbMode(flags)) {
		GbaThumbOpCategory category = GbaCpu::GetThumbOpCategory(opCode);
		if(category == GbaThumbOpCategory::HiRegBranchExch) {
			uint8_t op = (opCode >> 8) & 0x03;
			if(op == 3) {
				uint8_t rs = ((opCode >> 3) & 0x07) | ((opCode & 0x40) >> 3);
				//BX 14
				return rs == 14;
			}
		}
		return false;
	} else {
		ArmOpCategory category = GbaCpu::GetArmOpCategory(opCode);
		if(category == ArmOpCategory::BranchExchangeRegister) {
			//BX 14
			uint8_t rs = opCode & 0x0F;
			return rs == 14;
		}
		return false;
	}
}

bool GbaDisUtils::IsUnconditionalJump(uint32_t opCode, uint8_t flags)
{
	if(IsThumbMode(flags)) {
		GbaThumbOpCategory category = GbaCpu::GetThumbOpCategory(opCode);
		switch(category) {
			case GbaThumbOpCategory::UnconditionalBranch:
			case GbaThumbOpCategory::SoftwareInterrupt:
				return true;

			case GbaThumbOpCategory::PushPopReg: {
				bool storeLrLoadPc = opCode & (1 << 8);
				bool load = opCode & (1 << 11);
				return storeLrLoadPc && load;
			}

			case GbaThumbOpCategory::MultipleLoadStore: {
				uint16_t regMask = (uint8_t)opCode;
				if(regMask == 0) {
					bool load = opCode & (1 << 11);
					return load;
				}
				return false;
			}

			case GbaThumbOpCategory::HiRegBranchExch: {
				uint8_t op = (opCode >> 8) & 0x03;
				uint8_t rd = (opCode & 0x07) | ((opCode & 0x80) >> 4);
				return op == 3 || ((op == 0 || op == 2) && rd == 15);
			}

			case GbaThumbOpCategory::LongBranchLink: {
				bool high = opCode & (1 << 11);
				return high;
			}
		}
		return false;
	} else {
		bool isUnconditional = ((opCode >> 28) & 0x0F) == 0x0E;
		if(!isUnconditional) {
			return false;
		}
		return IsArmBranch(opCode);
	}
}

bool GbaDisUtils::IsArmBranch(uint32_t opCode)
{
	ArmOpCategory category = GbaCpu::GetArmOpCategory(opCode);
	switch(category) {
		case ArmOpCategory::Branch: return true;
		case ArmOpCategory::BranchExchangeRegister: return true;
		case ArmOpCategory::SoftwareInterrupt: return true;

		case ArmOpCategory::Msr: return false;

		case ArmOpCategory::Mrs:
		case ArmOpCategory::DataProcessing:
		case ArmOpCategory::Multiply:
		case ArmOpCategory::SingleDataSwap: {
			uint8_t rd = (opCode >> 12) & 0x0F;
			return rd == 15;
		}

		case ArmOpCategory::SingleDataTransfer:
		case ArmOpCategory::SignedHalfDataTransfer: {
			uint8_t rd = (opCode >> 12) & 0x0F;
			if(rd == 15) {
				bool pre = (opCode & (1 << 24));
				bool writeBack = (opCode & (1 << 21));
				bool load = (opCode & (1 << 20));
				uint8_t rn = (opCode >> 16) & 0x0F;
				if((rd != rn || !load) && (writeBack || !pre)) {
					return true;
				}
			}
			return false;
		}

		case ArmOpCategory::MultiplyLong: {
			uint8_t rl = (opCode >> 12) & 0x0F;
			uint8_t rh = (opCode >> 16) & 0x0F;
			return rl == 15 || rh == 15;
		}

		case ArmOpCategory::BlockDataTransfer: {
			uint8_t rn = (opCode >> 16) & 0x0F;
			bool load = (opCode & (1 << 20));
			if(rn == 15) {
				bool writeBack = (opCode & (1 << 21));
				if(writeBack || load) {
					return true;
				}
			}

			uint16_t regMask = (uint16_t)opCode;
			if((regMask & 0x8000) || regMask == 0) {
				return load;
			}

			return false;
		}
	}

	return false;
}

bool GbaDisUtils::IsConditionalJump(uint32_t opCode, uint8_t flags)
{
	if(IsThumbMode(flags)) {
		GbaThumbOpCategory category = GbaCpu::GetThumbOpCategory(opCode);
		return category == GbaThumbOpCategory::ConditionalBranch;
	} else {
		bool isConditional = ((opCode >> 28) & 0x0F) <= 0x0D;
		return isConditional && IsArmBranch(opCode);
	}
}

CdlFlags::CdlFlags GbaDisUtils::GetOpFlags(uint32_t opCode, uint8_t flags, uint32_t pc, uint32_t prevPc)
{
	if(pc == prevPc + GbaDisUtils::GetOpSize(opCode, flags)) {
		return CdlFlags::None;
	}

	if(IsJumpToSub(opCode, flags)) {
		return CdlFlags::SubEntryPoint;
	} else {
		return CdlFlags::JumpTarget;
	}
}

bool GbaDisUtils::IsThumbMode(uint8_t flags)
{
	return flags & GbaCdlFlags::Thumb;
}

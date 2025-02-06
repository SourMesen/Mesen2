#include "pch.h"
#include <regex>
#include <unordered_map>
#include "Utilities/HexUtilities.h"
#include "Utilities/StringUtilities.h"
#include "SNES/SnesCpu.h"
#include "SNES/Debugger/SnesAssembler.h"
#include "SNES/Debugger/SnesDisUtils.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/LabelManager.h"

SnesAssembler::SnesAssembler(LabelManager* labelManager) : Base6502Assembler<SnesAddrMode>(labelManager, CpuType::Snes)
{
}

string SnesAssembler::GetOpName(uint8_t opcode)
{
	return SnesDisUtils::OpName[opcode];
}

SnesAddrMode SnesAssembler::GetOpMode(uint8_t opcode)
{
	return SnesDisUtils::OpMode[opcode];
}

bool SnesAssembler::IsOfficialOp(uint8_t opcode)
{
	return true;
}

AssemblerSpecialCodes SnesAssembler::ResolveOpMode(AssemblerLineData& op, uint32_t instructionAddress, bool firstPass)
{
	AssemblerOperand& operand = op.Operands[0];
	AssemblerOperand& operand2 = op.Operands[1];
	AssemblerOperand& operand3 = op.Operands[2];
	if(operand.ByteCount > 3 || operand2.ByteCount > 1 || operand3.ByteCount > 0) {
		return AssemblerSpecialCodes::InvalidOperands;
	}

	if(op.OperandCount == 3) {
		//StkRelIndIdxY
		if(operand.HasOpeningParenthesis && operand.ByteCount == 1 && operand2.HasClosingParenthesis && operand2.Type == OperandType::S && operand3.Type == OperandType::Y) {
			op.AddrMode = SnesAddrMode::StkRelIndIdxY;
		} else {
			return AssemblerSpecialCodes::InvalidOperands;
		}
	} else if(operand.Type == OperandType::Custom && operand2.Type == OperandType::Custom && op.OperandCount == 2) {
		//BlkMov (MVP/MVN)
		if(operand.HasParenOrBracket() || operand2.HasParenOrBracket() || operand.ByteCount != 1 || operand2.ByteCount != 1) {
			return AssemblerSpecialCodes::InvalidOperands;
		}
		//Invert operand order
		AssemblerOperand orgOperand = operand;
		operand = operand2;
		operand2 = orgOperand;
		op.AddrMode = SnesAddrMode::BlkMov;
	} else if(operand.IsImmediate) {
		//Imm16, Imm8, Rel
		if(operand.HasParenOrBracket() || operand.ByteCount > 2 || op.OperandCount > 1) {
			return AssemblerSpecialCodes::ParsingError;
		}
		if(IsOpModeAvailable(op.OpCode, SnesAddrMode::Rel)) {
			op.AddrMode = SnesAddrMode::Rel;
		} else {
			if(IsOpModeAvailable(op.OpCode, SnesAddrMode::ImmM)) {
				op.AddrMode = SnesAddrMode::ImmM;
			} else if(IsOpModeAvailable(op.OpCode, SnesAddrMode::ImmX)) {
				op.AddrMode = SnesAddrMode::ImmX;
			} else {
				op.AddrMode = operand.ByteCount == 2 ? SnesAddrMode::Imm16 : SnesAddrMode::Imm8;
			}
		}
	} else if(operand.HasOpeningBracket) {
		//DirIndLng, DirIndLngIdxY, AbsIndLng
		if(operand.HasClosingBracket) {
			if(operand2.Type == OperandType::Y) {
				op.AddrMode = SnesAddrMode::DirIndLngIdxY;
			} else {
				if(op.OperandCount > 1) {
					return AssemblerSpecialCodes::ParsingError;
				}

				if(operand.ByteCount == 1) {
					AdjustOperandSize(op, operand, SnesAddrMode::DirIndLng, SnesAddrMode::AbsIndLng);
				} else {
					op.AddrMode = SnesAddrMode::AbsIndLng;
				}
			}
		} else {
			return AssemblerSpecialCodes::ParsingError;
		}
	} else if(operand.HasOpeningParenthesis) {
		//DirInd, AbsInd, DirIdxIndX, AbsIdxXInd, DirIndIdxY
		if(operand.HasClosingParenthesis) {
			if(operand2.Type == OperandType::Y) {
				op.AddrMode = SnesAddrMode::DirIndIdxY;
			} else {
				if(op.OperandCount > 1) {
					return AssemblerSpecialCodes::ParsingError;
				}

				if(operand.ByteCount == 1) {
					AdjustOperandSize(op, operand, SnesAddrMode::DirInd, SnesAddrMode::AbsInd);
				} else {
					op.AddrMode = SnesAddrMode::AbsInd;
				}
			}
		} else if(operand2.Type == OperandType::X && operand2.HasClosingParenthesis) {
			if(operand.ByteCount == 1) {
				AdjustOperandSize(op, operand, SnesAddrMode::DirIdxIndX, SnesAddrMode::AbsIdxXInd);
			} else {
				op.AddrMode = SnesAddrMode::AbsIdxXInd;
			}
		} else {
			return AssemblerSpecialCodes::ParsingError;
		}
	} else if(operand.HasParenOrBracket() || operand2.HasParenOrBracket()) {
		return AssemblerSpecialCodes::ParsingError;
	} else if(op.OperandCount == 2) {
		if(operand2.Type == OperandType::X) {
			switch(operand.ByteCount) {
				case 1: AdjustOperandSize(op, operand, SnesAddrMode::DirIdxX, SnesAddrMode::AbsIdxX); break;
				case 2: op.AddrMode = SnesAddrMode::AbsIdxX; break;
				case 3: op.AddrMode = SnesAddrMode::AbsLngIdxX; break;
				default: return AssemblerSpecialCodes::ParsingError;
			}
		} else if(operand2.Type == OperandType::Y) {
			switch(operand.ByteCount) {
				case 1: AdjustOperandSize(op, operand, SnesAddrMode::DirIdxY, SnesAddrMode::AbsIdxY); break;
				case 2: op.AddrMode = SnesAddrMode::AbsIdxY; break;
				default: return AssemblerSpecialCodes::ParsingError;
			}
		} else if(operand2.Type == OperandType::S && operand.ByteCount == 1) {
			op.AddrMode = SnesAddrMode::StkRel;
		} else {
			return AssemblerSpecialCodes::ParsingError;
		}
	} else if(op.OperandCount == 1) {
		if(IsOpModeAvailable(op.OpCode, SnesAddrMode::Rel) || IsOpModeAvailable(op.OpCode, SnesAddrMode::RelLng)) {
			//Convert "absolute" jump to a relative jump
			int32_t addressGap = operand.Value - (instructionAddress + 2);
			bool lngBranch = (addressGap < -128 || addressGap > 127) && IsOpModeAvailable(op.OpCode, SnesAddrMode::RelLng);
			int16_t maxPosGap = lngBranch ? 32767 : 127;
			int16_t maxNegGap = lngBranch ? -32768 : -128;
			if(addressGap > maxPosGap || addressGap < maxNegGap) {
				//Gap too long, can't jump that far
				if(!firstPass) {
					//Pretend this is ok on first pass, we're just trying to find all labels
					return AssemblerSpecialCodes::OutOfRangeJump;
				}
			}

			//Update data to match relative jump
			operand.ByteCount = lngBranch ? 2 : 1;
			operand.Value = lngBranch ? (uint16_t)addressGap : (uint8_t)addressGap;
			op.AddrMode = lngBranch ? SnesAddrMode::RelLng : SnesAddrMode::Rel;
		} else if(operand.ByteCount == 3) {
			op.AddrMode = SnesAddrMode::AbsLng;
		} else if(operand.ByteCount == 2) {
			op.AddrMode = SnesAddrMode::Abs;
		} else if(operand.ByteCount == 1) {
			//Sometimes direct addressing is not available, even if the operand is in the direct page
			AdjustOperandSize(op, operand, SnesAddrMode::Dir, SnesAddrMode::Abs);
		} else {
			return AssemblerSpecialCodes::ParsingError;
		}
	} else if(op.OperandCount == 0) {
		op.AddrMode = IsOpModeAvailable(op.OpCode, SnesAddrMode::Acc) ? SnesAddrMode::Acc : (IsOpModeAvailable(op.OpCode, SnesAddrMode::Stk) ? SnesAddrMode::Stk : SnesAddrMode::Imp);
	} else {
		return AssemblerSpecialCodes::ParsingError;
	}

	return AssemblerSpecialCodes::OK;
}
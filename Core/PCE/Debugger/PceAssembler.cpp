#include "pch.h"
#include <regex>
#include <unordered_map>
#include "Utilities/HexUtilities.h"
#include "Utilities/StringUtilities.h"
#include "PCE/Debugger/PceAssembler.h"
#include "PCE/Debugger/PceDisUtils.h"
#include "Debugger/LabelManager.h"

PceAssembler::PceAssembler(LabelManager* labelManager) : Base6502Assembler<PceAddrMode>(labelManager, CpuType::Pce)
{
}

string PceAssembler::GetOpName(uint8_t opcode)
{
	return PceDisUtils::GetOpName(opcode);
}

PceAddrMode PceAssembler::GetOpMode(uint8_t opcode)
{
	return PceDisUtils::GetOpMode(opcode);
}

bool PceAssembler::IsOfficialOp(uint8_t opcode)
{
	//Not implemented, but not necessary for PCE assembler
	return true;
}

void PceAssembler::AdjustLabelOperand(AssemblerOperand& operand)
{
	if(operand.ValueType == OperandValueType::Label) {
		if(operand.ByteCount == 1) {
			operand.ByteCount = 2;
		} else if(operand.ByteCount == 2 && operand.Value >= 0x2000 && operand.Value <= 0x20FF) {
			operand.ByteCount = 1;
		}
	}
}

void PceAssembler::AdjustLabelOperands(AssemblerLineData& op)
{
	AdjustLabelOperand(op.Operands[0]);
	AdjustLabelOperand(op.Operands[1]);
	AdjustLabelOperand(op.Operands[2]);
}

AssemblerSpecialCodes PceAssembler::ResolveOpMode(AssemblerLineData& op, uint32_t instructionAddress, bool firstPass)
{
	AssemblerOperand& operand = op.Operands[0];
	AssemblerOperand& operand2 = op.Operands[1];
	AssemblerOperand& operand3 = op.Operands[2];
	if(operand.ByteCount > 2 || operand2.ByteCount > 2 || operand3.ByteCount > 2) {
		return AssemblerSpecialCodes::InvalidOperands;
	}

	AdjustLabelOperands(op);

	if(operand3.Type != OperandType::None) {
		if(operand3.ByteCount > 0) {
			//Block transfer
			if(operand.ByteCount >= 1 && operand.ByteCount <= 2 && operand2.ByteCount >= 1 && operand2.ByteCount <= 2 && operand3.ByteCount >= 1 && operand3.ByteCount <= 2) {
				if(operand.IsImmediate || operand.HasParenOrBracket() || operand2.IsImmediate || operand2.HasParenOrBracket() || !operand3.IsImmediate || operand3.HasParenOrBracket()) {
					return AssemblerSpecialCodes::InvalidOperands;
				} else {
					operand.ByteCount = 2;
					operand2.ByteCount = 2;
					operand3.ByteCount = 2;
					op.AddrMode = PceAddrMode::Block;
				}
			} else {
				return AssemblerSpecialCodes::InvalidOperands;
			}
		} else {
			//ImZeroX, ImAbsX
			if(!operand.IsImmediate || operand.ByteCount != 1 || operand3.Type != OperandType::X) {
				return AssemblerSpecialCodes::InvalidOperands;
			}

			if(operand2.ByteCount == 1) {
				op.AddrMode = PceAddrMode::ImZeroX;
			} else if(operand2.ByteCount == 2) {
				op.AddrMode = PceAddrMode::ImAbsX;
			} else {
				return AssemblerSpecialCodes::InvalidOperands;
			}
		}
	} else if(operand2.Type == OperandType::Custom) {
		//ImZero, ImAbs, ZeroRel
		if(!operand.IsImmediate && operand.ByteCount == 1) {
			if(operand2.ByteCount == 2 || operand2.ValueType == OperandValueType::Label) {
				int16_t addressGap = operand2.Value - (instructionAddress + 3);
				if(addressGap > 127 || addressGap < -128) {
					//Gap too long, can't jump that far
					if(!firstPass) {
						//Pretend this is ok on first pass, we're just trying to find all labels
						return AssemblerSpecialCodes::OutOfRangeJump;
					}
				}

				//Update data to match relative jump
				operand2.ByteCount = 1;
				operand2.Value = (uint8_t)addressGap;
				op.AddrMode = PceAddrMode::ZeroRel;
			} else if(operand2.ByteCount == 1) {
				op.AddrMode = PceAddrMode::ZeroRel;
			} else {
				return AssemblerSpecialCodes::InvalidOperands;
			}
		} else {
			if(!operand.IsImmediate || operand.ByteCount != 1) {
				return AssemblerSpecialCodes::InvalidOperands;
			}

			if(operand2.ByteCount == 1) {
				op.AddrMode = PceAddrMode::ImZero;
			} else if(operand2.ByteCount == 2) {
				op.AddrMode = PceAddrMode::ImAbs;
			} else {
				return AssemblerSpecialCodes::InvalidOperands;
			}
		}
	} else if(operand.IsImmediate) {
		if(operand.HasOpeningParenthesis || operand.ByteCount == 0 || op.OperandCount > 1) {
			return AssemblerSpecialCodes::InvalidOperands;
		} else if(operand.ByteCount > 1) {
			//Can't be a 2-byte operand
			return AssemblerSpecialCodes::InvalidOperands;
		}
		op.AddrMode = IsOpModeAvailable(op.OpCode, PceAddrMode::Rel) ? PceAddrMode::Rel : PceAddrMode::Imm;
	} else if(operand.HasOpeningParenthesis) {
		//Ind, IndX, IndY, AbsXInd, ZInd
		if(operand2.Type == OperandType::X && operand2.HasClosingParenthesis) {
			if(operand.ByteCount == 2) {
				op.AddrMode = PceAddrMode::AbsXInd;
			} else if(operand.ByteCount == 1) {
				op.AddrMode = PceAddrMode::IndX;
			} else {
				return AssemblerSpecialCodes::InvalidOperands;
			}
		} else if(operand.HasClosingParenthesis && operand2.Type == OperandType::Y) {
			op.AddrMode = PceAddrMode::IndY;
		} else if(operand.HasClosingParenthesis) {
			if(operand.ByteCount == 2) {
				op.AddrMode = PceAddrMode::Ind;
			} else if(operand.ByteCount == 1) {
				op.AddrMode = PceAddrMode::ZInd;
			} else {
				return AssemblerSpecialCodes::InvalidOperands;
			}
		} else {
			return AssemblerSpecialCodes::InvalidOperands;
		}
	} else if(operand.HasParenOrBracket() || operand2.HasParenOrBracket()) {
		return AssemblerSpecialCodes::ParsingError;
	} else {
		if(operand2.Type == OperandType::X) {
			if(operand.ByteCount == 2) {
				op.AddrMode = PceAddrMode::AbsX;
			} else if(operand.ByteCount == 1) {
				//Sometimes zero page addressing is not available, even if the operand is in the zero page
				AdjustOperandSize(op, operand, PceAddrMode::ZeroX, PceAddrMode::AbsX);
			} else {
				return AssemblerSpecialCodes::InvalidOperands;
			}
		} else if(operand2.Type == OperandType::Y) {
			if(operand.ByteCount == 2) {
				op.AddrMode = PceAddrMode::AbsY;
			} else if(operand.ByteCount == 1) {
				//Sometimes zero page addressing is not available, even if the operand is in the zero page
				AdjustOperandSize(op, operand, PceAddrMode::ZeroY, PceAddrMode::AbsY);
			} else {
				return AssemblerSpecialCodes::InvalidOperands;
			}
		} else if(operand.Type == OperandType::A) {
			op.AddrMode = PceAddrMode::Acc;
		} else if(op.OperandCount == 0) {
			op.AddrMode = IsOpModeAvailable(op.OpCode, PceAddrMode::Acc) ? PceAddrMode::Acc : PceAddrMode::Imp;
		} else if(op.OperandCount == 1) {
			bool allowRelMode = IsOpModeAvailable(op.OpCode, PceAddrMode::Rel);
			if(allowRelMode) {
				op.AddrMode = PceAddrMode::Rel;

				//Convert "absolute" jump to a relative jump
				int16_t addressGap = operand.Value - (instructionAddress + 2);
				if(addressGap > 127 || addressGap < -128) {
					//Gap too long, can't jump that far
					if(!firstPass) {
						//Pretend this is ok on first pass, we're just trying to find all labels
						return AssemblerSpecialCodes::OutOfRangeJump;
					}
				}

				//Update data to match relative jump
				operand.ByteCount = 1;
				operand.Value = (uint8_t)addressGap;
			} else if(operand.ByteCount == 2) {
				op.AddrMode = PceAddrMode::Abs;
				operand.ByteCount = 2;
			} else if(operand.ByteCount == 1) {
				//Sometimes zero page addressing is not available, even if the operand is in the zero page
				AdjustOperandSize(op, operand, PceAddrMode::Zero, PceAddrMode::Abs);
			} else {
				return AssemblerSpecialCodes::InvalidOperands;
			}
		} else {
			return AssemblerSpecialCodes::InvalidOperands;
		}
	}

	return AssemblerSpecialCodes::OK;
}
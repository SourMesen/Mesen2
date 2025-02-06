#include "pch.h"
#include <regex>
#include <unordered_map>
#include "Utilities/HexUtilities.h"
#include "Utilities/StringUtilities.h"
#include "NES/Debugger/NesAssembler.h"
#include "NES/Debugger/NesDisUtils.h"
#include "Debugger/LabelManager.h"

NesAssembler::NesAssembler(LabelManager* labelManager) : Base6502Assembler<NesAddrMode>(labelManager, CpuType::Nes)
{
}

string NesAssembler::GetOpName(uint8_t opcode)
{
	return NesDisUtils::GetOpName(opcode);
}

bool NesAssembler::IsOfficialOp(uint8_t opcode)
{
	return !NesDisUtils::IsOpUnofficial(opcode);
}

NesAddrMode NesAssembler::GetOpMode(uint8_t opcode)
{
	NesAddrMode mode = NesDisUtils::GetOpMode(opcode);
	switch(mode) {
		case NesAddrMode::AbsXW: return NesAddrMode::AbsX;
		case NesAddrMode::AbsYW: return NesAddrMode::AbsY;
		case NesAddrMode::IndYW: return NesAddrMode::IndY;
		default: return mode;
	}
}

AssemblerSpecialCodes NesAssembler::ResolveOpMode(AssemblerLineData& op, uint32_t instructionAddress, bool firstPass)
{
	if(op.OperandCount > 2) {
		return AssemblerSpecialCodes::InvalidOperands;
	} else if(op.OperandCount == 2 && op.Operands[1].Type != OperandType::X && op.Operands[1].Type != OperandType::Y) {
		return AssemblerSpecialCodes::InvalidOperands;
	}

	AssemblerOperand& operand = op.Operands[0];
	AssemblerOperand& operand2 = op.Operands[1];
	if(operand.ByteCount > 2 || operand2.ByteCount > 0) {
		return AssemblerSpecialCodes::InvalidOperands;
	}

	if(operand.IsImmediate) {
		if(operand.HasParenOrBracket() || operand.ByteCount == 0 || op.OperandCount > 1) {
			return AssemblerSpecialCodes::ParsingError;
		} else if(operand.ByteCount > 1) {
			//Can't be a 2-byte operand
			return AssemblerSpecialCodes::ParsingError;
		}
		op.AddrMode = IsOpModeAvailable(op.OpCode, NesAddrMode::Rel) ? NesAddrMode::Rel : NesAddrMode::Imm;
	} else if(operand.HasOpeningParenthesis) {
		if(operand2.Type == OperandType::X && operand2.HasClosingParenthesis && operand.ByteCount == 1) {
			op.AddrMode = NesAddrMode::IndX;
		} else if(operand.HasClosingParenthesis && operand2.Type == OperandType::Y && operand.ByteCount == 1) {
			op.AddrMode = NesAddrMode::IndY;
		} else if(operand.HasClosingParenthesis && operand.ByteCount > 0) {
			op.AddrMode = NesAddrMode::Ind;
			operand.ByteCount = 2;
		} else {
			return AssemblerSpecialCodes::ParsingError;
		}
	} else if(operand.HasParenOrBracket() || operand2.HasParenOrBracket()) {
		return AssemblerSpecialCodes::ParsingError;
	} else {
		if(operand2.Type == OperandType::X) {
			if(operand.ByteCount == 2) {
				op.AddrMode = NesAddrMode::AbsX;
			} else if(operand.ByteCount == 1) {
				//Sometimes zero page addressing is not available, even if the operand is in the zero page
				AdjustOperandSize(op, operand, NesAddrMode::ZeroX, NesAddrMode::AbsX);
			} else {
				return AssemblerSpecialCodes::ParsingError;
			}
		} else if(operand2.Type == OperandType::Y) {
			if(operand.ByteCount == 2) {
				op.AddrMode = NesAddrMode::AbsY;
			} else if(operand.ByteCount == 1) {
				//Sometimes zero page addressing is not available, even if the operand is in the zero page
				AdjustOperandSize(op, operand, NesAddrMode::ZeroY, NesAddrMode::AbsY);
			} else {
				return AssemblerSpecialCodes::ParsingError;
			}
		} else if(operand.Type == OperandType::A) {
			op.AddrMode = NesAddrMode::Acc;
		} else if(op.OperandCount == 0) {
			op.AddrMode = IsOpModeAvailable(op.OpCode, NesAddrMode::Acc) ? NesAddrMode::Acc : NesAddrMode::Imp;
		} else if(op.OperandCount == 1) {
			if(IsOpModeAvailable(op.OpCode, NesAddrMode::Rel)) {
				op.AddrMode = NesAddrMode::Rel;

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
				op.AddrMode = NesAddrMode::Abs;
			} else if(operand.ByteCount == 1) {
				//Sometimes zero page addressing is not available, even if the operand is in the zero page
				AdjustOperandSize(op, operand, NesAddrMode::Zero, NesAddrMode::Abs);
			} else {
				return AssemblerSpecialCodes::ParsingError;
			}
		} else {
			return AssemblerSpecialCodes::ParsingError;
		}
	}

	return AssemblerSpecialCodes::OK;
}

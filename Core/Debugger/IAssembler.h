#pragma once
#include "pch.h"

class IAssembler
{
public:
	virtual ~IAssembler() {}
	virtual uint32_t AssembleCode(string code, uint32_t startAddress, int16_t* assembledCode) = 0;
};

enum AssemblerSpecialCodes
{
	OK = 0,
	EndOfLine = -1,
	ParsingError = -2,
	OutOfRangeJump = -3,
	LabelRedefinition = -4,
	MissingOperand = -5,
	OperandOutOfRange = -6,
	InvalidHex = -7,
	InvalidSpaces = -8,
	TrailingText = -9,
	UnknownLabel = -10,
	InvalidInstruction = -11,
	InvalidBinaryValue = -12,
	InvalidOperands = -13,
	InvalidLabel = -14,
};

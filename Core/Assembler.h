#pragma once
#include "stdafx.h"
#include <unordered_set>
#include <regex>
#include "CpuDisUtils.h"

class LabelManager;

struct LineData
{
	string OpCode;
	string Operand;
	string OperandSuffix;
	AddrMode Mode = AddrMode::Imp;
	int OperandSize = 0;
	bool IsHex = false;
	bool IsDecimal = false;
	bool IsImmediate = false;
	bool HasComment = false;
	bool HasOpeningParenthesis = false;
	bool HasOpeningBracket = false;
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
};

class Assembler
{
private:
	std::unordered_map<string, std::unordered_set<int>> _availableModesByOpName;
	bool _needSecondPass;

	shared_ptr<LabelManager> _labelManager;
	void ProcessLine(string code, uint32_t& instructionAddress, vector<int16_t>& output, std::unordered_map<string, uint32_t>& labels, bool firstPass, std::unordered_map<string, uint32_t>& currentPassLabels);
	AssemblerSpecialCodes GetLineData(std::smatch match, LineData& lineData, std::unordered_map<string, uint32_t>& labels, bool firstPass);
	AssemblerSpecialCodes GetAddrModeAndOperandSize(LineData& lineData, std::unordered_map<string, uint32_t>& labels, bool firstPass);
	void AssembleInstruction(LineData& lineData, uint32_t& instructionAddress, vector<int16_t>& output, bool firstPass);

	bool IsOpModeAvailable(string& opCode, AddrMode mode);

public:
	Assembler(shared_ptr<LabelManager> labelManager);

	uint32_t AssembleCode(string code, uint32_t startAddress, int16_t* assembledCode);
};
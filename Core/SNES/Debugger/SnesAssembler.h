#pragma once
#include "stdafx.h"
#include <regex>
#include "Debugger/IAssembler.h"
#include "SNES/Debugger/CpuDisUtils.h"

class LabelManager;

struct SnesLineData
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

class SnesAssembler : public IAssembler
{
private:
	std::unordered_map<string, std::unordered_set<int>> _availableModesByOpName;
	bool _needSecondPass = false;

	LabelManager* _labelManager;
	void ProcessLine(string code, uint32_t& instructionAddress, vector<int16_t>& output, std::unordered_map<string, uint32_t>& labels, bool firstPass, std::unordered_map<string, uint32_t>& currentPassLabels);
	AssemblerSpecialCodes GetLineData(std::smatch match, SnesLineData& lineData, std::unordered_map<string, uint32_t>& labels, bool firstPass);
	AssemblerSpecialCodes GetAddrModeAndOperandSize(SnesLineData& lineData, std::unordered_map<string, uint32_t>& labels, bool firstPass);
	void AssembleInstruction(SnesLineData& lineData, uint32_t& instructionAddress, vector<int16_t>& output, bool firstPass);

	bool IsOpModeAvailable(string& opCode, AddrMode mode);

public:
	SnesAssembler(LabelManager* labelManager);
	virtual ~SnesAssembler();

	uint32_t AssembleCode(string code, uint32_t startAddress, int16_t* assembledCode);
};
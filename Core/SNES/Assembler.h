#pragma once
#include "stdafx.h"
#include <regex>
#include "IAssembler.h"
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

class Assembler : public IAssembler
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
	virtual ~Assembler();

	uint32_t AssembleCode(string code, uint32_t startAddress, int16_t* assembledCode);
};
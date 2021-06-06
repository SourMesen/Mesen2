#pragma once
#include "stdafx.h"
#include <regex>
#include "Debugger/IAssembler.h"
#include "NES/NesTypes.h"

class LabelManager;

struct NesLineData
{
	string OpCode;
	string Operand;
	string OperandSuffix;
	NesAddrMode Mode = NesAddrMode::None;
	int OperandSize = 0;
	bool IsHex = false;
	bool IsDecimal = false;
	bool IsImmediate = false;
	bool HasComment = false;
	bool HasOpeningParenthesis = false;
};

class NesAssembler final : public IAssembler
{
private:
	std::unordered_map<string, unordered_set<int>> _availableModesByOpName;
	bool _needSecondPass = false;

	shared_ptr<LabelManager> _labelManager;
	void ProcessLine(string code, uint32_t &instructionAddress, vector<int16_t>& output, unordered_map<string, uint16_t> &labels, bool firstPass, unordered_map<string, uint16_t> &currentPassLabels);
	AssemblerSpecialCodes GetLineData(std::smatch match, NesLineData& lineData, unordered_map<string, uint16_t> &labels, bool firstPass);
	AssemblerSpecialCodes GetAddrModeAndOperandSize(NesLineData& lineData, unordered_map<string, uint16_t> &labels, bool firstPass);
	void AssembleInstruction(NesLineData& lineData, uint32_t &instructionAddress, vector<int16_t>& output, bool firstPass);

	bool IsOpModeAvailable(string &opCode, NesAddrMode mode);

public:
	NesAssembler(shared_ptr<LabelManager> labelManager);
	virtual ~NesAssembler() = default;

	uint32_t AssembleCode(string code, uint32_t startAddress, int16_t* assembledCode);
};
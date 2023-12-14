#pragma once
#include "pch.h"
#include "Debugger/IAssembler.h"

class LabelManager;

class GbAssembler : public IAssembler
{
	enum class ParamType
	{
		None,
		Literal,
		Byte,
		Short,
		Address,
		HighAddress,
		RelAddress,
		StackOffset
	};

	struct ParamEntry
	{
		string Param;
		ParamType Type;
	};

	struct OpCodeEntry
	{
		uint16_t OpCode;
		int ParamCount;
		ParamEntry Param1;
		ParamEntry Param2;
	};

private:
	unordered_map<string, vector<OpCodeEntry>> _opCodes;
	LabelManager* _labelManager;

	void InitParamEntry(ParamEntry& entry, string param);
	bool IsRegisterName(string op);
	void InitAssembler();
	int ReadValue(string operand, int min, int max, unordered_map<string, uint16_t>& localLabels, bool firstPass);
	bool IsMatch(ParamEntry& entry, string operand, uint32_t address, unordered_map<string, uint16_t>& localLabels, bool firstPass);
	void PushOp(uint16_t opCode, vector<int16_t>& output, uint32_t& address);
	void PushByte(uint8_t operand, vector<int16_t>& output, uint32_t& address);
	void PushWord(uint16_t operand, vector<int16_t>& output, uint32_t& address);
	void ProcessOperand(ParamEntry& entry, string operand, vector<int16_t>& output, uint32_t& address, unordered_map<string, uint16_t>& localLabels, bool firstPass);

	void RunPass(vector<int16_t>& output, string code, uint32_t address, int16_t* assembledCode, bool firstPass, unordered_map<string, uint16_t>& localLabels);

public:
	GbAssembler(LabelManager* labelManager);
	virtual ~GbAssembler();

	uint32_t AssembleCode(string code, uint32_t startAddress, int16_t* assembledCode);
};
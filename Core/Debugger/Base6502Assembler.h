#pragma once
#include "pch.h"
#include "Shared/CpuType.h"
#include "Debugger/IAssembler.h"
#include "Utilities/StringUtilities.h"

class LabelManager;

template<class T>
class Base6502Assembler : public IAssembler
{
protected:
	enum class OperandType
	{
		None,
		A,
		X,
		Y,
		S,
		Custom
	};

	enum class OperandValueType
	{
		None,
		Decimal,
		Hex,
		Binary,
		Label
	};

	struct AssemblerOperand
	{
		OperandType Type = OperandType::None;
		OperandValueType ValueType = OperandValueType::None;
		int64_t Value = 0;
		int ByteCount = 0;
		int OperandSize = 0;
		bool IsImmediate = false;
		bool HasOpeningParenthesis = false;
		bool HasClosingParenthesis = false;
		bool HasOpeningBracket = false;
		bool HasClosingBracket = false;

		bool HasParenOrBracket()
		{
			return HasOpeningBracket || HasClosingParenthesis || HasOpeningBracket || HasClosingBracket;
		}
	};

	struct AssemblerLineData
	{
		string OpCode;
		T AddrMode = {};
		AssemblerOperand Operands[3] = {};
		int OperandCount = 0;
	};

private:
	unordered_map<string, unordered_map<T, uint8_t>> _availableModesByOpName;
	bool _needSecondPass = false;
	CpuType _cpuType;

	LabelManager* _labelManager;
	void ProcessLine(string code, uint32_t& instructionAddress, vector<int16_t>& output, unordered_map<string, uint32_t>& labels, bool firstPass, unordered_map<string, uint32_t>& currentPassLabels);
	AssemblerSpecialCodes ParseOperand(AssemblerLineData& lineData, string operand, bool firstPass, unordered_map<string, uint32_t>& labels);
	void AssembleInstruction(AssemblerLineData& lineData, uint32_t& instructionAddress, vector<int16_t>& output, bool firstPass);

protected:
	bool IsOpModeAvailable(string& opCode, T mode);
	int16_t GetOpByteCode(string& opCode, T addrMode);

	virtual string GetOpName(uint8_t opcode) = 0;
	virtual T GetOpMode(uint8_t opcode) = 0;
	virtual bool IsOfficialOp(uint8_t opcode) = 0;
	virtual AssemblerSpecialCodes ResolveOpMode(AssemblerLineData& op, uint32_t instructionAddress, bool firstPass) = 0;

	void AdjustOperandSize(AssemblerLineData& op, AssemblerOperand& operand, T orgMode, T extMode)
	{
		if(!IsOpModeAvailable(op.OpCode, orgMode)) {
			op.AddrMode = extMode;
			operand.ByteCount = 2;
		} else {
			op.AddrMode = orgMode;
		}
	}

public:
	Base6502Assembler(LabelManager* labelManager, CpuType cpuType)
	{
		_labelManager = labelManager;
		_cpuType = cpuType;
	}

	virtual ~Base6502Assembler()
	{
	}

	uint32_t AssembleCode(string code, uint32_t startAddress, int16_t* assembledCode)
	{
		for(int i = 0; i < 256; i++) {
			string opName = StringUtilities::ToUpper(GetOpName(i));
			if(_availableModesByOpName.find(opName) == _availableModesByOpName.end()) {
				_availableModesByOpName[opName] = unordered_map<T, uint8_t>();
			}

			T addrMode = GetOpMode(i);
			if(IsOfficialOp(i) || _availableModesByOpName[opName].find(addrMode) == _availableModesByOpName[opName].end()) {
				_availableModesByOpName[opName][addrMode] = i;
			}
		}

		unordered_map<string, uint32_t> temporaryLabels;
		unordered_map<string, uint32_t> currentPassLabels;

		vector<int16_t> output;
		output.reserve(1000);

		uint32_t originalStartAddr = startAddress;

		vector<string> codeLines = StringUtilities::Split(code, '\n');

		//Make 2 passes - first one to find all labels, second to assemble
		_needSecondPass = false;
		for(string& line : codeLines) {
			ProcessLine(line, startAddress, output, temporaryLabels, true, currentPassLabels);
		}

		if(_needSecondPass) {
			vector<int16_t> prevOut;
			int pass = 1;
			do {
				prevOut = output;
				currentPassLabels.clear();
				output.clear();
				startAddress = originalStartAddr;
				for(string& line : codeLines) {
					ProcessLine(line, startAddress, output, temporaryLabels, false, currentPassLabels);
				}
				pass++;
			} while(prevOut != output && pass <= 5);
		}

		memcpy(assembledCode, output.data(), std::min<int>(100000, (int)output.size()) * sizeof(uint16_t));
		return (uint32_t)output.size();
	}
};

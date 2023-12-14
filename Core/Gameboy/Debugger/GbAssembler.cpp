#include "pch.h"
#include <regex>
#include "Debugger/LabelManager.h"
#include "Gameboy/Debugger/GbAssembler.h"
#include "Gameboy/Debugger/GameboyDisUtils.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/HexUtilities.h"

static const std::regex labelRegex = std::regex("^\\s*([@_a-zA-Z][@_a-zA-Z0-9+]*)", std::regex_constants::icase);

GbAssembler::GbAssembler(LabelManager* labelManager)
{
	_labelManager = labelManager;
	InitAssembler();
}

GbAssembler::~GbAssembler()
{
}

void GbAssembler::InitAssembler()
{
	for(int i = 0; i < 512; i++) {
		string op = GameboyDisUtils::GetOpTemplate(i & 0xFF, i >= 256);
		size_t spaceIndex = op.find(' ');
		size_t commaIndex = op.find(',');
		string opName;

		OpCodeEntry entry = {};
		if(spaceIndex != string::npos) {
			opName = op.substr(0, spaceIndex);
			entry.ParamCount = commaIndex != string::npos ? 2 : 1;
		} else {
			opName = op;
			entry.ParamCount = 0;
		}

		entry.OpCode = i < 256 ? i : ((i << 8) | 0xCB);

		std::transform(opName.begin(), opName.end(), opName.begin(), ::tolower);
		if(_opCodes.find(opName) == _opCodes.end()) {
			_opCodes[opName] = vector<OpCodeEntry>();
		}

		if(entry.ParamCount > 0) {
			string operands = op.substr(spaceIndex + 1);
			operands.erase(std::remove_if(operands.begin(), operands.end(), isspace), operands.end());
			if(entry.ParamCount == 2) {
				vector<string> operandList = StringUtilities::Split(operands, ',');
				InitParamEntry(entry.Param1, operandList[0]);
				InitParamEntry(entry.Param2, operandList[1]);
			} else if(entry.ParamCount == 1) {
				InitParamEntry(entry.Param1, operands);
			}
		}
		_opCodes[opName].push_back(entry);
	}
}

void GbAssembler::InitParamEntry(ParamEntry& entry, string param)
{
	if(param == "a") {
		entry.Type = ParamType::Short;
	} else if(param == "d") {
		entry.Type = ParamType::Byte;
	} else if(param == "e") {
		entry.Type = ParamType::Short;
	} else if(param == "r") {
		entry.Type = ParamType::RelAddress;
	} else if(param == "(a)") {
		entry.Type = ParamType::Address;
	} else if(param == "(c)") {
		entry.Type = ParamType::HighAddress;
	} else if(param == "SP+d") {
		entry.Type = ParamType::StackOffset;
	} else {
		std::transform(param.begin(), param.end(), param.begin(), ::tolower);
		entry.Type = ParamType::Literal;
		entry.Param = param;
	}
	entry.Param = param;
}

bool GbAssembler::IsRegisterName(string op)
{
	std::transform(op.begin(), op.end(), op.begin(), ::tolower);
	return op == "hl" || op == "af" || op == "bc" || op == "de" || op == "a" || op == "b" || op == "c" || op == "d" || op == "e" || op == "f" || op == "l" || op == "h";
}

int GbAssembler::ReadValue(string operand, int min, int max, unordered_map<string, uint16_t>& localLabels, bool firstPass)
{
	int value = 0;
	switch(operand[0]) {
		//Hex
		case '$': value = HexUtilities::FromHex(operand.substr(1)); break;

		case '%':
			//Binary
			for(size_t i = 1; i < operand.size(); i++) {
				value <<= 1;
				value |= operand[i] == '1' ? 1 : 0;
			}
			break;

		default:
			if(std::regex_match(operand, labelRegex)) {
				if(firstPass) {
					return 0;
				} else if(localLabels.find(operand) != localLabels.end()) {
					value = localLabels.find(operand)->second;
				} else {
					int labelAddress = _labelManager->GetLabelRelativeAddress(operand, CpuType::Gameboy);
					if(labelAddress >= 0) {
						//Matching label found
						value = labelAddress;
					} else {
						return -1;
					}
				}
			} else {
				//Decimal
				for(size_t i = 0; i < operand.size(); i++) {
					if(operand[i] != '-' && (operand[i] < '0' || operand[i] > '9')) {
						return -1;
					}
				}

				try {
					value = std::stoi(operand);
					if(value < 0) {
						value = max + value + 1;
					}
				} catch(std::exception&) {
					return -1;
				}
			}
			break;
	}

	if(value < min || value > max) {
		return -1;
	}

	return value;
}

bool GbAssembler::IsMatch(ParamEntry& entry, string operand, uint32_t address, unordered_map<string, uint16_t>& localLabels, bool firstPass)
{
	if(entry.Type != ParamType::Literal && IsRegisterName(operand)) {
		return false;
	}

	switch(entry.Type) {
		case ParamType::None: return false;

		case ParamType::Literal: {
			string param = entry.Param;
			std::transform(param.begin(), param.end(), param.begin(), ::tolower);
			std::transform(operand.begin(), operand.end(), operand.begin(), ::tolower);
			return operand == param;
		}

		case ParamType::Byte:
			return ReadValue(operand, -128, 0xFF, localLabels, firstPass) >= 0;

		case ParamType::Short:
			return ReadValue(operand, -32768, 0xFFFF, localLabels, firstPass) >= 0;

		case ParamType::Address:
			if(operand.size() > 2 && operand[0] == '(' && operand[operand.size() - 1] == ')') {
				return ReadValue(operand.substr(1, operand.size() - 2), 0, 0xFFFF, localLabels, firstPass) >= 0;
			}
			return false;

		case ParamType::HighAddress:
			if(operand.size() > 2 && operand[0] == '(' && operand[operand.size() - 1] == ')') {
				return ReadValue(operand.substr(1, operand.size() - 2), 0xFF00, 0xFFFF, localLabels, firstPass) >= 0;
			}
			return false;

		case ParamType::StackOffset:
			std::transform(operand.begin(), operand.end(), operand.begin(), ::tolower);
			if(operand.size() > 3 && operand.substr(0, 3) == "sp+") {
				return ReadValue(operand.substr(3), 0, 0xFF, localLabels, firstPass) >= 0;
			}
			return false;

		case ParamType::RelAddress: {
			int value = ReadValue(operand, 0, 0xFFFF, localLabels, firstPass);
			if(value >= 0) {
				if(firstPass) {
					//Behave as if the label was in range since the address is unknown
					return true;
				}
				int offset = (value - (address + 2));
				return offset >= -128 && offset <= 127;
			} else if(firstPass) {
				return 0;
			}
			return false;
		}
	}

	return true;
}

void GbAssembler::PushOp(uint16_t opCode, vector<int16_t>& output, uint32_t& address)
{
	if(opCode < 256) {
		PushByte((uint8_t)opCode, output, address);
	} else {
		PushWord((uint16_t)opCode, output, address);
	}
}

void GbAssembler::PushByte(uint8_t operand, vector<int16_t>& output, uint32_t& address)
{
	output.push_back(operand);
	address++;
}

void GbAssembler::PushWord(uint16_t operand, vector<int16_t>& output, uint32_t& address)
{
	output.push_back((uint8_t)operand);
	output.push_back((operand >> 8));
	address += 2;
}

void GbAssembler::ProcessOperand(ParamEntry& entry, string operand, vector<int16_t>& output, uint32_t& address, unordered_map<string, uint16_t>& localLabels, bool firstPass)
{
	switch(entry.Type) {
		default:
			break;

		case ParamType::Byte:
			PushByte((uint8_t)ReadValue(operand, -128, 255, localLabels, firstPass), output, address);
			break;

		case ParamType::Short:
			PushWord((uint16_t)ReadValue(operand, -32768, 65535, localLabels, firstPass), output, address);
			break;

		case ParamType::Address:
			if(operand.size() > 2 && operand[0] == '(' && operand[operand.size() - 1] == ')') {
				PushWord((uint16_t)ReadValue(operand.substr(1, operand.size() - 2), 0, 0xFFFF, localLabels, firstPass), output, address);
			}
			break;

		case ParamType::HighAddress:
			if(operand.size() > 2 && operand[0] == '(' && operand[operand.size() - 1] == ')') {
				PushByte((uint8_t)ReadValue(operand.substr(1, operand.size() - 2), 0xFF00, 0xFFFF, localLabels, firstPass), output, address);
			}
			break;

		case ParamType::StackOffset:
			std::transform(operand.begin(), operand.end(), operand.begin(), ::tolower);
			if(operand.size() > 3 && operand.substr(0, 3) == "sp+") {
				PushByte((uint8_t)ReadValue(operand.substr(3), 0, 0xFF, localLabels, firstPass), output, address);
			}
			break;

		case ParamType::RelAddress: {
			int value = ReadValue(operand, 0, 0xFFFF, localLabels, firstPass);
			int offset = (value - (address + 1));
			PushByte((uint8_t)offset, output, address);
			break;
		}
	}
}

void GbAssembler::RunPass(vector<int16_t>& output, string code, uint32_t address, int16_t* assembledCode, bool firstPass, unordered_map<string, uint16_t>& localLabels)
{
	unordered_set<string> currentPassLabels;
	for(string line : StringUtilities::Split(code, '\n')) {
		//Remove comment
		size_t commentIndex = line.find(';');
		if(commentIndex != string::npos) {
			line = line.substr(0, commentIndex);
		}

		//Check if this is a label definition
		size_t labelDefIndex = line.find(':');
		if(labelDefIndex != string::npos) {
			std::smatch match;
			string labelName = line.substr(0, labelDefIndex);
			if(std::regex_search(labelName, match, labelRegex)) {
				string label = match.str(1);
				if(firstPass && currentPassLabels.find(label) != currentPassLabels.end()) {
					output.push_back(AssemblerSpecialCodes::LabelRedefinition);
					continue;
				} else {
					localLabels[label] = address;
					currentPassLabels.emplace(label);
					line = line.substr(labelDefIndex + 1);
				}
			} else {
				output.push_back(AssemblerSpecialCodes::InvalidLabel);
				continue;
			}
		}

		//Trim left spaces
		size_t startIndex = line.find_first_not_of("\t ");
		if(startIndex > 0 && startIndex != string::npos) {
			line = line.substr(startIndex);
		}

		//Check if this is a .db statement
		if(line.size() > 3 && line.substr(0, 3) == ".db") {
			line = line.substr(3);
			for(string byte : StringUtilities::Split(line, ' ')) {
				if(byte.empty()) {
					continue;
				}

				int value = ReadValue(byte, -128, 255, localLabels, true);
				if(value >= 0) {
					PushByte((uint8_t)value, output, address);
				}
			}
			output.push_back(AssemblerSpecialCodes::EndOfLine);
			continue;
		}

		//Find op code name
		size_t spaceIndex = line.find(' ');
		string opName;
		if(spaceIndex != string::npos) {
			opName = line.substr(0, spaceIndex);
		} else {
			opName = line;
		}

		if(opName.empty()) {
			output.push_back(AssemblerSpecialCodes::EndOfLine);
			continue;
		}

		std::transform(opName.begin(), opName.end(), opName.begin(), ::tolower);

		if(_opCodes.find(opName) == _opCodes.end()) {
			//No matching opcode found, mark it as invalid
			output.push_back(AssemblerSpecialCodes::InvalidInstruction);
			continue;
		}

		//Find the operands given
		int paramCount = 0;
		vector<string> operandList;
		if(spaceIndex != string::npos) {
			string operands = line.substr(spaceIndex + 1);
			operands.erase(std::remove_if(operands.begin(), operands.end(), isspace), operands.end());
			if(!operands.empty()) {
				size_t commaIndex = line.find(',');
				if(commaIndex != string::npos) {
					paramCount = 2;
					operandList = StringUtilities::Split(operands, ',');

					bool invalid = operandList.size() > 2;
					for(string operand : operandList) {
						if(operand.empty()) {
							invalid = true;
							break;
						}
					}

					if(invalid) {
						output.push_back(AssemblerSpecialCodes::InvalidOperands);
						continue;
					}
				} else {
					paramCount = 1;
					operandList = { operands };
				}
			}
		}

		bool matchFound = false;
		//Find a matching set of opcode + operands
		for(OpCodeEntry& entry : _opCodes.find(opName)->second) {
			if(entry.ParamCount == paramCount) {
				if(paramCount == 0) {
					PushOp(entry.OpCode, output, address);
					matchFound = true;
					break;
				} else if(paramCount == 1) {
					if(IsMatch(entry.Param1, operandList[0], address, localLabels, firstPass)) {
						PushOp(entry.OpCode, output, address);
						ProcessOperand(entry.Param1, operandList[0], output, address, localLabels, firstPass);
						matchFound = true;
						break;
					}
				} else if(paramCount == 2) {
					if(IsMatch(entry.Param1, operandList[0], address, localLabels, firstPass) && IsMatch(entry.Param2, operandList[1], address, localLabels, firstPass)) {
						PushOp(entry.OpCode, output, address);
						ProcessOperand(entry.Param1, operandList[0], output, address, localLabels, firstPass);
						ProcessOperand(entry.Param2, operandList[1], output, address, localLabels, firstPass);
						matchFound = true;
						break;
					}
				}
			}
		}

		if(!matchFound) {
			output.push_back(AssemblerSpecialCodes::InvalidOperands);
		} else {
			output.push_back(AssemblerSpecialCodes::EndOfLine);
		}
	}
}

uint32_t GbAssembler::AssembleCode(string code, uint32_t address, int16_t* assembledCode)
{
	vector<int16_t> output;
	unordered_map<string, uint16_t> localLabels;

	RunPass(output, code, address, assembledCode, true, localLabels);
	output.clear();
	RunPass(output, code, address, assembledCode, false, localLabels);

	memcpy(assembledCode, output.data(), std::min<int>(100000, (int)output.size()) * sizeof(uint16_t));
	return (uint32_t)output.size();
}

#include "pch.h"
#include <regex>
#include <unordered_map>
#include "Utilities/HexUtilities.h"
#include "Utilities/StringUtilities.h"
#include "Debugger/Base6502Assembler.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/LabelManager.h"

static const std::regex labelRegex = std::regex("^\\s*([@_a-zA-Z][@_a-zA-Z0-9+]*):(.*)", std::regex_constants::icase);
static const std::regex byteRegex = std::regex("^\\s*[.]db\\s+((\\$[a-fA-F0-9]{1,2}[ ])*)(\\$[a-fA-F0-9]{1,2})+\\s*(;*)(.*)$", std::regex_constants::icase);

template<class T>
void Base6502Assembler<T>::ProcessLine(string code, uint32_t& instructionAddress, vector<int16_t>& output, unordered_map<string, uint32_t>& labels, bool firstPass, unordered_map<string, uint32_t>& currentPassLabels)
{
	//Remove comments
	size_t commentOffset = code.find_first_of(';');
	if(commentOffset != string::npos) {
		code = code.substr(0, commentOffset);
	}

	code = StringUtilities::Trim(code);

	std::smatch match;
	if(std::regex_search(code, match, byteRegex)) {
		//Parse .db statements
		vector<string> bytes = StringUtilities::Split(match.str(1) + match.str(3), ' ');
		for(string& byte : bytes) {
			output.push_back((uint8_t)(HexUtilities::FromHex(byte.substr(1))));
			instructionAddress++;
		}
		output.push_back(AssemblerSpecialCodes::EndOfLine);
		return;
	} else if(std::regex_search(code, match, labelRegex)) {
		//Parse label definitions
		string label = match.str(1);
		string afterLabel = match.str(2);
		if(currentPassLabels.find(match.str(1)) != currentPassLabels.end()) {
			output.push_back(AssemblerSpecialCodes::LabelRedefinition);
		} else {
			labels[match.str(1)] = instructionAddress;
			currentPassLabels[match.str(1)] = instructionAddress;
			ProcessLine(afterLabel, instructionAddress, output, labels, firstPass, currentPassLabels);
		}
		return;
	}

	code = StringUtilities::Trim(code);
	if(code.empty()) {
		output.push_back(AssemblerSpecialCodes::EndOfLine);
		return;
	}
	
	AssemblerLineData op = {};

	size_t opnameOffset = code.find_first_of(' ', 0);
	if(opnameOffset != string::npos) {
		op.OpCode = StringUtilities::ToUpper(code.substr(0, opnameOffset));
		code = StringUtilities::Trim(code.substr(opnameOffset));
		if(code.size() > 0) {
			vector<string> operands = StringUtilities::Split(code, ',');
			for(string& operand : operands) {
				if(op.OperandCount >= 3) {
					output.push_back(AssemblerSpecialCodes::InvalidOperands);
					return;
				}

				AssemblerSpecialCodes result = ParseOperand(op, StringUtilities::Trim(operand), firstPass, labels);
				if(result != AssemblerSpecialCodes::OK) {
					output.push_back(result);
					return;
				}
				op.OperandCount++;
			}
		}
	} else {
		//no operands could be found
		op.OpCode = StringUtilities::ToUpper(code);
	}

	AssemblerSpecialCodes result = ResolveOpMode(op, instructionAddress, firstPass);
	if(result != AssemblerSpecialCodes::OK) {
		output.push_back(result);
		return;
	}
	
	AssembleInstruction(op, instructionAddress, output, firstPass);
}

template<class T>
AssemblerSpecialCodes Base6502Assembler<T>::ParseOperand(AssemblerLineData& lineData, string operandStr, bool firstPass, unordered_map<string, uint32_t>& labels)
{
	AssemblerOperand& operand = lineData.Operands[lineData.OperandCount];

	if(operandStr.empty()) {
		return AssemblerSpecialCodes::OK;
	}

	//Check for operands that match
	static const std::regex operandRegex = std::regex("^([(\\[]?)[\\s]*(((#?)[\\s]*([$%]?)(-?)([0-9a-f]{1,16}))|([@_a-zA-Z][@_a-zA-Z0-9+]*))[\\s]*([\\])]?)[\\s]*$", std::regex_constants::icase);

	std::smatch match;
	if(std::regex_search(operandStr, match, operandRegex)) {
		operand.HasOpeningBracket = !match.str(1).empty() && match.str(1) == "[";
		operand.HasOpeningParenthesis = !match.str(1).empty() && match.str(1) == "(";

		operand.HasClosingBracket = !match.str(9).empty() && match.str(9) == "]";
		operand.HasClosingParenthesis = !match.str(9).empty() && match.str(9) == ")";

		operand.IsImmediate = !match.str(4).empty();

		bool hasNegativeSign = !match.str(6).empty();
		string label = match.str(8);

		if(!match.str(5).empty()) {
			if(hasNegativeSign) {
				return AssemblerSpecialCodes::InvalidOperands;
			}
			operand.ValueType = match.str(5) == "$" ? OperandValueType::Hex : OperandValueType::Binary;
		} else if(!label.empty()) {
			operand.ValueType = OperandValueType::Label;
		} else {
			if(match.str(7)[0] < '0' || match.str(7)[0] > '9') {
				label = match.str(7);
				operand.ValueType = OperandValueType::Label;
			} else {
				operand.ValueType = OperandValueType::Decimal;
			}
		}

		if(operand.ValueType != OperandValueType::Label) {
			operand.Type = OperandType::Custom;

			string rawOperand = match.str(7);
			for(char c : rawOperand) {
				if(operand.ValueType == OperandValueType::Hex && !((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
					return AssemblerSpecialCodes::InvalidHex;
				} else if(operand.ValueType == OperandValueType::Binary && c != '0' && c != '1') {
					return AssemblerSpecialCodes::InvalidBinaryValue;
				} else if(operand.ValueType == OperandValueType::Decimal && (c < '0' || c > '9')) {
					return AssemblerSpecialCodes::InvalidOperands;
				}
			}

			if(operand.ValueType == OperandValueType::Hex) {
				operand.Value = HexUtilities::FromHex(rawOperand);
				if(rawOperand.size() == 0) {
					return AssemblerSpecialCodes::MissingOperand;
				} else if(rawOperand.size() <= 2) {
					operand.ByteCount = 1;
				} else if(rawOperand.size() <= 4) {
					operand.ByteCount = 2;
				} else if(rawOperand.size() <= 6) {
					operand.ByteCount = 3;
				} else {
					return AssemblerSpecialCodes::OperandOutOfRange;
				}
			} else if(operand.ValueType == OperandValueType::Decimal) {
				operand.Value = std::stoll((hasNegativeSign ? "-" : "") + rawOperand);
				if(operand.Value < -0x800000) {
					//< -2^23 is invalid
					return AssemblerSpecialCodes::OperandOutOfRange;
				} else if(operand.Value < -0x8000) {
					operand.ByteCount = 3;
				} else if(operand.Value < -0x80) {
					operand.ByteCount = 2;
				} else if(operand.Value <= 0xFF) {
					operand.ByteCount = 1;
				} else if(operand.Value <= 0xFFFF) {
					operand.ByteCount = 2;
				} else if(operand.Value <= 0xFFFFFF) {
					operand.ByteCount = 3;
				} else {
					//>= 2^23 is invalid
					return AssemblerSpecialCodes::OperandOutOfRange;
				}
			} else if(operand.ValueType == OperandValueType::Binary) {
				//Convert the binary value to hex
				int32_t value = 0;
				for(size_t i = 0; i < rawOperand.size(); i++) {
					value <<= 1;
					value |= rawOperand[i] == '1' ? 1 : 0;
				}
				operand.Value = value;
				operand.ByteCount = rawOperand.size() > 8 ? 2 : 1;
			}
		} else {
			//Labels / register name
			string lcOperand = StringUtilities::ToLower(label);
			if(lcOperand == "a") {
				operand.Type = OperandType::A;
			} else if(lcOperand == "x") {
				operand.Type = OperandType::X;
			} else if(lcOperand == "y") {
				operand.Type = OperandType::Y;
			} else if(lcOperand == "s") {
				operand.Type = OperandType::S;
			} else {
				operand.Type = OperandType::Custom;

				//Label
				auto result = labels.find(label);
				int32_t addr = -1;
				if(result != labels.end()) {
					addr = result->second;
				} else {
					addr = _labelManager->GetLabelRelativeAddress(label, _cpuType);
				}

				if(addr > 0xFFFF) {
					operand.Value = addr;
					operand.ByteCount = 3;
				} else if(addr > 0xFF) {
					operand.Value = addr;
					operand.ByteCount = 2;
				} else if(addr >= 0) {
					operand.Value = addr;
					operand.ByteCount = 1;
				} else {
					if(firstPass) {
						//First pass, we couldn't find a matching label, so it might be defined later on
						//Pretend it exists for now
						_needSecondPass = true;
						operand.Value = 0xFFFF;
						operand.ByteCount = 2;
					} else {
						return AssemblerSpecialCodes::UnknownLabel;
					}
				}
			}
		}
		return AssemblerSpecialCodes::OK;
	} else {
		//invalid operand
		return AssemblerSpecialCodes::InvalidOperands;
	}
}

template<class T>
bool Base6502Assembler<T>::IsOpModeAvailable(string& opCode, T addrMode)
{
	return _availableModesByOpName[opCode].find(addrMode) != _availableModesByOpName[opCode].end();
}

template<class T>
int16_t Base6502Assembler<T>::GetOpByteCode(string& opCode, T addrMode)
{
	auto nameResult = _availableModesByOpName.find(opCode);
	if(nameResult != _availableModesByOpName.end()) {
		auto modeResult = nameResult->second.find(addrMode);
		if(modeResult != _availableModesByOpName[opCode].end()) {
			return modeResult->second;
		}
	}

	return -1;
}

template<class T>
void Base6502Assembler<T>::AssembleInstruction(AssemblerLineData& op, uint32_t& instructionAddress, vector<int16_t>& output, bool firstPass)
{
	int16_t result = GetOpByteCode(op.OpCode, op.AddrMode);
	if(result < 0) {
		output.push_back(AssemblerSpecialCodes::InvalidInstruction);
		return;
	}

	uint8_t opCode = (uint8_t)result;
	output.push_back(opCode);
	instructionAddress++;

	for(int i = 0; i < op.OperandCount; i++) {
		instructionAddress += op.Operands[i].ByteCount;

		if(op.Operands[i].ByteCount == 1) {
			output.push_back(op.Operands[i].Value & 0xFF);
		} else if(op.Operands[i].ByteCount == 2) {
			output.push_back(op.Operands[i].Value & 0xFF);
			output.push_back((op.Operands[i].Value >> 8) & 0xFF);
		} else if(op.Operands[i].ByteCount == 3) {
			output.push_back(op.Operands[i].Value & 0xFF);
			output.push_back((op.Operands[i].Value >> 8) & 0xFF);
			output.push_back((op.Operands[i].Value >> 16) & 0xFF);
		}
	}
			
	output.push_back(AssemblerSpecialCodes::EndOfLine);
}

enum class SnesAddrMode : uint8_t;
template void Base6502Assembler<SnesAddrMode>::ProcessLine(string code, uint32_t& instructionAddress, vector<int16_t>& output, unordered_map<string, uint32_t>& labels, bool firstPass, unordered_map<string, uint32_t>& currentPassLabels);
template bool Base6502Assembler<SnesAddrMode>::IsOpModeAvailable(string& opcode, SnesAddrMode mode);

enum class NesAddrMode;
template void Base6502Assembler<NesAddrMode>::ProcessLine(string code, uint32_t& instructionAddress, vector<int16_t>& output, unordered_map<string, uint32_t>& labels, bool firstPass, unordered_map<string, uint32_t>& currentPassLabels);
template bool Base6502Assembler<NesAddrMode>::IsOpModeAvailable(string& opcode, NesAddrMode mode);

enum class PceAddrMode;
template void Base6502Assembler<PceAddrMode>::ProcessLine(string code, uint32_t& instructionAddress, vector<int16_t>& output, unordered_map<string, uint32_t>& labels, bool firstPass, unordered_map<string, uint32_t>& currentPassLabels);
template bool Base6502Assembler<PceAddrMode>::IsOpModeAvailable(string& opcode, PceAddrMode mode);

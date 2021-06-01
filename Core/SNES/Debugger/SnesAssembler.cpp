#include "stdafx.h"
#include <regex>
#include <unordered_map>
#include "Utilities/HexUtilities.h"
#include "Utilities/StringUtilities.h"
#include "SNES/Cpu.h"
#include "SNES/Debugger/SnesAssembler.h"
#include "SNES/Debugger/CpuDisUtils.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/LabelManager.h"

static const std::regex instRegex = std::regex("^\\s*([a-zA-Z]{3})[\\s]*(#%|#){0,1}([([]{0,1})[\\s]*([$]{0,1})([^\\[\\],)(;:]*)[\\s]*((,[$][0-9a-f]{1,2}|,x\\)|\\),y|,x|,y|,s\\),y|,s|\\)|\\],y|\\]){0,1})\\s*(;*)(.*)", std::regex_constants::icase);
static const std::regex isCommentOrBlank = std::regex("^\\s*([;]+.*$|\\s*$)", std::regex_constants::icase);
static const std::regex labelRegex = std::regex("^\\s*([@_a-zA-Z][@_a-zA-Z0-9]*):(.*)", std::regex_constants::icase);
static const std::regex byteRegex = std::regex("^\\s*[.]db\\s+((\\$[a-fA-F0-9]{1,2}[ ])*)(\\$[a-fA-F0-9]{1,2})+\\s*(;*)(.*)$", std::regex_constants::icase);

void SnesAssembler::ProcessLine(string code, uint32_t& instructionAddress, vector<int16_t>& output, std::unordered_map<string, uint32_t>& labels, bool firstPass, std::unordered_map<string, uint32_t>& currentPassLabels)
{
	//Remove extra spaces as part of processing
	size_t offset = code.find_first_of(',', 0);
	if(offset != string::npos) {
		code.erase(std::remove(code.begin() + offset + 1, code.end(), ' '), code.end());
	}
	offset = code.find_first_of(')', 0);
	if(offset != string::npos) {
		code.erase(std::remove(code.begin() + offset + 1, code.end(), ' '), code.end());
	}

	//Determine if the line is blank, a comment, a label or code
	std::smatch match;
	if(std::regex_search(code, match, byteRegex)) {
		vector<string> bytes = StringUtilities::Split(match.str(1) + match.str(3), ' ');
		for(string& byte : bytes) {
			output.push_back((uint8_t)(HexUtilities::FromHex(byte.substr(1))));
			instructionAddress++;
		}
		output.push_back(AssemblerSpecialCodes::EndOfLine);
	} else if(std::regex_search(code, match, labelRegex)) {
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
	} else if(std::regex_search(code, match, isCommentOrBlank)) {
		output.push_back(AssemblerSpecialCodes::EndOfLine);
		return;
	} else if(std::regex_search(code, match, instRegex) && match.size() > 1) {
		SnesLineData lineData;
		AssemblerSpecialCodes result = GetLineData(match, lineData, labels, firstPass);
		if(result == AssemblerSpecialCodes::OK) {
			AssembleInstruction(lineData, instructionAddress, output, firstPass);
		} else {
			output.push_back(result);
		}
	} else {
		output.push_back(AssemblerSpecialCodes::ParsingError);
	}
}

AssemblerSpecialCodes SnesAssembler::GetLineData(std::smatch match, SnesLineData& lineData, std::unordered_map<string, uint32_t>& labels, bool firstPass)
{
	bool isBinary = match.str(2).length() > 1 && match.str(2)[1] == '%'; //Immediate + binary: "#%"

	lineData.OpCode = match.str(1);
	lineData.IsImmediate = !match.str(2).empty();
	lineData.IsHex = !match.str(4).empty();
	lineData.HasComment = !match.str(8).empty();
	lineData.OperandSuffix = match.str(6);
	lineData.HasOpeningParenthesis = match.str(3) == "(";
	lineData.HasOpeningBracket = match.str(3) == "[";

	std::transform(lineData.OperandSuffix.begin(), lineData.OperandSuffix.end(), lineData.OperandSuffix.begin(), ::toupper);
	std::transform(lineData.OpCode.begin(), lineData.OpCode.end(), lineData.OpCode.begin(), ::toupper);

	bool foundSpace = false;
	for(char c : match.str(5)) {
		if(c != ' ' && c != '\t') {
			if(foundSpace) {
				//can't have spaces in operands (except at the very end)
				return AssemblerSpecialCodes::InvalidSpaces;
			} else if(lineData.IsHex && !((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
				//invalid hex
				return AssemblerSpecialCodes::InvalidHex;
			} else if(isBinary && c != '0' && c != '1') {
				return AssemblerSpecialCodes::InvalidBinaryValue;
			}

			lineData.Operand.push_back(c);
		} else {
			foundSpace = true;
		}
	}

	if(isBinary) {
		//Convert the binary value to hex
		if(lineData.Operand.size() == 0) {
			return AssemblerSpecialCodes::MissingOperand;
		} else if(lineData.Operand.size() <= 8) {
			lineData.IsHex = true;
			int value = 0;
			for(size_t i = 0; i < lineData.Operand.size(); i++) {
				value <<= 1;
				value |= lineData.Operand[i] == '1' ? 1 : 0;
			}
			lineData.Operand = HexUtilities::ToHex(value, false);
		} else {
			return AssemblerSpecialCodes::OperandOutOfRange;
		}
	}

	if(!lineData.HasComment && !match.str(9).empty()) {
		//something is trailing at the end of the line, and it's not a comment
		return AssemblerSpecialCodes::TrailingText;
	}

	if(!lineData.IsHex) {
		bool allNumeric = true;
		for(size_t i = 0; i < lineData.Operand.size(); i++) {
			if(lineData.Operand[i] == '-' && i == 0 && lineData.Operand.size() > 1) {
				//First char is a minus sign, and more characters follow, continue
				continue;
			}

			if((lineData.Operand[i] < '0' || lineData.Operand[i] > '9')) {
				allNumeric = false;
				break;
			}
		}

		if(allNumeric && !lineData.Operand.empty()) {
			//Operand is not empty, and it only contains decimal values
			lineData.IsDecimal = true;
		} else {
			lineData.IsDecimal = false;
		}
	}

	return GetAddrModeAndOperandSize(lineData, labels, firstPass);
}

AssemblerSpecialCodes SnesAssembler::GetAddrModeAndOperandSize(SnesLineData& lineData, std::unordered_map<string, uint32_t>& labels, bool firstPass)
{
	int opSize = 0;
	bool invalid = false;
	string operand = lineData.Operand;

	if(lineData.IsHex) {
		if(operand.size() == 0) {
			return AssemblerSpecialCodes::MissingOperand;
		} else if(operand.size() <= 2) {
			opSize = 1;
		} else if(operand.size() <= 4) {
			opSize = 2;
		} else if(operand.size() <= 6) {
			opSize = 3;
		} else {
			return AssemblerSpecialCodes::OperandOutOfRange;
		}
	} else if(lineData.IsDecimal) {
		int value = std::stoi(operand.c_str());
		if(value < -0x800000) {
			//< -2^23 is invalid
			return AssemblerSpecialCodes::OperandOutOfRange;
		} else if(value < -0x8000) {
			opSize = 3;
		} else if(value < -0x80) {
			opSize = 2;
		} else if(value <= 0xFF) {
			opSize = 1;
		} else if(value <= 0xFFFF) {
			opSize = 2;
		} else if(value <= 0xFFFFFF) {
			opSize = 3;
		} else {
			//>= 2^23 is invalid
			return AssemblerSpecialCodes::OperandOutOfRange;
		}
	} else if(!operand.empty()) {
		//Check if the operand is a known label
		auto findResult = labels.find(operand);
		if(findResult != labels.end()) {
			lineData.Operand = HexUtilities::ToHex((uint16_t)findResult->second);
			lineData.IsHex = true;
			opSize = 2;
		} else if(operand.size() == 1 && (operand[0] == 'A' || operand[0] == 'a') && lineData.OperandSuffix.empty() && !lineData.IsHex && !lineData.IsImmediate && !lineData.HasOpeningParenthesis && !lineData.HasOpeningBracket) {
			//Allow optional "A" after AddrMode == Accumulator instructions
			lineData.Mode = AddrMode::Acc;
			opSize = 0;
		} else {
			int32_t addr = _labelManager->GetLabelRelativeAddress(operand);
			if(addr > 0xFFFF) {
				lineData.Operand = HexUtilities::ToHex24(addr);
				lineData.IsHex = true;
				opSize = 3;
			} else if(addr > 0xFF) {
				lineData.Operand = HexUtilities::ToHex((uint16_t)addr);
				lineData.IsHex = true;
				opSize = 2;
			} else if(addr >= 0) {
				lineData.Operand = HexUtilities::ToHex((uint8_t)addr);
				lineData.IsHex = true;
				opSize = 1;
			} else {
				if(firstPass) {
					//First pass, we couldn't find a matching label, so it might be defined later on
					//Pretend it exists for now
					_needSecondPass = true;
					lineData.Operand = "FFFF";
					lineData.IsHex = true;
					opSize = 2;
				} else {
					return AssemblerSpecialCodes::UnknownLabel;
				}
			}
		}
	} else {
		//No operand
		opSize = 0;
	}

	if(lineData.Mode == AddrMode::Imp) {
		if(lineData.OperandSuffix.substr(0, 2) == ",$") {
			//Used by MVP, MVN
			opSize = 2;
			lineData.Mode = AddrMode::BlkMov;
			uint8_t dest = HexUtilities::FromHex(lineData.OperandSuffix.substr(2));
			lineData.Operand += HexUtilities::ToHex(dest);
		} else if(lineData.IsImmediate) {
			if(lineData.HasOpeningParenthesis || lineData.HasOpeningBracket|| opSize == 0) {
				invalid = true;
			} else if(opSize >= 3) {
				invalid = true;
			}
			lineData.Mode = opSize == 2 ? AddrMode::Imm16 : AddrMode::Imm8; //or Rel
		} else if(lineData.HasOpeningBracket) {
			if(lineData.OperandSuffix == "]") {
				switch(opSize){
					case 1: lineData.Mode = AddrMode::DirIndLng; break;
					case 2: lineData.Mode = AddrMode::AbsIndLng; break;
					default: invalid = true; break;
				}
			} else if(lineData.OperandSuffix == "],Y") {
				if(opSize == 1) {
					lineData.Mode = AddrMode::DirIndLngIdxY;
				} else {
					invalid = true;
				}
			}
		} else if(lineData.HasOpeningParenthesis) {
			if(lineData.OperandSuffix == ")") {
				lineData.Mode = opSize == 1 ? AddrMode::DirInd : AddrMode::AbsInd;
			} else if(lineData.OperandSuffix == ",X)") {
				lineData.Mode = opSize == 1 ? AddrMode::DirIdxIndX : AddrMode::AbsIdxXInd;
			} else if(lineData.OperandSuffix == "),Y") {
				if(opSize == 1) {
					lineData.Mode = AddrMode::DirIndIdxY;
				} else {
					return AssemblerSpecialCodes::OperandOutOfRange;
				}
			} else if(lineData.OperandSuffix == ",S),Y") {
				if(opSize == 1) {
					lineData.Mode = AddrMode::StkRelIndIdxY;
				} else {
					return AssemblerSpecialCodes::OperandOutOfRange;
				}
			} else {
				invalid = true;
			}
		} else {
			if(lineData.OperandSuffix == ",X") {
				if(opSize == 3) {
					lineData.Mode = AddrMode::AbsLngIdxX;
				} else if(opSize == 2) {
					lineData.Mode = AddrMode::AbsIdxX;
				} else if(opSize == 1) {
					//Sometimes zero page addressing is not available, even if the operand is in the zero page
					lineData.Mode = AddrMode::DirIdxX;
				} else {
					invalid = true;
				}
			} else if(lineData.OperandSuffix == ",Y") {
				if(opSize == 2) {
					lineData.Mode = AddrMode::AbsIdxY;
				} else if(opSize == 1) {
					//Sometimes zero page addressing is not available, even if the operand is in the zero page
					lineData.Mode = AddrMode::DirIdxY;
				} else {
					invalid = true;
				}
			} else if(lineData.OperandSuffix == ",S") {
				if(opSize == 1) {
					lineData.Mode = AddrMode::StkRel;
				} else {
					return AssemblerSpecialCodes::OperandOutOfRange;
				}
			} else if(lineData.OperandSuffix.empty()) {
				if(opSize == 0) {
					lineData.Mode = AddrMode::Imp; //or Acc
				} else if(opSize == 3) {
					lineData.Mode = AddrMode::AbsLng;
				} else if(opSize == 2) {
					lineData.Mode = AddrMode::Abs;
				} else if(opSize == 1) {
					//Sometimes zero page addressing is not available, even if the operand is in the zero page
					lineData.Mode = AddrMode::Dir;
				} else {
					invalid = true;
				}
			} else {
				invalid = true;
			}
		}
	}

	/*if(lineData.Mode == AddrMode::None) {
		invalid = true;
	}*/

	lineData.OperandSize = opSize;

	return invalid ? AssemblerSpecialCodes::ParsingError : AssemblerSpecialCodes::OK;
}

bool SnesAssembler::IsOpModeAvailable(string& opCode, AddrMode mode)
{
	return _availableModesByOpName[opCode].find((int)mode) != _availableModesByOpName[opCode].end();
}

void SnesAssembler::AssembleInstruction(SnesLineData& lineData, uint32_t& instructionAddress, vector<int16_t>& output, bool firstPass)
{
	bool foundMatch = false;

	for(int i = 0; i < 256; i++) {
		AddrMode opMode = CpuDisUtils::OpMode[i];
		if(lineData.OpCode == CpuDisUtils::OpName[i]) {
			bool modeMatch = opMode == lineData.Mode;
			if(!modeMatch) {
				if(lineData.Mode == AddrMode::Imp && (opMode == AddrMode::Acc || opMode == AddrMode::Stk)) {
					modeMatch = true;
				} else if(lineData.Mode == AddrMode::Imm8 && opMode == AddrMode::Sig8) {
					modeMatch = true;
				} else if((lineData.Mode == AddrMode::Imm8 || lineData.Mode == AddrMode::Imm16) && (opMode == AddrMode::ImmX || opMode == AddrMode::ImmM)) {
					modeMatch = true;
				} else if((opMode == AddrMode::Rel || opMode == AddrMode::RelLng) && (lineData.Mode == AddrMode::AbsLng || lineData.Mode == AddrMode::Abs || lineData.Mode == AddrMode::Dir)) {
					bool lngBranch = opMode == AddrMode::RelLng;
					modeMatch = true;

					//Convert "absolute" jump to a relative jump
					int value = lineData.IsHex ? HexUtilities::FromHex(lineData.Operand) : std::stoi(lineData.Operand);
					if(lineData.Mode == AddrMode::Abs) {
						value |= (instructionAddress & 0xFF0000);
					}

					int32_t addressGap;
					if(lineData.Mode == AddrMode::Dir) {
						addressGap = (int8_t)value;
					} else {
						addressGap = value - (instructionAddress + (lngBranch ? 3 : 2));
					}

					if(addressGap > (lngBranch ? 32767 : 127) || addressGap < (lngBranch ? -32768 : -128)) {
						//Gap too long, can't jump that far
						if(!firstPass) {
							//Pretend this is ok on first pass, we're just trying to find all labels
							output.push_back(AssemblerSpecialCodes::OutOfRangeJump);
							return;
						}
					}

					//Update data to match relative jump
					lineData.OperandSize = lngBranch ? 2 : 1;
					lineData.IsHex = true;
					lineData.Operand = lngBranch ? HexUtilities::ToHex((uint16_t)addressGap) : HexUtilities::ToHex((uint8_t)addressGap);
				}
			}

			if(modeMatch) {
				output.push_back(i);
				instructionAddress += (lineData.OperandSize + 1);

				if(lineData.OperandSize == 1) {
					int value = lineData.IsHex ? HexUtilities::FromHex(lineData.Operand) : std::stoi(lineData.Operand);
					output.push_back(value & 0xFF);
				} else if(lineData.OperandSize == 2) {
					int value = lineData.IsHex ? HexUtilities::FromHex(lineData.Operand) : std::stoi(lineData.Operand);
					output.push_back(value & 0xFF);
					output.push_back((value >> 8) & 0xFF);
				} else if(lineData.OperandSize == 3) {
					int value = lineData.IsHex ? HexUtilities::FromHex(lineData.Operand) : std::stoi(lineData.Operand);
					output.push_back(value & 0xFF);
					output.push_back((value >> 8) & 0xFF);
					output.push_back((value >> 16) & 0xFF);
				}

				foundMatch = true;
				break;
			}
		}
	}

	if(!foundMatch) {
		output.push_back(AssemblerSpecialCodes::InvalidInstruction);
	} else {
		output.push_back(AssemblerSpecialCodes::EndOfLine);
	}
}

SnesAssembler::SnesAssembler(shared_ptr<LabelManager> labelManager)
{
	_labelManager = labelManager;
}

SnesAssembler::~SnesAssembler()
{
}

uint32_t SnesAssembler::AssembleCode(string code, uint32_t startAddress, int16_t* assembledCode)
{
	for(uint8_t i = 0; i < 255; i++) {
		if(_availableModesByOpName.find(CpuDisUtils::OpName[i]) == _availableModesByOpName.end()) {
			_availableModesByOpName[CpuDisUtils::OpName[i]] = std::unordered_set<int>();
		}
		_availableModesByOpName[CpuDisUtils::OpName[i]].emplace((int)CpuDisUtils::OpMode[i]);
	}

	std::unordered_map<string, uint32_t> temporaryLabels;
	std::unordered_map<string, uint32_t> currentPassLabels;

	size_t i = 0;
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
		currentPassLabels.clear();
		output.clear();
		startAddress = originalStartAddr;
		for(string& line : codeLines) {
			ProcessLine(line, startAddress, output, temporaryLabels, false, currentPassLabels);
		}
	}

	memcpy(assembledCode, output.data(), std::min<int>(100000, (int)output.size()) * sizeof(uint16_t));
	return (uint32_t)output.size();
}

#include "stdafx.h"
#include <regex>
#include <unordered_map>
#include "Utilities/HexUtilities.h"
#include "Utilities/StringUtilities.h"
#include "NES/Debugger/NesAssembler.h"
#include "NES/Debugger/NesDisUtils.h"
#include "Debugger/LabelManager.h"

static const std::regex instRegex = std::regex("^\\s*([a-zA-Z]{3})[*]{0,1}[\\s]*(#%|#){0,1}([(]{0,1})[\\s]*([$]{0,1})([^,)(;:]*)[\\s]*((,x\\)|\\),y|,x|,y|\\)){0,1})\\s*(;*)(.*)", std::regex_constants::icase);
static const std::regex isCommentOrBlank = std::regex("^\\s*([;]+.*$|\\s*$)", std::regex_constants::icase);
static const std::regex labelRegex = std::regex("^\\s*([@_a-zA-Z][@_a-zA-Z0-9]*):(.*)", std::regex_constants::icase);
static const std::regex byteRegex = std::regex("^\\s*[.]byte\\s+((\\$[a-fA-F0-9]{1,2},)*)(\\$[a-fA-F0-9]{1,2})+\\s*(;*)(.*)$", std::regex_constants::icase);

void NesAssembler::ProcessLine(string code, uint32_t &instructionAddress, vector<int16_t>& output, std::unordered_map<string, uint16_t> &labels, bool firstPass, std::unordered_map<string, uint16_t> &currentPassLabels)
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
		vector<string> bytes = StringUtilities::Split(match.str(1) + match.str(3), ',');
		for(string &byte : bytes) {
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
		NesLineData lineData = {};
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

AssemblerSpecialCodes NesAssembler::GetLineData(std::smatch match, NesLineData& lineData, unordered_map<string, uint16_t> &labels, bool firstPass)
{
	bool isBinary = match.str(2).length() > 1 && match.str(2)[1] == '%'; //Immediate + binary: "#%"

	lineData.OpCode = match.str(1);
	lineData.IsImmediate = !match.str(2).empty();
	lineData.IsHex = !match.str(4).empty();
	lineData.HasComment = !match.str(8).empty();
	lineData.OperandSuffix = match.str(6);
	lineData.HasOpeningParenthesis = !match.str(3).empty();

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

AssemblerSpecialCodes NesAssembler::GetAddrModeAndOperandSize(NesLineData& lineData, unordered_map<string, uint16_t> &labels, bool firstPass)
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
		} else {
			return AssemblerSpecialCodes::OperandOutOfRange;
		}
	} else if(lineData.IsDecimal) {
		int value = std::stoi(operand.c_str());
		if(value < -32768) {
			//< -32768 is invalid
			return AssemblerSpecialCodes::OperandOutOfRange;
		} else if(value < -128) {
			//-32768 to -129 is 2 bytes
			opSize = 2;
		} else if(value <= 255) {
			//-128 to 255 is 2 bytes
			opSize = 1;
		} else if(value <= 65535) {
			//256 to 65535 is 2 bytes
			opSize = 2;
		} else {
			//> 65535 is invalid
			return AssemblerSpecialCodes::OperandOutOfRange;
		}
	} else if(!operand.empty()) {
		//Check if the operand is a known label
		auto findResult = labels.find(operand);
		if(findResult != labels.end()) {
			lineData.Operand = HexUtilities::ToHex((uint16_t)findResult->second);
			lineData.IsHex = true;
			opSize = 2;
		} else if(operand.size() == 1 && (operand[0] == 'A' || operand[0] == 'a') && lineData.OperandSuffix.empty() && !lineData.IsHex && !lineData.IsImmediate && !lineData.HasOpeningParenthesis) {
			//Allow optional "A" after AddrMode == Accumulator instructions
			lineData.Mode = NesAddrMode::Acc;
			opSize = 0;
		} else {
			int32_t addr = _labelManager->GetLabelRelativeAddress(operand);
			if(addr >= 256) {
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

	if(lineData.Mode == NesAddrMode::None) {
		if(lineData.IsImmediate) {
			if(lineData.HasOpeningParenthesis || opSize == 0) {
				invalid = true;
			} else if(opSize > 1) {
				if(lineData.IsHex && HexUtilities::FromHex(operand) > 0xFF) {
					//Can't be a 2-byte operand
					invalid = true;
				} else if(lineData.IsDecimal) {
					int value = std::stoi(operand.c_str());
					if(value < -128 || value > 255) {
						//Can't be a 2-byte operand
						invalid = true;
					}
				}
				opSize = 1;
			}
			lineData.Mode = NesAddrMode::Imm; //or Rel
		} else if(lineData.HasOpeningParenthesis) {
			if(lineData.OperandSuffix.compare(")") == 0) {
				opSize = 2;
				lineData.Mode = NesAddrMode::Ind;
			} else if(lineData.OperandSuffix.compare(",X)") == 0) {
				opSize = 1;
				lineData.Mode = NesAddrMode::IndX;
			} else if(lineData.OperandSuffix.compare("),Y") == 0) {
				opSize = 1;
				lineData.Mode = NesAddrMode::IndY;
			} else {
				invalid = true;
			}
		} else {
			if(lineData.OperandSuffix.compare(",X") == 0) {
				if(opSize == 2) {
					lineData.Mode = NesAddrMode::AbsX;
				} else if(opSize == 1) {
					//Sometimes zero page addressing is not available, even if the operand is in the zero page
					lineData.Mode = IsOpModeAvailable(lineData.OpCode, NesAddrMode::ZeroX) ? NesAddrMode::ZeroX : NesAddrMode::AbsX;
				} else {
					invalid = true;
				}
			} else if(lineData.OperandSuffix.compare(",Y") == 0) {
				if(opSize == 2) {
					lineData.Mode = NesAddrMode::AbsY;
				} else if(opSize == 1) {
					//Sometimes zero page addressing is not available, even if the operand is in the zero page
					lineData.Mode = IsOpModeAvailable(lineData.OpCode, NesAddrMode::ZeroY) ? NesAddrMode::ZeroY : NesAddrMode::AbsY;
				} else {
					invalid = true;
				}
			} else if(lineData.OperandSuffix.empty()) {
				if(opSize == 0) {
					lineData.Mode = NesAddrMode::Imp; //or Acc
				} else if(opSize == 2) {
					lineData.Mode = NesAddrMode::Abs;
				} else if(opSize == 1) {
					//Sometimes zero page addressing is not available, even if the operand is in the zero page
					lineData.Mode = IsOpModeAvailable(lineData.OpCode, NesAddrMode::Zero) ? NesAddrMode::Zero : NesAddrMode::Abs;
				} else {
					invalid = true;
				}
			} else {
				invalid = true;
			}
		}
	}

	if(lineData.Mode == NesAddrMode::None) {
		invalid = true;
	}

	lineData.OperandSize = opSize;

	return invalid ? AssemblerSpecialCodes::ParsingError : AssemblerSpecialCodes::OK;
}

bool NesAssembler::IsOpModeAvailable(string &opCode, NesAddrMode mode)
{
	return _availableModesByOpName[opCode].find((int)mode) != _availableModesByOpName[opCode].end();
}

void NesAssembler::AssembleInstruction(NesLineData& lineData, uint32_t &instructionAddress, vector<int16_t>& output, bool firstPass)
{
	bool foundMatch = false;
	if(lineData.Mode == NesAddrMode::Imp && lineData.OpCode.compare("NOP") == 0) {
		//NOP has multiple name+addressing type collisions, the "official" NOP is 0xEA
		output.push_back(0xEA);
		instructionAddress++;
		foundMatch = true;
	} else {
		for(int i = 0; i < 256; i++) {
			NesAddrMode opMode = NesDisUtils::GetOpMode(i);
			if(lineData.OpCode.compare(NesDisUtils::GetOpName(i)) == 0) {
				bool modeMatch = opMode == lineData.Mode;
				if(!modeMatch) {
					if((lineData.Mode == NesAddrMode::Imp && opMode == NesAddrMode::Acc) ||
						(lineData.Mode == NesAddrMode::IndY && opMode == NesAddrMode::IndYW) ||
						(lineData.Mode == NesAddrMode::AbsY && opMode == NesAddrMode::AbsYW) ||
						(lineData.Mode == NesAddrMode::AbsX && opMode == NesAddrMode::AbsXW)) {
						modeMatch = true;
					} else if((lineData.Mode == NesAddrMode::Abs && opMode == NesAddrMode::Rel) ||
								(lineData.Mode == NesAddrMode::Imm && opMode == NesAddrMode::Rel)) {
						if(lineData.OperandSize == 2) {
							if(lineData.Mode == NesAddrMode::Imm) {
								//Hardcoded jump values must be 1-byte
								if(firstPass) {
									//Pretend this is ok on first pass, we're just trying to find all labels
									lineData.OperandSize = 1;
									lineData.IsHex = true;
									lineData.Operand = "0";
									modeMatch = true;
								} else {
									output.push_back(AssemblerSpecialCodes::OutOfRangeJump);
									return;
								}
							} else {
								modeMatch = true;

								//Convert "absolute" jump to a relative jump
								int value = lineData.IsHex ? HexUtilities::FromHex(lineData.Operand) : std::stoi(lineData.Operand);

								int16_t addressGap = value - (instructionAddress + 2);
								if(addressGap > 127 || addressGap < -128) {
									//Gap too long, can't jump that far
									if(!firstPass) {
										//Pretend this is ok on first pass, we're just trying to find all labels
										output.push_back(AssemblerSpecialCodes::OutOfRangeJump);
										return;
									}
								}

								//Update data to match relative jump
								lineData.OperandSize = 1;
								lineData.IsHex = true;
								lineData.Operand = HexUtilities::ToHex((uint8_t)addressGap);
							}
						} else {
							//Accept 1-byte relative jumps
							modeMatch = true;
						}
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
					}

					foundMatch = true;
					break;
				}
			}
		}
	}

	if(!foundMatch) {
		output.push_back(AssemblerSpecialCodes::InvalidInstruction);
	} else {
		output.push_back(AssemblerSpecialCodes::EndOfLine);
	}
}

NesAssembler::NesAssembler(shared_ptr<LabelManager> labelManager)
{
	_labelManager = labelManager;
}

uint32_t NesAssembler::AssembleCode(string code, uint32_t startAddress, int16_t* assembledCode)
{
	for(uint8_t i = 0; i < 255; i++) {
		string opName = NesDisUtils::GetOpName(i);
		if(_availableModesByOpName.find(opName) == _availableModesByOpName.end()) {
			_availableModesByOpName[opName] = std::unordered_set<int>();
		}
		_availableModesByOpName[opName].emplace((int)NesDisUtils::GetOpMode(i));
	}

	std::unordered_map<string, uint16_t> temporaryLabels;
	std::unordered_map<string, uint16_t> currentPassLabels;

	vector<int16_t> output;
	output.reserve(1000);

	uint16_t originalStartAddr = startAddress;

	vector<string> codeLines = StringUtilities::Split(code, '\n');

	//Make 2 passes - first one to find all labels, second to assemble
	_needSecondPass = false;
	for(string &line : codeLines) {
		ProcessLine(line, startAddress, output, temporaryLabels, true, currentPassLabels);
	}
	
	if(_needSecondPass) {
		currentPassLabels.clear();
		output.clear();
		startAddress = originalStartAddr;
		for(string &line : codeLines) {
			ProcessLine(line, startAddress, output, temporaryLabels, false, currentPassLabels);
		}
	}

	memcpy(assembledCode, output.data(), output.size() * sizeof(uint16_t));
	return (uint32_t)output.size();
}

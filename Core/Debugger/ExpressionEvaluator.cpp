#include "pch.h"
#include <climits>
#include <algorithm>
#include "Debugger/DebugTypes.h"
#include "Debugger/ExpressionEvaluator.h"
#include "SNES/SnesConsole.h"
#include "Debugger/Debugger.h"
#include "Debugger/IDebugger.h"
#include "Debugger/MemoryDumper.h"
#include "Debugger/Disassembler.h"
#include "Debugger/LabelManager.h"
#include "Debugger/DebugUtilities.h"
#include "Utilities/HexUtilities.h"

const vector<string> ExpressionEvaluator::_binaryOperators = { { "*", "/", "%", "+", "-", "<<", ">>", "<", "<=", ">", ">=", "==", "!=", "&", "^", "|", "&&", "||" } };
const vector<int> ExpressionEvaluator::_binaryPrecedence = { { 10,  10,  10,   9,   9,    8,    8,   7,   7,    7,    7,    6,    6,   5,   4,   3,    2,    1 } };
const vector<string> ExpressionEvaluator::_unaryOperators = { { "+", "-", "~", "!", ":", "#" } };
const vector<int> ExpressionEvaluator::_unaryPrecedence = { { 11,  11,  11,  11, 11, 11 } };
const std::unordered_set<string> ExpressionEvaluator::_operators = { { "*", "/", "%", "+", "-", "<<", ">>", "<", "<=", ">", ">=", "==", "!=", "&", "^", "|", "&&", "||", "~", "!", "(", ")", "{", "}", "[", "]", ":", "#" } };

bool ExpressionEvaluator::IsOperator(string token, int &precedence, bool unaryOperator)
{
	if(unaryOperator) {
		for(size_t i = 0, len = _unaryOperators.size(); i < len; i++) {
			if(token.compare(_unaryOperators[i]) == 0) {
				precedence = _unaryPrecedence[i];
				return true;
			}
		}
	} else {
		for(size_t i = 0, len = _binaryOperators.size(); i < len; i++) {
			if(token.compare(_binaryOperators[i]) == 0) {
				precedence = _binaryPrecedence[i];
				return true;
			}
		}
	}
	return false;
}

EvalOperators ExpressionEvaluator::GetOperator(string token, bool unaryOperator)
{
	if(unaryOperator) {
		for(size_t i = 0, len = _unaryOperators.size(); i < len; i++) {
			if(token.compare(_unaryOperators[i]) == 0) {
				return (EvalOperators)(EvalOperators::Plus + i);
			}
		}
	} else {
		for(size_t i = 0, len = _binaryOperators.size(); i < len; i++) {
			if(token.compare(_binaryOperators[i]) == 0) {
				return (EvalOperators)(EvalOperators::Multiplication + i);
			}
		}
	}
	return EvalOperators::Addition;
}

unordered_map<string, int64_t>* ExpressionEvaluator::GetAvailableTokens()
{
	switch(_cpuType) {
		case CpuType::Snes: return &GetSnesTokens();
		case CpuType::Spc: return &GetSpcTokens();
		case CpuType::NecDsp: return &GetNecDspTokens();
		case CpuType::Sa1: return &GetSnesTokens();
		case CpuType::Gsu: return &GetGsuTokens();
		case CpuType::Cx4: return &GetCx4Tokens();
		case CpuType::St018: return &GetSt018Tokens();
		case CpuType::Gameboy: return &GetGameboyTokens();
		case CpuType::Nes: return &GetNesTokens();
		case CpuType::Pce: return &GetPceTokens();
		case CpuType::Sms: return &GetSmsTokens();
		case CpuType::Gba: return &GetGbaTokens();
		case CpuType::Ws: return &GetWsTokens();
	}

	return nullptr;
}

bool ExpressionEvaluator::CheckSpecialTokens(string expression, size_t &pos, string &output, ExpressionData &data)
{
	string token;
	size_t initialPos = pos;
	size_t len = expression.size();
	do {
		char c = std::tolower(expression[pos]);
		if((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '@' || c == '\'') {
			//Only letters, numbers and underscore are allowed in code labels
			token += c;
			pos++;
		} else {
			break;
		}
	} while(pos < len);

	int64_t tokenValue = -1;

	unordered_map<string, int64_t>* availableTokens = GetAvailableTokens();
	if(availableTokens) {
		auto result = availableTokens->find(token);
		if(result != availableTokens->end()) {
			tokenValue = result->second;
		}
	}

	if(tokenValue != -1) {
		output += std::to_string(tokenValue);
		return true;
	}

	int64_t sharedToken = ProcessSharedTokens(token);
	if(sharedToken != -1) {
		output += std::to_string(sharedToken);
		return true;
	}

	string originalExpression = expression.substr(initialPos, pos - initialPos);
	bool validLabel = _labelManager->ContainsLabel(originalExpression);
	if(!validLabel) {
		//Check if a multi-byte label exists for this name
		string label = originalExpression + "+0";
		validLabel = _labelManager->ContainsLabel(label);
	}

	if(validLabel) {
		data.Labels.push_back(originalExpression);
		output += std::to_string(EvalValues::FirstLabelIndex + data.Labels.size() - 1);
		return true;
	} else {
		return false;
	}
}

int64_t ExpressionEvaluator::ProcessSharedTokens(string token) 
{
	if(token == "value") {
		return EvalValues::Value;
	} else if(token == "address") {
		return EvalValues::Address;
	} else if(token == "memaddress") {
		return EvalValues::MemoryAddress;
	} else if(token == "iswrite") {
		return EvalValues::IsWrite;
	} else if(token == "isread") {
		return EvalValues::IsRead;
	} else if(token == "isdma") {
		return EvalValues::IsDma;
	} else if(token == "isdummy") {
		return EvalValues::IsDummy;
	} else if(token == "oppc") {
		return EvalValues::OpProgramCounter;
	}
	return -1;
}

string ExpressionEvaluator::GetNextToken(string expression, size_t &pos, ExpressionData &data, bool &success, bool previousTokenIsOp)
{
	string output;
	success = true;

	if(expression.empty()) {
		success = false;
		return "";
	}

	char c = std::tolower(expression[pos]);
	char nextChar = (pos + 1 < expression.size()) ? std::tolower(expression[pos + 1]) : '\0';
	if(c == '$' || (c == '0' && nextChar == 'x')) {
		//Hex numbers
		pos++;
		if(c == '0') {
			//Skip over 'x' in '0x' prefix
			pos++;
		}

		for(size_t len = expression.size(); pos < len; pos++) {
			c = std::tolower(expression[pos]);
			if((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')) {
				output += c;
			} else {
				break;
			}
		}
		if(output.empty()) {
			//No numbers followed the hex mark, this isn't a valid expression
			success = false;
		}
		output = std::to_string((uint32_t)HexUtilities::FromHex(output));
	} else if(c == '%' && previousTokenIsOp) {
		//Binary numbers
		pos++;
		for(size_t len = expression.size(); pos < len; pos++) {
			c = std::tolower(expression[pos]);
			if(c == '0' || c == '1') {
				output += c;
			} else {
				break;
			}
		}
		if(output.empty()) {
			//No numbers followed the binary mark, this isn't a valid expression
			success = false;
		}

		uint32_t value = 0;
		for(size_t i = 0; i < output.size(); i++) {
			value <<= 1;
			value |= output[i] == '1' ? 1 : 0;
		}
		output = std::to_string(value);
	} else if(c >= '0' && c <= '9') {
		//Regular numbers
		for(size_t len = expression.size(); pos < len; pos++) {
			c = std::tolower(expression[pos]);
			if(c >= '0' && c <= '9') {
				output += c;
			} else {
				break;
			}
		}
	} else if((c < 'a' || c > 'z') && c != '_' && c != '@' && c != '\'') {
		//Operators
		string operatorToken;
		for(size_t len = expression.size(); pos < len; pos++) {
			c = std::tolower(expression[pos]);
			operatorToken += c;
			if(output.empty() || _operators.find(operatorToken) != _operators.end()) {
				//If appending the next char results in a valid operator, append it (or if this is the first character)
				output += c;
			} else {
				//Reached the end of the operator, return
				break;
			}
		}
	} else {
		//Special tokens and labels
		success = CheckSpecialTokens(expression, pos, output, data);
	}

	return output;
}
	
bool ExpressionEvaluator::ProcessSpecialOperator(EvalOperators evalOp, std::stack<EvalOperators> &opStack, std::stack<int> &precedenceStack, vector<int64_t> &outputQueue)
{
	if(opStack.empty()) {
		return false;
	}
	while(opStack.top() != evalOp) {
		outputQueue.push_back(opStack.top());
		opStack.pop();
		precedenceStack.pop();

		if(opStack.empty()) {
			return false;
		}
	}
	if(evalOp != EvalOperators::Parenthesis) {
		outputQueue.push_back(opStack.top());
	}
	opStack.pop();
	precedenceStack.pop();

	return true;
}

bool ExpressionEvaluator::ToRpn(string expression, ExpressionData &data)
{
	std::stack<EvalOperators> opStack = std::stack<EvalOperators>();	
	std::stack<int> precedenceStack;

	size_t position = 0;
	int parenthesisCount = 0;
	int bracketCount = 0;
	int braceCount = 0;

	bool previousTokenIsOp = true;
	bool operatorExpected = false;
	bool operatorOrEndTokenExpected = false;
	while(true) {
		bool success = true;
		string token = GetNextToken(expression, position, data, success, previousTokenIsOp);
		if(!success) {
			return false;
		}

		if(token.empty()) {
			break;
		}

		bool requireOperator = operatorExpected;
		bool requireOperatorOrEndToken = operatorOrEndTokenExpected;
		bool unaryOperator = previousTokenIsOp;
		
		operatorExpected = false;
		operatorOrEndTokenExpected = false;
		previousTokenIsOp = false;
		
		int precedence = 0;
		if(IsOperator(token, precedence, unaryOperator)) {
			EvalOperators op = GetOperator(token, unaryOperator);
			bool rightAssociative = unaryOperator;
			while(!opStack.empty() && ((rightAssociative && precedence < precedenceStack.top()) || (!rightAssociative && precedence <= precedenceStack.top()))) {
				//Pop operators from the stack until we find something with higher priority (or empty the stack)
				data.RpnQueue.push_back(opStack.top());
				opStack.pop();
				precedenceStack.pop();
			}
			opStack.push(op);
			precedenceStack.push(precedence);

			previousTokenIsOp = true;
		} else if(requireOperator) {
			//We needed an operator, and got something else, this isn't a valid expression (e.g "(3)4" or "[$00]22")
			return false;
		} else if(requireOperatorOrEndToken && token[0] != ')' && token[0] != ']' && token[0] != '}') {
			//We needed an operator or close token - this isn't a valid expression (e.g "%1134")
			return false;
		} else if(token[0] == '(') {
			parenthesisCount++;
			opStack.push(EvalOperators::Parenthesis);
			precedenceStack.push(0);
			previousTokenIsOp = true;
		} else if(token[0] == ')') {
			parenthesisCount--;
			if(!ProcessSpecialOperator(EvalOperators::Parenthesis, opStack, precedenceStack, data.RpnQueue)) {
				return false;
			}
			operatorOrEndTokenExpected = true;
		} else if(token[0] == '[') {
			bracketCount++;
			opStack.push(EvalOperators::Bracket);
			precedenceStack.push(0);
		} else if(token[0] == ']') {
			bracketCount--;
			if(!ProcessSpecialOperator(EvalOperators::Bracket, opStack, precedenceStack, data.RpnQueue)) {
				return false;
			}
			operatorOrEndTokenExpected = true;
		} else if(token[0] == '{') {
			braceCount++;
			opStack.push(EvalOperators::Braces);
			precedenceStack.push(0);
		} else if(token[0] == '}') {
			braceCount--;
			if(!ProcessSpecialOperator(EvalOperators::Braces, opStack, precedenceStack, data.RpnQueue)){
				return false;
			}
			operatorOrEndTokenExpected = true;
		} else {
			if(token[0] < '0' || token[0] > '9') {
				return false;
			} else {
				data.RpnQueue.push_back(std::stoll(token));
				operatorOrEndTokenExpected = true;
			}
		}
	}

	if(braceCount || bracketCount || parenthesisCount) {
		//Mismatching number of brackets/braces/parenthesis
		return false;
	}

	while(!opStack.empty()) {
		data.RpnQueue.push_back(opStack.top());
		opStack.pop();
	}

	return true;
}

int64_t ExpressionEvaluator::Evaluate(ExpressionData &data, EvalResultType &resultType, MemoryOperationInfo &operationInfo, AddressInfo& addressInfo)
{
	if(data.RpnQueue.empty()) {
		resultType = EvalResultType::Invalid;
		return 0;
	}

	int pos = 0;
	int64_t right = 0;
	int64_t left = 0;
	int64_t operandStack[100];
	resultType = EvalResultType::Numeric;

	for(size_t i = 0, len = data.RpnQueue.size(); i < len; i++) {
		int64_t token = data.RpnQueue[i];

		if(token >= EvalValues::RegA) {
			//Replace value with a special value
			if(token >= EvalValues::FirstLabelIndex) {
				int64_t labelIndex = token - EvalValues::FirstLabelIndex;
				if((size_t)labelIndex < data.Labels.size()) {
					token = _labelManager->GetLabelRelativeAddress(data.Labels[(uint32_t)labelIndex], _cpuType);
				} else {
					token = -2;
				}
				if(token < 0) {
					//Label is no longer valid
					resultType = token == -1 ? EvalResultType::OutOfScope : EvalResultType::Invalid;
					return 0;
				}
			} else {
				switch(token) {
					case EvalValues::Value: token = operationInfo.Value; break;
					case EvalValues::Address: token = operationInfo.Address; break;
					case EvalValues::MemoryAddress: token = addressInfo.Address; break;
					case EvalValues::IsWrite: token = operationInfo.Type == MemoryOperationType::Write || operationInfo.Type == MemoryOperationType::DmaWrite || operationInfo.Type == MemoryOperationType::DummyWrite; break;
					case EvalValues::IsRead: token = operationInfo.Type != MemoryOperationType::Write && operationInfo.Type != MemoryOperationType::DmaWrite && operationInfo.Type != MemoryOperationType::DummyWrite; break;
					case EvalValues::IsDma: token = operationInfo.Type == MemoryOperationType::DmaRead || operationInfo.Type == MemoryOperationType::DmaWrite; break;
					case EvalValues::IsDummy: token = operationInfo.Type == MemoryOperationType::DummyRead|| operationInfo.Type == MemoryOperationType::DummyWrite; break;
					case EvalValues::OpProgramCounter: token = _cpuDebugger->GetProgramCounter(true); break;

					default:
						if(!_cpuDebugger) {
							token = 0;
						} else {
							switch(_cpuType) {
								case CpuType::Snes: token = GetSnesTokenValue(token, resultType); break;
								case CpuType::Spc: token = GetSpcTokenValue(token, resultType); break;
								case CpuType::NecDsp: token = GetNecDspTokenValue(token, resultType); break;
								case CpuType::Sa1: token = GetSnesTokenValue(token, resultType); break;
								case CpuType::Gsu: token = GetGsuTokenValue(token, resultType); break;
								case CpuType::Cx4: token = GetCx4TokenValue(token, resultType); break;
								case CpuType::St018: token = GetSt018TokenValue(token, resultType); break;
								case CpuType::Gameboy: token = GetGameboyTokenValue(token, resultType); break;
								case CpuType::Nes: token = GetNesTokenValue(token, resultType); break;
								case CpuType::Pce: token = GetPceTokenValue(token, resultType); break;
								case CpuType::Sms: token = GetSmsTokenValue(token, resultType); break;
								case CpuType::Gba: token = GetGbaTokenValue(token, resultType); break;
								case CpuType::Ws: token = GetWsTokenValue(token, resultType); break;
							}
						}
						break;
				}
			}
		} else if(token >= EvalOperators::Multiplication) {
			if(pos <= 0) {
				resultType = EvalResultType::Invalid;
				return 0;
			}

			right = operandStack[--pos];
			if(pos > 0 && token <= EvalOperators::LogicalOr) {
				//Only do this for binary operators
				left = operandStack[--pos];
			}

			resultType = EvalResultType::Numeric;
			switch(token) {
				case EvalOperators::Multiplication: token = left * right; break;
				case EvalOperators::Division: 
					if(right == 0) {
						resultType = EvalResultType::DivideBy0;
						return 0;
					}
					token = left / right; break;
				case EvalOperators::Modulo:
					if(right == 0) {
						resultType = EvalResultType::DivideBy0;
						return 0;
					}
					token = left % right;
					break;
				case EvalOperators::Addition: token = left + right; break;
				case EvalOperators::Substration: token = left - right; break;
				case EvalOperators::ShiftLeft: token = left << right; break;
				case EvalOperators::ShiftRight: token = left >> right; break;
				case EvalOperators::SmallerThan: token = left < right; resultType = EvalResultType::Boolean; break;
				case EvalOperators::SmallerOrEqual: token = left <= right; resultType = EvalResultType::Boolean; break;
				case EvalOperators::GreaterThan: token = left > right; resultType = EvalResultType::Boolean; break;
				case EvalOperators::GreaterOrEqual: token = left >= right; resultType = EvalResultType::Boolean; break;
				case EvalOperators::Equal: token = left == right; resultType = EvalResultType::Boolean; break;
				case EvalOperators::NotEqual: token = left != right; resultType = EvalResultType::Boolean; break;
				case EvalOperators::BinaryAnd: token = left & right; break;
				case EvalOperators::BinaryXor: token = left ^ right; break;
				case EvalOperators::BinaryOr: token = left | right; break;
				case EvalOperators::LogicalAnd: token = (bool)(left && right); resultType = EvalResultType::Boolean; break;
				case EvalOperators::LogicalOr: token = (bool)(left || right); resultType = EvalResultType::Boolean; break;

				//Unary operators
				case EvalOperators::Plus: token = right; break;
				case EvalOperators::Minus: token = -right; break;
				case EvalOperators::BinaryNot: token = ~right; break;
				case EvalOperators::LogicalNot: token = (bool)!right; break;
				case EvalOperators::AbsoluteAddress: token = right >= 0 ? _debugger->GetAbsoluteAddress({ (int32_t)right, _cpuMemory }).Address : -1; break;
				case EvalOperators::ReadDword: token = _debugger->GetMemoryDumper()->GetMemoryValue32(_cpuMemory, (uint32_t)right); break;

				case EvalOperators::Bracket: token = _debugger->GetMemoryDumper()->GetMemoryValue(_cpuMemory, (uint32_t)right); break;
				case EvalOperators::Braces: token = _debugger->GetMemoryDumper()->GetMemoryValue16(_cpuMemory, (uint32_t)right); break;
				default: throw std::runtime_error("Invalid operator");
			}
		}
		operandStack[pos++] = token;
		if(pos >= 100) {
			resultType = EvalResultType::Invalid;
			return 0;
		}
	}
	return std::clamp<int64_t>(operandStack[0], INT32_MIN, UINT32_MAX);
}

ExpressionEvaluator::ExpressionEvaluator(Debugger* debugger, IDebugger* cpuDebugger, CpuType cpuType)
{
	_debugger = debugger;
	_cpuDebugger = cpuDebugger;
	_labelManager = debugger->GetLabelManager();
	_cpuType = cpuType;
	_cpuMemory = DebugUtilities::GetCpuMemoryType(cpuType);
}

bool ExpressionEvaluator::ReturnBool(int64_t value, EvalResultType& resultType)
{
	resultType = EvalResultType::Boolean;
	return value != 0;
}

ExpressionData ExpressionEvaluator::GetRpnList(string expression, bool &success)
{
	ExpressionData* cachedData = PrivateGetRpnList(expression, success);
	if(cachedData) {
		return *cachedData;
	} else {
		return ExpressionData();
	}
}

void ExpressionEvaluator::GetTokenList(char* tokenList)
{
	//Returns all CPU-specific tokens, sorted by the their EvalValues order
	unordered_map<string, int64_t>* availableTokens = GetAvailableTokens();
	
	vector<std::pair<string, int64_t>> entries;
	
	int pos = 0;
	if(availableTokens) {
		for(auto entry : *availableTokens) {
			entries.push_back(entry);
		}
	}

	std::sort(entries.begin(), entries.end(), [&](const auto& a, const auto& b) -> bool {
		return a.second < b.second;
	});

	for(auto entry : entries) {
		if(pos + entry.first.size() + 1 >= 1000) {
			break;
		}

		memcpy(tokenList + pos, entry.first.c_str(), entry.first.size());
		pos += (int)entry.first.size();
		tokenList[pos] = '|';
		pos++;
	}
}

ExpressionData* ExpressionEvaluator::PrivateGetRpnList(string expression, bool& success)
{
	ExpressionData *cachedData = nullptr;
	{
		LockHandler lock = _cacheLock.AcquireSafe();

		auto result = _cache.find(expression);
		if(result != _cache.end()) {
			cachedData = &(result->second);
		}
	}

	if(cachedData == nullptr) {
		string fixedExp = expression;
		fixedExp.erase(std::remove(fixedExp.begin(), fixedExp.end(), ' '), fixedExp.end());
		ExpressionData data;
		success = ToRpn(fixedExp, data);
		if(success) {
			LockHandler lock = _cacheLock.AcquireSafe();
			_cache[expression] = data;
			cachedData = &_cache[expression];
		}
	} else {
		success = true;
	}

	return cachedData;
}

int64_t ExpressionEvaluator::PrivateEvaluate(string expression, EvalResultType &resultType, MemoryOperationInfo &operationInfo, AddressInfo& addressInfo, bool& success)
{
	success = true;
	ExpressionData *cachedData = PrivateGetRpnList(expression, success);

	if(!success) {
		resultType = EvalResultType::Invalid;
		return 0;
	}

	return Evaluate(*cachedData, resultType, operationInfo, addressInfo);	
}

int64_t ExpressionEvaluator::Evaluate(string expression, EvalResultType &resultType, MemoryOperationInfo &operationInfo, AddressInfo& addressInfo)
{
	try {
		bool success;
		int64_t result = PrivateEvaluate(expression, resultType, operationInfo, addressInfo, success);
		if(success) {
			return result;
		}
	} catch(std::exception&) {
	}
	resultType = EvalResultType::Invalid;
	return 0;
}

bool ExpressionEvaluator::Validate(string expression)
{
	try {
		EvalResultType type;
		MemoryOperationInfo operationInfo = {};
		AddressInfo addressInfo = {};
		bool success;
		PrivateEvaluate(expression, type, operationInfo, addressInfo, success);
		return success;
	} catch(std::exception&) {
		return false;
	}
}

#if _DEBUG
#include <assert.h>
#include "SNES/SnesCpuTypes.h"
#include "SNES/SnesPpuTypes.h"
void ExpressionEvaluator::RunTests()
{
	//Some basic unit tests to run in debug mode
	auto test = [=](string expr, EvalResultType expectedType, int64_t expectedResult) {
		MemoryOperationInfo opInfo = {};
		AddressInfo addrInfo = {};
		EvalResultType type;
		int64_t result = Evaluate(expr, type, opInfo, addrInfo);

		assert(type == expectedType);
		assert(result == expectedResult);
	};
	
	test("1 - -1", EvalResultType::Numeric, 2);
	test("1 - (-1)", EvalResultType::Numeric, 2);
	test("1 - -(-1)", EvalResultType::Numeric, 0);
	test("(0 - 1) == -1 && 5 < 10", EvalResultType::Boolean, true);
	test("(0 - 1) == 0 || 5 < 10", EvalResultType::Boolean, true);
	test("(0 - 1) == 0 || 5 < -10", EvalResultType::Boolean, false);
	test("(0 - 1) == 0 || 15 < 10", EvalResultType::Boolean, false);

	test("10 != $10", EvalResultType::Boolean, true);
	test("10 == $A", EvalResultType::Boolean, true);
	test("10 == $0A", EvalResultType::Boolean, true);

	test("(0 - 1 == 0 || 15 < 10", EvalResultType::Invalid, 0);
	test("10 / 0", EvalResultType::DivideBy0, 0);

	uint8_t byte4500 = _debugger->GetMemoryDumper()->GetMemoryValue(_cpuMemory, 0x4500);
	uint16_t word4500 = _debugger->GetMemoryDumper()->GetMemoryValue16(_cpuMemory, 0x4500);
	uint32_t dword4500 = _debugger->GetMemoryDumper()->GetMemoryValue32(_cpuMemory, 0x4500);
	uint32_t dword4501 = _debugger->GetMemoryDumper()->GetMemoryValue32(_cpuMemory, 0x4501);
	uint8_t indirectByte = _debugger->GetMemoryDumper()->GetMemoryValue(_cpuMemory, 0x4500 + byte4500);

	SnesCpuState& state = (SnesCpuState&)_cpuDebugger->GetState();
	SnesPpuState ppu;
	_cpuDebugger->GetPpuState(ppu);

	test("x + 5", EvalResultType::Numeric, state.X + 5);
	test("x == 0", EvalResultType::Boolean, state.X == 0);
	test("x == y", EvalResultType::Boolean, state.X == state.Y);
	test("x == y == scanline", EvalResultType::Boolean, state.X == state.Y == ppu.Scanline); //because (x == y) is true, and true != scanline
	test("x == y && !(a == x)", EvalResultType::Boolean, state.X == state.Y && !(state.A == state.X));

	test("(~0 & ~1) & $FFF == $FFE", EvalResultType::Numeric, 0); //because of operator priority (& is done after ==)
	test("((~0 & ~1) & $FFF) == $FFE", EvalResultType::Boolean, true);

	test("1+3*3+10/(3+4)", EvalResultType::Numeric, 11);
	test("(1+3*3+10)/(3+4)", EvalResultType::Numeric, 2);
	test("(1+3*3+10)/3+4", EvalResultType::Numeric, 10);

	test("{$4500}", EvalResultType::Numeric, word4500);
	test("[$4500]", EvalResultType::Numeric, byte4500);
	
	test("[$45]3", EvalResultType::Invalid, 0);
	test("($45)3", EvalResultType::Invalid, 0);
	test("($45]", EvalResultType::Invalid, 0);
	
	test("%11", EvalResultType::Numeric, 3);
	test("%011", EvalResultType::Numeric, 3);
	test("%1011", EvalResultType::Numeric, 11);
	test("%12", EvalResultType::Invalid, 0);

	test("10 % 5", EvalResultType::Numeric, 0);
	test("%100 % 5", EvalResultType::Numeric, 4);
	test("%100%5", EvalResultType::Numeric, 4);
	test("%10(10)", EvalResultType::Invalid, 0);
	test("10%4*10", EvalResultType::Numeric, 20);
	test("(5+5)%3", EvalResultType::Numeric, 1);
	test("11%%10", EvalResultType::Numeric, 1); //11 modulo of 2 in binary (%10)

	test("[$4500+[$4500]]", EvalResultType::Numeric, indirectByte);
	test("-($10+[$4500])", EvalResultType::Numeric, -(0x10 + byte4500));

	test("#$4500", EvalResultType::Numeric, dword4500);
	test("#$4500+1", EvalResultType::Numeric, dword4500 + 1);
	test("#($4500+1)", EvalResultType::Numeric, dword4501);
}
#endif
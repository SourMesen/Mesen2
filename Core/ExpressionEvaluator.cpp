#include "stdafx.h"
#include <climits>
#include <algorithm>
#include "DebugTypes.h"
#include "ExpressionEvaluator.h"
#include "Console.h"
#include "Debugger.h"
#include "MemoryDumper.h"
#include "Disassembler.h"
#include "../Utilities/HexUtilities.h"

const vector<string> ExpressionEvaluator::_binaryOperators = { { "*", "/", "%", "+", "-", "<<", ">>", "<", "<=", ">", ">=", "==", "!=", "&", "^", "|", "&&", "||" } };
const vector<int> ExpressionEvaluator::_binaryPrecedence = { { 10,  10,  10,   9,   9,    8,    8,   7,   7,    7,    7,    6,    6,   5,   4,   3,    2,    1 } };
const vector<string> ExpressionEvaluator::_unaryOperators = { { "+", "-", "~", "!" } };
const vector<int> ExpressionEvaluator::_unaryPrecedence = { { 11,  11,  11,  11 } };
const std::unordered_set<string> ExpressionEvaluator::_operators = { { "*", "/", "%", "+", "-", "<<", ">>", "<", "<=", ">", ">=", "==", "!=", "&", "^", "|", "&&", "||", "~", "!", "(", ")", "{", "}", "[", "]" } };

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

bool ExpressionEvaluator::CheckSpecialTokens(string expression, size_t &pos, string &output, ExpressionData &data)
{
	string token;
	//size_t initialPos = pos;
	size_t len = expression.size();
	do {
		char c = std::tolower(expression[pos]);
		if((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '@') {
			//Only letters, numbers and underscore are allowed in code labels
			token += c;
			pos++;
		} else {
			break;
		}
	} while(pos < len);

	if(token == "a") {
		output += std::to_string((int64_t)EvalValues::RegA);
	} else if(token == "x") {
		output += std::to_string((int64_t)EvalValues::RegX);
	} else if(token == "y") {
		output += std::to_string((int64_t)EvalValues::RegY);
	} else if(token == "ps") {
		output += std::to_string((int64_t)EvalValues::RegPS);
	} else if(token == "sp") {
		output += std::to_string((int64_t)EvalValues::RegSP);
	} else if(token == "pc") {
		output += std::to_string((int64_t)EvalValues::RegPC);
	} else if(token == "oppc") {
		output += std::to_string((int64_t)EvalValues::RegOpPC);
	} else if(token == "previousoppc") {
		output += std::to_string((int64_t)EvalValues::PreviousOpPC);
	} else if(token == "frame") {
		output += std::to_string((int64_t)EvalValues::PpuFrameCount);
	} else if(token == "cycle") {
		output += std::to_string((int64_t)EvalValues::PpuCycle);
	} else if(token == "scanline") {
		output += std::to_string((int64_t)EvalValues::PpuScanline);
	} else if(token == "irq") {
		output += std::to_string((int64_t)EvalValues::Irq);
	} else if(token == "nmi") {
		output += std::to_string((int64_t)EvalValues::Nmi);
	} else if(token == "value") {
		output += std::to_string((int64_t)EvalValues::Value);
	} else if(token == "address") {
		output += std::to_string((int64_t)EvalValues::Address);
	} else if(token == "romaddress") {
		output += std::to_string((int64_t)EvalValues::AbsoluteAddress);
	} else if(token == "iswrite") {
		output += std::to_string((int64_t)EvalValues::IsWrite);
	} else if(token == "isread") {
		output += std::to_string((int64_t)EvalValues::IsRead);
	} else {
		return false;
		//TODO LABELS
		/*string originalExpression = expression.substr(initialPos, pos - initialPos);
		bool validLabel = _debugger->GetLabelManager()->ContainsLabel(originalExpression);
		if(!validLabel) {
			//Check if a multi-byte label exists for this name
			string label = originalExpression + "+0";
			validLabel = _debugger->GetLabelManager()->ContainsLabel(label);
		}

		if(validLabel) {
			data.Labels.push_back(originalExpression);
			output += std::to_string(EvalValues::FirstLabelIndex + data.Labels.size() - 1);
		} else {
			return false;
		}*/
	}

	return true;
}

string ExpressionEvaluator::GetNextToken(string expression, size_t &pos, ExpressionData &data, bool &success)
{
	string output;
	success = true;

	char c = std::tolower(expression[pos]);
	if(c == '$') {
		//Hex numbers
		pos++;
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
	} else if(c == '%') {
		//Binary numbers
		pos++;
		for(size_t len = expression.size(); pos < len; pos++) {
			c = std::tolower(expression[pos]);
			if(c == '0' || c <= '1') {
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
	} else if((c < 'a' || c > 'z') && c != '_' && c != '@') {
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
		string token = GetNextToken(expression, position, data, success);
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
			operatorExpected = true;
		} else if(token[0] == '[') {
			bracketCount++;
			opStack.push(EvalOperators::Bracket);
			precedenceStack.push(0);
		} else if(token[0] == ']') {
			bracketCount--;
			if(!ProcessSpecialOperator(EvalOperators::Bracket, opStack, precedenceStack, data.RpnQueue)) {
				return false;
			}
			operatorExpected = true;
		} else if(token[0] == '{') {
			braceCount++;
			opStack.push(EvalOperators::Braces);
			precedenceStack.push(0);
		} else if(token[0] == '}') {
			braceCount--;
			if(!ProcessSpecialOperator(EvalOperators::Braces, opStack, precedenceStack, data.RpnQueue)){
				return false;
			}
			operatorExpected = true;
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

int32_t ExpressionEvaluator::Evaluate(ExpressionData &data, DebugState &state, EvalResultType &resultType, MemoryOperationInfo &operationInfo)
{
	if(data.RpnQueue.empty()) {
		resultType = EvalResultType::Invalid;
		return 0;
	}

	int pos = 0;
	int64_t right = 0;
	int64_t left = 0;
	resultType = EvalResultType::Numeric;

	for(size_t i = 0, len = data.RpnQueue.size(); i < len; i++) {
		int64_t token = data.RpnQueue[i];

		if(token >= EvalValues::RegA) {
			//Replace value with a special value
			if(token >= EvalValues::FirstLabelIndex) {
				resultType = EvalResultType::Invalid;
				return 0;
				
				//TODO
				/*int64_t labelIndex = token - EvalValues::FirstLabelIndex;
				if((size_t)labelIndex < data.Labels.size()) {
					token = _debugger->GetLabelManager()->GetLabelRelativeAddress(data.Labels[(uint32_t)labelIndex]);
					if(token < -1) {
						//Label doesn't exist, try to find a matching multi-byte label
						string label = data.Labels[(uint32_t)labelIndex] + "+0";
						token = _debugger->GetLabelManager()->GetLabelRelativeAddress(label);
					}
				} else {
					token = -2;
				}
				if(token < 0) {
					//Label is no longer valid
					resultType = token == -1 ? EvalResultType::OutOfScope : EvalResultType::Invalid;
					return 0;
				}*/
			} else {
				switch(token) {
					case EvalValues::RegA: token = state.Cpu.A; break;
					case EvalValues::RegX: token = state.Cpu.X; break;
					case EvalValues::RegY: token = state.Cpu.Y; break;
					case EvalValues::RegSP: token = state.Cpu.SP; break;
					case EvalValues::RegPS: token = state.Cpu.PS; break;
					case EvalValues::RegPC: token = state.Cpu.PC; break;
					
					//TODO
					/*case EvalValues::RegOpPC: token = state.Cpu.DebugPC; break;*/
					case EvalValues::PpuFrameCount: token = state.Ppu.FrameCount; break;
					case EvalValues::PpuCycle: token = state.Ppu.Cycle; break;
					case EvalValues::PpuScanline: token = state.Ppu.Scanline; break;
					case EvalValues::Nmi: token = state.Cpu.NmiFlag; resultType = EvalResultType::Boolean; break;
					case EvalValues::Irq: token = state.Cpu.IrqSource != 0; resultType = EvalResultType::Boolean; break;
					case EvalValues::Value: token = operationInfo.Value; break;
					case EvalValues::Address: token = operationInfo.Address; break;
					//case EvalValues::AbsoluteAddress: token = _debugger->GetAbsoluteAddress(operationInfo.Address); break;
					case EvalValues::IsWrite: token = operationInfo.Type == MemoryOperationType::Write || operationInfo.Type == MemoryOperationType::DmaWrite; break;
					case EvalValues::IsRead: token = operationInfo.Type != MemoryOperationType::Write && operationInfo.Type != MemoryOperationType::DmaWrite; break;
					//case EvalValues::PreviousOpPC: token = state.CPU.PreviousDebugPC; break;
				}
			}
		} else if(token >= EvalOperators::Multiplication) {
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
				case EvalOperators::LogicalAnd: token = left && right; resultType = EvalResultType::Boolean; break;
				case EvalOperators::LogicalOr: token = left || right; resultType = EvalResultType::Boolean; break;

				//Unary operators
				case EvalOperators::Plus: token = right; break;
				case EvalOperators::Minus: token = -right; break;
				case EvalOperators::BinaryNot: token = ~right; break;
				case EvalOperators::LogicalNot: token = !right; break;
				case EvalOperators::Bracket: token = _debugger->GetMemoryDumper()->GetMemoryValue(SnesMemoryType::CpuMemory, (uint32_t)right); break;
				case EvalOperators::Braces: token = _debugger->GetMemoryDumper()->GetMemoryValueWord(SnesMemoryType::CpuMemory, (uint32_t)right); break;
				default: throw std::runtime_error("Invalid operator");
			}
		}
		operandStack[pos++] = token;
	}
	return (int32_t)operandStack[0];
}

ExpressionEvaluator::ExpressionEvaluator(Debugger* debugger)
{
	_debugger = debugger;
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

int32_t ExpressionEvaluator::PrivateEvaluate(string expression, DebugState &state, EvalResultType &resultType, MemoryOperationInfo &operationInfo, bool& success)
{
	success = true;
	ExpressionData *cachedData = PrivateGetRpnList(expression, success);

	if(!success) {
		resultType = EvalResultType::Invalid;
		return 0;
	}

	return Evaluate(*cachedData, state, resultType, operationInfo);	
}

int32_t ExpressionEvaluator::Evaluate(string expression, DebugState &state, EvalResultType &resultType, MemoryOperationInfo &operationInfo)
{
	try {
		bool success;
		int32_t result = PrivateEvaluate(expression, state, resultType, operationInfo, success);
		if(success) {
			return result;
		}
	} catch(std::exception e) {
	}
	resultType = EvalResultType::Invalid;
	return 0;
}

bool ExpressionEvaluator::Validate(string expression)
{
	try {
		DebugState state;
		EvalResultType type;
		MemoryOperationInfo operationInfo;
		bool success;
		PrivateEvaluate(expression, state, type, operationInfo, success);
		return success;
	} catch(std::exception e) {
		return false;
	}
}

#if _DEBUG
#include <assert.h>
void ExpressionEvaluator::RunTests()
{
	//Some basic unit tests to run in debug mode
	auto test = [=](string expr, EvalResultType expectedType, int expectedResult) {
		DebugState state = { 0 };
		MemoryOperationInfo opInfo(0, 0, MemoryOperationType::Read);
		EvalResultType type;
		int32_t result = Evaluate(expr, state, type, opInfo);

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

	test("x + 5", EvalResultType::Numeric, 5);
	test("x == 0", EvalResultType::Boolean, true);
	test("x == y", EvalResultType::Boolean, true);
	test("x == y == scanline", EvalResultType::Boolean, false); //because (x == y) is true, and true != scanline
	test("x == y && !(a == x)", EvalResultType::Boolean, false);

	test("(~0 & ~1) & $FFF == $FFE", EvalResultType::Numeric, 0); //because of operator priority (& is done after ==)
	test("((~0 & ~1) & $FFF) == $FFE", EvalResultType::Boolean, true);

	test("1+3*3+10/(3+4)", EvalResultType::Numeric, 11);
	test("(1+3*3+10)/(3+4)", EvalResultType::Numeric, 2);
	test("(1+3*3+10)/3+4", EvalResultType::Numeric, 10);

	test("{$4500}", EvalResultType::Numeric, 0x4545);
	test("[$4500]", EvalResultType::Numeric, 0x45);
	
	test("[$45]3", EvalResultType::Invalid, 0);
	test("($45)3", EvalResultType::Invalid, 0);
	test("($45]", EvalResultType::Invalid, 0);
	
	test("%11", EvalResultType::Numeric, 3);
	test("%011", EvalResultType::Numeric, 3);
	test("%1011", EvalResultType::Numeric, 11);
	test("%12", EvalResultType::Invalid, 0);
}
#endif
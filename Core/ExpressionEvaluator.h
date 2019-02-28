#pragma once
#include "stdafx.h"
#include <stack>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include "DebugTypes.h"
#include "../Utilities/SimpleLock.h"

class Debugger;

enum EvalOperators : int64_t
{
	//Binary operators
	Multiplication = 20000000000,
	Division = 20000000001,
	Modulo = 20000000002,
	Addition = 20000000003,
	Substration = 20000000004,
	ShiftLeft = 20000000005,
	ShiftRight = 20000000006,
	SmallerThan = 20000000007,
	SmallerOrEqual = 20000000008,
	GreaterThan = 20000000009,
	GreaterOrEqual = 20000000010,
	Equal = 20000000011,
	NotEqual = 20000000012,
	BinaryAnd = 20000000013,
	BinaryXor = 20000000014,
	BinaryOr = 20000000015,
	LogicalAnd = 20000000016,
	LogicalOr = 20000000017,

	//Unary operators
	Plus = 20000000050,
	Minus = 20000000051,
	BinaryNot = 20000000052,
	LogicalNot = 20000000053,

	//Used to read ram address
	Bracket = 20000000054, //Read byte
	Braces = 20000000055, //Read word

	//Special value, not used as an operator
	Parenthesis = 20000000100,
};

enum EvalValues : int64_t
{
	RegA = 20000000100,
	RegX = 20000000101,
	RegY = 20000000102,
	RegSP = 20000000103,
	RegPS = 20000000104,
	RegPC = 20000000105,
	RegOpPC = 20000000106,
	PpuFrameCount = 20000000107,
	PpuCycle = 20000000108,
	PpuScanline = 20000000109,
	Nmi = 20000000110,
	Irq = 20000000111,
	Value = 20000000112,
	Address = 20000000113,
	AbsoluteAddress = 20000000114,
	IsWrite = 20000000115,
	IsRead = 20000000116,
	PreviousOpPC = 20000000117,
	Sprite0Hit = 20000000118,
	SpriteOverflow = 20000000119,
	VerticalBlank = 20000000120,
	Branched = 20000000121,

	FirstLabelIndex = 20000002000,
};

enum class EvalResultType : int32_t
{
	Numeric = 0,
	Boolean = 1,
	Invalid = 2,
	DivideBy0 = 3,
	OutOfScope = 4
};

class StringHasher
{
public:
	size_t operator()(const std::string& t) const 
	{
		//Quick hash for expressions - most are likely to have different lengths, and not expecting dozens of breakpoints, either, so this should be fine.
		return t.size();
	}
};

struct ExpressionData
{
	std::vector<int64_t> RpnQueue;
	std::vector<string> Labels;
};

class ExpressionEvaluator
{
private:
	static const vector<string> _binaryOperators;
	static const vector<int> _binaryPrecedence;
	static const vector<string> _unaryOperators;
	static const vector<int> _unaryPrecedence;
	static const std::unordered_set<string> _operators;

	std::unordered_map<string, ExpressionData, StringHasher> _cache;
	SimpleLock _cacheLock;

	int64_t operandStack[1000];
	Debugger* _debugger;

	bool IsOperator(string token, int &precedence, bool unaryOperator);
	EvalOperators GetOperator(string token, bool unaryOperator);
	bool CheckSpecialTokens(string expression, size_t &pos, string &output, ExpressionData &data);
	string GetNextToken(string expression, size_t &pos, ExpressionData &data, bool &success);
	bool ProcessSpecialOperator(EvalOperators evalOp, std::stack<EvalOperators> &opStack, std::stack<int> &precedenceStack, vector<int64_t> &outputQueue);
	bool ToRpn(string expression, ExpressionData &data);
	int32_t PrivateEvaluate(string expression, DebugState &state, EvalResultType &resultType, MemoryOperationInfo &operationInfo, bool &success);
	ExpressionData* PrivateGetRpnList(string expression, bool& success);

public:
	ExpressionEvaluator(Debugger* debugger);

	int32_t Evaluate(ExpressionData &data, DebugState &state, EvalResultType &resultType, MemoryOperationInfo &operationInfo);
	int32_t Evaluate(string expression, DebugState &state, EvalResultType &resultType, MemoryOperationInfo &operationInfo);
	ExpressionData GetRpnList(string expression, bool &success);

	bool Validate(string expression);

#if _DEBUG
	void RunTests();
#endif
};
#pragma once
#include "stdafx.h"
#include <stack>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include "DebugTypes.h"
#include "Utilities/SimpleLock.h"

class Debugger;
class LabelManager;

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

	R0 = 20000000120,
	R1 = 20000000121,
	R2 = 20000000122,
	R3 = 20000000123,
	R4 = 20000000124,
	R5 = 20000000125,
	R6 = 20000000126,
	R7 = 20000000127,
	R8 = 20000000128,
	R9 = 20000000129,
	R10 = 20000000130,
	R11 = 20000000131,
	R12 = 20000000132,
	R13 = 20000000133,
	R14 = 20000000134,
	R15 = 20000000135,
	SrcReg = 20000000137,
	DstReg = 20000000138,
	SFR = 20000000139,
	PBR = 20000000140,
	RomBR = 20000000141,
	RamBR = 20000000142,

	RegB = 20000000160,
	RegC = 20000000161,
	RegD = 20000000162,
	RegE = 20000000163,
	RegF = 20000000164,
	RegH = 20000000165,
	RegL = 20000000166,
	RegAF = 20000000167,
	RegBC = 20000000168,
	RegDE = 20000000169,
	RegHL = 20000000170,

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
	LabelManager* _labelManager;
	CpuType _cpuType;
	SnesMemoryType _cpuMemory;

	bool IsOperator(string token, int &precedence, bool unaryOperator);
	EvalOperators GetOperator(string token, bool unaryOperator);
	bool CheckSpecialTokens(string expression, size_t &pos, string &output, ExpressionData &data);
	int64_t ProcessCpuSpcTokens(string token);
	int64_t ProcessSharedTokens(string token);
	int64_t ProcessGsuTokens(string token);
	int64_t ProcessGameboyTokens(string token);
	string GetNextToken(string expression, size_t &pos, ExpressionData &data, bool &success, bool previousTokenIsOp);
	bool ProcessSpecialOperator(EvalOperators evalOp, std::stack<EvalOperators> &opStack, std::stack<int> &precedenceStack, vector<int64_t> &outputQueue);
	bool ToRpn(string expression, ExpressionData &data);
	int32_t PrivateEvaluate(string expression, DebugState &state, EvalResultType &resultType, MemoryOperationInfo &operationInfo, bool &success);
	ExpressionData* PrivateGetRpnList(string expression, bool& success);

public:
	ExpressionEvaluator(Debugger* debugger, CpuType cpuType);

	int32_t Evaluate(ExpressionData &data, DebugState &state, EvalResultType &resultType, MemoryOperationInfo &operationInfo);
	int32_t Evaluate(string expression, DebugState &state, EvalResultType &resultType, MemoryOperationInfo &operationInfo);
	ExpressionData GetRpnList(string expression, bool &success);

	bool Validate(string expression);

#if _DEBUG
	void RunTests();
#endif
};
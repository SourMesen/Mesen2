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
class IDebugger;

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
	AbsoluteAddress = 20000000054,

	//Used to read ram address
	Bracket = 20000000060, //Read byte
	Braces = 20000000061, //Read word

	//Special value, not used as an operator
	Parenthesis = 20000000100,
};

enum EvalValues : int64_t
{
	RegA = 20000000100,
	RegX,
	RegY,
	RegSP,

	RegPS,
	RegPS_Carry,
	RegPS_Zero,
	RegPS_Interrupt,
	RegPS_Memory,
	RegPS_Index,
	RegPS_Decimal,
	RegPS_Overflow,
	RegPS_Negative,

	RegPC,
	PpuFrameCount,
	PpuCycle,
	PpuHClock,
	PpuScanline,
	Nmi,
	Irq,
	Value,
	Address,
	IsWrite,
	IsRead,
	IsDma,
	IsDummy,
	OpProgramCounter,

	R0,
	R1,
	R2,
	R3,
	R4,
	R5,
	R6,
	R7,
	R8,
	R9,
	R10,
	R11,
	R12,
	R13,
	R14,
	R15,
	SrcReg,
	DstReg,
	SFR,
	PBR,
	RomBR,
	RamBR,

	RegB,
	RegC,
	RegD,
	RegE,
	RegF,
	RegH,
	RegL,
	RegAF,
	RegBC,
	RegDE,
	RegHL,

	RegTR,
	RegTRB,
	RegRP,
	RegDP,
	RegDR,
	RegSR,
	RegK,
	RegM,
	RegN,

	RegPB,
	RegP,
	RegMult,
	
	RegMDR,
	RegMAR,
	RegDPR,

	Sprite0Hit,
	VerticalBlank,
	SpriteOverflow,

	FirstLabelIndex,
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
	vector<int64_t> RpnQueue;
	vector<string> Labels;
};

class ExpressionEvaluator
{
private:
	static const vector<string> _binaryOperators;
	static const vector<int> _binaryPrecedence;
	static const vector<string> _unaryOperators;
	static const vector<int> _unaryPrecedence;
	static const unordered_set<string> _operators;

	unordered_map<string, ExpressionData, StringHasher> _cache;
	SimpleLock _cacheLock;
	
	Debugger* _debugger;
	IDebugger* _cpuDebugger;
	LabelManager* _labelManager;
	CpuType _cpuType;
	MemoryType _cpuMemory;

	bool IsOperator(string token, int &precedence, bool unaryOperator);
	EvalOperators GetOperator(string token, bool unaryOperator);
	unordered_map<string, int64_t>* GetAvailableTokens();
	bool CheckSpecialTokens(string expression, size_t &pos, string &output, ExpressionData &data);

	unordered_map<string, int64_t>& GetSnesTokens();
	int64_t GetSnesTokenValue(int64_t token, EvalResultType& resultType);

	unordered_map<string, int64_t>& GetSpcTokens();
	int64_t GetSpcTokenValue(int64_t token, EvalResultType& resultType);

	unordered_map<string, int64_t>& GetGsuTokens();
	int64_t GetGsuTokenValue(int64_t token, EvalResultType& resultType);

	unordered_map<string, int64_t>& GetCx4Tokens();
	int64_t GetCx4TokenValue(int64_t token, EvalResultType& resultType);
	
	unordered_map<string, int64_t>& GetNecDspTokens();
	int64_t GetNecDspTokenValue(int64_t token, EvalResultType& resultType);

	unordered_map<string, int64_t>& GetGameboyTokens();
	int64_t GetGameboyTokenValue(int64_t token, EvalResultType& resultType);

	unordered_map<string, int64_t>& GetNesTokens();
	int64_t GetNesTokenValue(int64_t token, EvalResultType& resultType);

	unordered_map<string, int64_t>& GetPceTokens();
	int64_t GetPceTokenValue(int64_t token, EvalResultType& resultType);

	bool ReturnBool(int64_t value, EvalResultType& resultType);

	int64_t ProcessSharedTokens(string token);
	
	string GetNextToken(string expression, size_t &pos, ExpressionData &data, bool &success, bool previousTokenIsOp);
	bool ProcessSpecialOperator(EvalOperators evalOp, std::stack<EvalOperators> &opStack, std::stack<int> &precedenceStack, vector<int64_t> &outputQueue);
	bool ToRpn(string expression, ExpressionData &data);
	int32_t PrivateEvaluate(string expression, EvalResultType &resultType, MemoryOperationInfo &operationInfo, bool &success);
	ExpressionData* PrivateGetRpnList(string expression, bool& success);

protected:

public:
	ExpressionEvaluator(Debugger* debugger, IDebugger* cpuDebugger, CpuType cpuType);

	int32_t Evaluate(ExpressionData &data, EvalResultType &resultType, MemoryOperationInfo &operationInfo);
	int32_t Evaluate(string expression, EvalResultType &resultType, MemoryOperationInfo &operationInfo);
	ExpressionData GetRpnList(string expression, bool &success);

	void GetTokenList(char* tokenList);

	bool Validate(string expression);

#if _DEBUG
	void RunTests();
#endif
};
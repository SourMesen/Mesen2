#pragma once
#include "pch.h"
#include <stack>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include "Debugger/DebugTypes.h"
#include "Utilities/SimpleLock.h"

class Debugger;
class LabelManager;
class IDebugger;

enum EvalOperators : int64_t
{
	//Binary operators
	Multiplication = 2000000000000,
	Division,
	Modulo,
	Addition,
	Substration,
	ShiftLeft,
	ShiftRight,
	SmallerThan,
	SmallerOrEqual,
	GreaterThan,
	GreaterOrEqual,
	Equal,
	NotEqual,
	BinaryAnd,
	BinaryXor,
	BinaryOr,
	LogicalAnd,
	LogicalOr,

	//Unary operators
	Plus,
	Minus,
	BinaryNot,
	LogicalNot,
	AbsoluteAddress,
	ReadDword, //Read dword (32-bit)

	//Used to read ram address
	Bracket, //Read byte (8-bit)
	Braces, //Read word (16-bit)

	//Special value, not used as an operator
	Parenthesis,
};

enum EvalValues : int64_t
{
	RegA = 3000000000000,
	RegX,
	RegY,

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
	RegIX,
	RegIY,

	RegAltA,
	RegAltB,
	RegAltC,
	RegAltD,
	RegAltE,
	RegAltF,
	RegAltH,
	RegAltL,
	RegAltAF,
	RegAltBC,
	RegAltDE,
	RegAltHL,
	RegI,
	RegR,

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

	RegSP,
	RegDB,
	RegPS,

	RegPC,
	PpuFrameCount,
	PpuCycle,
	PpuHClock,
	PpuScanline,

	PpuVramAddress,
	PpuTmpVramAddress,

	Nmi,
	Irq,
	Value,
	Address,
	MemoryAddress,
	IsWrite,
	IsRead,
	IsDma,
	IsDummy,
	OpProgramCounter,

	RegPS_Carry,
	RegPS_Zero,
	RegPS_Interrupt,
	RegPS_Memory,
	RegPS_Index,
	RegPS_Decimal,
	RegPS_Overflow,
	RegPS_Negative,

	Sprite0Hit,
	VerticalBlank,
	SpriteOverflow,
	SpriteCollision,

	SpcDspReg,

	PceVramTransferDone,
	PceSatbTransferDone,
	PceScanlineDetected,
	PceIrqVdc2,
	PceSelectedPsgChannel,
	PceSelectedVdcRegister,

	SmsVdpAddressReg,
	SmsVdpCodeReg,

	CPSR,

	RegAX,
	RegBX,
	RegCX,
	RegDX,

	RegAL,
	RegBL,
	RegCL,
	RegDL,

	RegAH,
	RegBH,
	RegCH,
	RegDH,

	RegCS,
	RegDS,
	RegES,
	RegSS,

	RegSI,
	RegDI,
	RegBP,
	RegIP,

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

	unordered_map<string, int64_t>& GetSt018Tokens();
	int64_t GetSt018TokenValue(int64_t token, EvalResultType& resultType);

	unordered_map<string, int64_t>& GetGameboyTokens();
	int64_t GetGameboyTokenValue(int64_t token, EvalResultType& resultType);

	unordered_map<string, int64_t>& GetNesTokens();
	int64_t GetNesTokenValue(int64_t token, EvalResultType& resultType);

	unordered_map<string, int64_t>& GetPceTokens();
	int64_t GetPceTokenValue(int64_t token, EvalResultType& resultType);

	unordered_map<string, int64_t>& GetSmsTokens();
	int64_t GetSmsTokenValue(int64_t token, EvalResultType& resultType);

	unordered_map<string, int64_t>& GetGbaTokens();
	int64_t GetGbaTokenValue(int64_t token, EvalResultType& resultType);

	unordered_map<string, int64_t>& GetWsTokens();
	int64_t GetWsTokenValue(int64_t token, EvalResultType& resultType);

	bool ReturnBool(int64_t value, EvalResultType& resultType);

	int64_t ProcessSharedTokens(string token);
	
	string GetNextToken(string expression, size_t &pos, ExpressionData &data, bool &success, bool previousTokenIsOp);
	bool ProcessSpecialOperator(EvalOperators evalOp, std::stack<EvalOperators> &opStack, std::stack<int> &precedenceStack, vector<int64_t> &outputQueue);
	bool ToRpn(string expression, ExpressionData &data);
	int64_t PrivateEvaluate(string expression, EvalResultType &resultType, MemoryOperationInfo &operationInfo, AddressInfo& addressInfo, bool &success);
	ExpressionData* PrivateGetRpnList(string expression, bool& success);

protected:

public:
	ExpressionEvaluator(Debugger* debugger, IDebugger* cpuDebugger, CpuType cpuType);

	int64_t Evaluate(ExpressionData &data, EvalResultType &resultType, MemoryOperationInfo &operationInfo, AddressInfo& addressInfo);
	int64_t Evaluate(string expression, EvalResultType &resultType, MemoryOperationInfo &operationInfo, AddressInfo& addressInfo);
	ExpressionData GetRpnList(string expression, bool &success);

	void GetTokenList(char* tokenList);

	bool Validate(string expression);

#if _DEBUG
	void RunTests();
#endif
};
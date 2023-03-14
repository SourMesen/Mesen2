#include "pch.h"
#include "Debugger/ExpressionEvaluator.h"
#include "SNES/SpcTypes.h"
#include "SNES/Debugger/SpcDebugger.h"

unordered_map<string, int64_t>& ExpressionEvaluator::GetSpcTokens()
{
	static unordered_map<string, int64_t> supportedTokens = {
		{ "a", EvalValues::RegA },
		{ "x", EvalValues::RegX },
		{ "y", EvalValues::RegY },
		{ "ps", EvalValues::RegPS },
		{ "sp", EvalValues::RegSP },
		{ "pc", EvalValues::RegPC },
		{ "dspreg", EvalValues::SpcDspReg },
	};

	return supportedTokens;
}

int64_t ExpressionEvaluator::GetSpcTokenValue(int64_t token, EvalResultType& resultType)
{
	SpcState& s = (SpcState&)((SpcDebugger*)_cpuDebugger)->GetState();
	switch(token) {
		case EvalValues::RegA: return s.A;
		case EvalValues::RegX: return s.X;
		case EvalValues::RegY: return s.Y;
		case EvalValues::RegSP: return s.SP;
		case EvalValues::RegPS: return s.PS;
		case EvalValues::RegPC: return s.PC;
		case EvalValues::SpcDspReg: return s.DspReg;
		default: return 0;
	}
}
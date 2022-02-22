#include "stdafx.h"
#include "ExpressionEvaluator.h"
#include "NES/NesTypes.h"

unordered_map<string, int64_t>& ExpressionEvaluator::GetNesTokens()
{
	static unordered_map<string, int64_t> supportedTokens = {
		{ "a", EvalValues::RegA },
		{ "x", EvalValues::RegX },
		{ "y", EvalValues::RegY },
		{ "ps", EvalValues::RegPS },
		{ "sp", EvalValues::RegSP },
		{ "pc", EvalValues::RegPC },
		{ "irq", EvalValues::Irq },
		{ "nmi", EvalValues::Nmi }
	};

	return supportedTokens;
}

int64_t ExpressionEvaluator::GetNesTokenValue(int64_t token, EvalResultType& resultType, NesCpuState& s)
{
	switch(token) {
		case EvalValues::RegA: return s.A;
		case EvalValues::RegX: return s.X;
		case EvalValues::RegY: return s.Y;
		case EvalValues::RegSP: return s.SP;
		case EvalValues::RegPS: return s.PS;
		case EvalValues::RegPC: return s.PC;
		case EvalValues::Nmi:
			resultType = EvalResultType::Boolean;
			return s.NMIFlag;
		case EvalValues::Irq:
			resultType = EvalResultType::Boolean;
			return s.IRQFlag != 0;
		default: return 0;
	}
}